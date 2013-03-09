#include "ThreadPool.h"


ThreadPool::ThreadPool()
{
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::StartThreads()
{
	thread_1_task = std::make_shared<ThreadTask>(thread_1);
	thread_2_task = std::make_shared<ThreadTask>(thread_2);

	thread_1 = std::make_shared<boost::thread>(Thread_1_func, thread_1_task);
	thread_2 = std::make_shared<boost::thread>(Thread_2_func, thread_2_task);

	thread_tasks.push_back(thread_1_task);
	thread_tasks.push_back(thread_2_task);
}

void ThreadPool::Update()
{
	if(!disk_writing_task_queue.empty())
	{
		for(unsigned int i = 0; i < thread_tasks.size(); i++)
		{
			if(!thread_tasks.at(i)->ThreadRunning())
			{
				thread_tasks.at(i)->AddDiskTask(disk_writing_task_queue.front());
			}
		}
	}
	if(!allocation_task_queue.empty())
	{

	}
}

void ThreadPool::ScheduleAllocation( vfb_ptr video_frame_buffer )
{
	allocation_task_queue.push_back(std::make_shared<AllocationTask>(video_frame_buffer));
}

void ThreadPool::ScheduleWriteToDisk( vfb_ptr video_frame_buffer, 
									vwm_ptr video_writer, bool flip )
{
	disk_writing_task_queue.push_back(std::make_shared<DiskWritingTask>(video_frame_buffer, video_writer, flip));
}


void Thread_1_func( std::shared_ptr<ThreadTask> thread_queue )
{
	bool running = true;
	boost::thread::id thread_id = thread_queue->GetThreadID();

	while(running)
	{
		if(thread_queue->NewTaskToRun())
		{
			thread_queue->SetThreadRunning();
			
			if(thread_queue->GetAllocationTask() != NULL)
			{
				thread_queue->GetAllocationTask()->video_frame_buffer->Lock_AllocAndInit(thread_id);
			}

			if(thread_queue->GetDiskTask() != NULL)
			{
				thread_queue->WriteFramesToDisk();
			}
		}
	}
}

void Thread_2_func( std::shared_ptr<ThreadTask> thread_queue )
{
	bool running = true;
	boost::thread::id thread_id = thread_queue->GetThreadID();

	while(running)
	{
		if(thread_queue->NewTaskToRun())
		{
			thread_queue->SetThreadRunning();

			if(thread_queue->GetAllocationTask() != NULL)
			{
				thread_queue->GetAllocationTask()->video_frame_buffer->Lock_AllocAndInit(thread_id);
			}

			if(thread_queue->GetDiskTask() != NULL)
			{
				thread_queue->WriteFramesToDisk();
			}
		}
	}
}
