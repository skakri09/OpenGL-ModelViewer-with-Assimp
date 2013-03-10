#include "ThreadPool.h"


ThreadPool::ThreadPool()
{
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::StartThreads()
{
	thread_1_task = std::make_shared<ThreadedEncodeWriter>();
	//thread_2_task = std::make_shared<ThreadTask>();

	thread_1 = std::make_shared<boost::thread>(Thread_main, thread_1_task);
	//thread_2 = std::make_shared<boost::thread>(Thread_main, thread_2_task);

	thread_communicators.push_back(thread_1_task);
	//thread_communicators.push_back(thread_2_task);
}

void ThreadPool::Update(std::vector<vfb_ptr>* none_main_thread_buffers)
{
	//Must ask for old tasks before assigning new tasks if we want to get old memory back
	for(unsigned int i = 0; i < thread_communicators.size(); i++)
	{
		if(thread_communicators.at(i)->Get_Status() == FINISHED)
		{
			std::shared_ptr<std::deque<DiskWritingTask>> sp_dq_dwt = thread_communicators.at(i)->GetDiskWritingTasksForRecycle();

			for(unsigned int i = 0; i < sp_dq_dwt->size(); i++)
			{
				none_main_thread_buffers->push_back(sp_dq_dwt->at(i).video_frame_buffer);
				none_main_thread_buffers->back()->ResetBufferForReuse();

			}

			thread_communicators.at(i)->ClearOldInformation();
		}
	}

	if(!disk_writing_task_queue.empty())
	{
		for(unsigned int i = 0; i < thread_communicators.size(); i++)
		{
			if(!thread_communicators.at(i)->ThreadRunning() && !thread_communicators.at(i)->HaveNewTaskToRun())
				thread_communicators.at(i)->AddWritingTasks(&disk_writing_task_queue);
		}
	}

	if(!allocation_task_queue.empty())
	{
		for(unsigned int i = 0; i < thread_communicators.size(); i++)
		{
			//if(!thread_communicators.at(i)->ThreadRunning() && !thread_communicators.at(i)->HaveNewTaskToRun())
			//{
			//	thread_communicators.at(i)->AddAllocationTask(allocation_task_queue.front());
			//	allocation_task_queue.pop_front();
			//}
			//if(allocation_task_queue.empty())
			//	break;
		}
	}
}

void ThreadPool::ScheduleAllocation( vfb_ptr video_frame_buffer )
{
	allocation_task_queue.push_back(std::make_shared<AllocationTask>(video_frame_buffer));
}

void ThreadPool::ScheduleWriteToDisk( vfb_ptr video_frame_buffer, bool flip )
{
	//add if(FULL)
	disk_writing_task_queue.push_back(DiskWritingTask(video_frame_buffer, flip));
}


void Thread_main( std::shared_ptr<ThreadedEncodeWriter> thread_queue )
{
	bool run_thread = true;
	boost::thread::id thread_id = boost::this_thread::get_id();

	while(run_thread)
	{

		if(thread_queue->HaveNewTaskToRun())
		{
			thread_queue->SetThreadRunning();
		
			thread_queue->WriteFramesToDisk(thread_id);

			thread_queue->SetThreadFinished();
		}
		//else
			//boost::this_thread::sleep(boost::posix_time::millisec(10));//sleep for 10 millisec
	}
}
