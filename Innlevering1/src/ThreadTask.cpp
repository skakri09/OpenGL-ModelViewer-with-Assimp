#include "ThreadTask.h"

ThreadedEncodeWriter::ThreadedEncodeWriter()
{
	have_task = false;
	thread_running = false;
	object_status = IDLE;
	disk_writing_tasks = new std::deque<DiskWritingTask>();
}


ThreadedEncodeWriter::~ThreadedEncodeWriter()
{
	object_status = SHUT_DOWN_WRITER_WHEN_DONE;
	video_writer_ptr p = video_writer.Lock_GetVideoWriter(boost::this_thread::get_id());
	if(p->isOpened())
		p->release();
	if(disk_writing_tasks != NULL)
	{
		delete disk_writing_tasks;
		disk_writing_tasks = NULL;
	}
}

void ThreadedEncodeWriter::BeginWriting( const std::string& filename, int fourcc, double fps,
	cv::Size frameSize, bool isColor )
{
	video_writer_ptr vwp = video_writer.Lock_GetVideoWriter(boost::this_thread::get_id());
	vwp->open(filename, fourcc, fps, frameSize, isColor);
	video_writer.ReleaseMutex(boost::this_thread::get_id());

	object_status = NO_TASK;
	have_task = false;
	thread_running = false;
	finish_writing = false;
}


void ThreadedEncodeWriter::FinishWriting()
{
	finish_writing = true;
	if(object_status == THIS_TASK_FINISHED || object_status == IDLE || object_status == NO_TASK)
	{
		video_writer_ptr vwp = video_writer.Try_Lock_GetVideoWriter(boost::this_thread::get_id());
		if(vwp != NULL)
		{
			vwp->release();
			video_writer.ReleaseMutex(boost::this_thread::get_id());
			object_status = IDLE;
		}
		else THREADING_EXCEPTION("something wrong with the logic. This else should not happen");
	}
	else
		object_status = SHUT_DOWN_WRITER_WHEN_DONE;
}

bool ThreadedEncodeWriter::IsFinished()
{
	return finish_writing;
}

bool ThreadedEncodeWriter::ThreadRunning()
{
	return thread_running;
}

void ThreadedEncodeWriter::SetThreadFinished()
{
	have_task = false;
	thread_running = false;
	object_status = THIS_TASK_FINISHED;
}

void ThreadedEncodeWriter::SetThreadRunning()
{
	thread_running = true;
}

//void ThreadedEncodeWriter::AddAllocationTask( std::shared_ptr<AllocationTask> task )
//{
//	if(!thread_running &!have_task)
//	{
//		current_or_previous_task = ALLOCATION_TASK;
//		allocation_task = task;
//		have_task = true;
//	}
//	else
//		THREADING_EXCEPTION("thread is already running");
//}




bool ThreadedEncodeWriter::AddWritingTasks(std::deque<DiskWritingTask>* tasks )
{
	if(!thread_running && !have_task)
	{
		disk_writing_tasks->insert(disk_writing_tasks->end(), tasks->begin(), tasks->end());
		have_task = true;
		object_status = WORKING;
		return true;
	}
	else
	{
		THREADING_EXCEPTION("thread is already running");
		return false;
	}
}

bool ThreadedEncodeWriter::HaveNewTaskToRun()
{
	return have_task;
}

std::deque<DiskWritingTask>* ThreadedEncodeWriter::GetTasks()
{
	return disk_writing_tasks;
}

std::deque<DiskWritingTask>* ThreadedEncodeWriter::GetDiskWritingTasksForRecycle()
{
	if(!thread_running)
	{
		return disk_writing_tasks;
	}
	else THREADING_EXCEPTION("thread still running");
}

void ThreadedEncodeWriter::WriteFramesToDisk( boost::thread::id thread_id )
{
	video_writer_ptr writer = video_writer.Lock_GetVideoWriter(thread_id);
	if(writer->isOpened())
	{
		for(unsigned int i = 0; i < disk_writing_tasks->size(); i++)
		{
			vfb_ptr video_frame_buffer = disk_writing_tasks->at(i).video_frame_buffer;
			vf_deque_ptr vf_dq_ptr = video_frame_buffer->Lock_GetVideoFrameBuffer(thread_id);

			for(unsigned int x = 0; x < vf_dq_ptr->size(); x++)
			{
				vf_ptr p = vf_dq_ptr->at(x);

				if(disk_writing_tasks->at(i).flip)
					p->Flip();

				*writer << p->image;
			}
			video_frame_buffer->ReleaseMutex(thread_id);
		}
		if(object_status == SHUT_DOWN_WRITER_WHEN_DONE && writer->isOpened())
			writer->release();
		video_writer.ReleaseMutex(thread_id);
	}
	else
		THREADING_EXCEPTION("videowriter is not open");
}

void ThreadedEncodeWriter::ClearOldInformation()
{
	disk_writing_tasks->clear();
	object_status = NO_TASK;
	have_task = false;
	thread_running = false;
}

ThreadedEncoderWriter_STATUS ThreadedEncodeWriter::Get_Status()
{
	return object_status;
}


