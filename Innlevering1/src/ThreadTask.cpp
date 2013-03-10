#include "ThreadTask.h"

ThreadedEncodeWriter::ThreadedEncodeWriter()
{
	have_task = false;
	thread_running = false;
	current_or_previous_task = NO_TASK;
}


void ThreadedEncodeWriter::BeginWriting( const std::string& filename, int fourcc, double fps,
									cv::Size frameSize, bool isColor )
{
	video_writer.open(filename, fourcc, fps, frameSize, isColor);
}


void ThreadedEncodeWriter::FinishWriting()
{
	finish_writing = true;
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

void ThreadedEncodeWriter::AddAllocationTask( std::shared_ptr<AllocationTask> task )
{
	if(!thread_running &!have_task)
	{
		current_or_previous_task = ALLOCATION_TASK;
		allocation_task = task;
		have_task = true;
	}
	else
		THREADING_EXCEPTION("thread is already running");
}

void ThreadedEncodeWriter::AddDiskTask( std::shared_ptr<DiskWritingTask> task )
{
	if(!thread_running && !have_task)
	{
		disk_writing_tasks = task;
		have_task = true;
		current_or_previous_task = WRITING_TASK;
	}
	else
		THREADING_EXCEPTION("thread is already running");
}


void ThreadedEncodeWriter::AddWritingTasks( std::shared_ptr<std::deque<DiskWritingTask>> tasks )
{
	if(!thread_running && !have_task)
		disk_writing_tasks->insert(disk_writing_tasks->back(), tasks->begin(), tasks->end());
	else
		THREADING_EXCEPTION("thread is already running");
}

bool ThreadedEncodeWriter::HaveNewTaskToRun()
{
	return have_task;
}

CurrentOrPreviousTask ThreadedEncodeWriter::WhatTaskToRun()
{
	return current_or_previous_task;
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

std::shared_ptr<AllocationTask> ThreadedEncodeWriter::GetAllocationTask()
{
	return allocation_task;
}

void ThreadedEncodeWriter::WriteFramesToDisk( boost::thread::id thread_id )
{
	if(video_writer.isOpened())
	{
		for(unsigned int i = 0; i < disk_writing_tasks->size(); i++)
		{
			vfb_ptr video_frame_buffer = disk_writing_tasks->at(i)->video_frame_buffer;
			vf_deque_ptr vf_dq_ptr = video_frame_buffer->Lock_GetVideoFrameBuffer(thread_id);

			for(unsigned int x = 0; x < vf_dq_ptr->size(); x++)
			{
				vf_ptr p = vf_dq_ptr->at(x);

				if(disk_writing_tasks->at(i)->flip)
					p->Flip();

				*video_writer << p->image;
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
	have_task = false;
	thread_running = false;
}


	