#include "ThreadTask.h"

ThreadTask::ThreadTask()
{
	have_task = false;
	thread_running = false;
	current_or_previous_task = NO_TASK;
}

bool ThreadTask::ThreadRunning()
{
	return thread_running;
}

void ThreadTask::SetThreadFinished()
{
	have_task = false;
	thread_running = false;
}

void ThreadTask::SetThreadRunning()
{
	thread_running = true;
}

void ThreadTask::AddAllocationTask( std::shared_ptr<AllocationTask> task )
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

void ThreadTask::AddDiskTask( std::shared_ptr<DiskWritingTask> task )
{
	if(!thread_running &!have_task)
	{
		disk_writing_task = task;
		have_task = true;
		current_or_previous_task = WRITING_TASK;
	}
	else
		THREADING_EXCEPTION("thread is already running");
}

bool ThreadTask::HaveNewTaskToRun()
{
	return have_task;
}

CurrentOrPreviousTask ThreadTask::WhatTaskToRun()
{
	return current_or_previous_task;
}

std::shared_ptr<DiskWritingTask> ThreadTask::GetDiskTask()
{
	return disk_writing_task;
}

std::shared_ptr<AllocationTask> ThreadTask::GetAllocationTask()
{
	return allocation_task;
}

void ThreadTask::WriteFramesToDisk( boost::thread::id thread_id )
{
	vfb_ptr video_frame_buffer = disk_writing_task->video_frame_buffer;
	vwm_ptr video_writer = disk_writing_task->video_writer;

	video_writer_ptr vid_writer = video_writer->Lock_GetVideoWriter(thread_id);
	vf_deque_ptr vf_dq_ptr = video_frame_buffer->Lock_GetVideoFrameBuffer(thread_id);

	for(unsigned int i = 0; i < vf_dq_ptr->size(); i++)
	{
		vf_ptr p = vf_dq_ptr->at(i);

		if(disk_writing_task->flip)
			p->Flip();

		*vid_writer << p->image;
	}

	video_writer->ReleaseMutex(thread_id);
	video_frame_buffer->ReleaseMutex(thread_id);
}

std::shared_ptr<DiskWritingTask> ThreadTask::GetDiskWritingTask()
{
	if(!thread_running)
	{
		return disk_writing_task;
	}
	else THREADING_EXCEPTION("thread still running");
}

void ThreadTask::ClearOldInformation()
{
	allocation_task = NULL;
	disk_writing_task = NULL;
	current_or_previous_task = NO_TASK;
	have_task = false;
	thread_running = false;
}

	