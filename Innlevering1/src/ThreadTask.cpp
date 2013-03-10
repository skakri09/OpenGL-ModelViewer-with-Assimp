#include "ThreadTask.h"

ThreadedEncodeWriter::ThreadedEncodeWriter()
{
	have_task = false;
	thread_running = false;
	object_status = NO_TASK;
}


void ThreadedEncodeWriter::BeginWriting( const std::string& filename, int fourcc, double fps,
									cv::Size frameSize, bool isColor )
{
	video_writer.open(filename, fourcc, fps, frameSize, isColor);
}


void ThreadedEncodeWriter::FinishWriting()
{
	finish_writing = true;
	object_status = FINISHED;
}


bool ThreadedEncodeWriter::ThreadRunning()
{
	return thread_running;
}

void ThreadedEncodeWriter::SetThreadFinished()
{
	have_task = false;
	thread_running = false;
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




void ThreadedEncodeWriter::AddWritingTasks(const std::deque<DiskWritingTask>* tasks )
{
	if(!thread_running && !have_task)
	{
		disk_writing_tasks->insert(disk_writing_tasks->end(), tasks->begin(), tasks->end());
		object_status = WORKING;
	}
	else
		THREADING_EXCEPTION("thread is already running");
}

bool ThreadedEncodeWriter::HaveNewTaskToRun()
{
	return have_task;
}

std::shared_ptr<std::deque<DiskWritingTask>> ThreadedEncodeWriter::GetTasks()
{
	return disk_writing_tasks;
}

std::shared_ptr<std::deque<DiskWritingTask>> ThreadedEncodeWriter::GetDiskWritingTasksForRecycle()
{
	if(!thread_running)
	{
		return disk_writing_tasks;
	}
	else THREADING_EXCEPTION("thread still running");
}

void ThreadedEncodeWriter::WriteFramesToDisk( boost::thread::id thread_id )
{
	if(video_writer.isOpened())
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

				video_writer << p->image;
			}
			video_frame_buffer->ReleaseMutex(thread_id);
		}
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


	