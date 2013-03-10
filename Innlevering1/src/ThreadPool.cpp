#include "ThreadPool.h"


ThreadPool::ThreadPool()
{
	disk_writing_task_queue = new std::deque<DiskWritingTask>();
	stop_writing_after_current_frames = false;
}

ThreadPool::~ThreadPool()
{
	if(disk_writing_task_queue != NULL)
	{
		delete disk_writing_task_queue;
		disk_writing_task_queue = NULL;
	}
}

void ThreadPool::StartThreads()
{
	thread_1_task = std::make_shared<ThreadedEncodeWriter>();
	//thread_2_task = std::make_shared<ThreadTask>();

	thread_1 = std::make_shared<boost::thread>(Writer_thread_main, thread_1_task);
	//thread_2 = std::make_shared<boost::thread>(Thread_main, thread_2_task);

	thread_communicators.push_back(thread_1_task);
	//thread_communicators.push_back(thread_2_task);
}

void ThreadPool::Update(std::vector<vfb_ptr>* none_main_thread_buffers)
{
	//Must ask for old tasks before assigning new tasks if we want to get old memory back
	for(unsigned int i = 0; i < thread_communicators.size(); i++)
	{
		if(thread_communicators.at(i)->Get_Status() == THIS_TASK_FINISHED)
		{
			std::deque<DiskWritingTask>* sp_dq_dwt = thread_communicators.at(i)->GetDiskWritingTasksForRecycle();

			for(unsigned int x = 0; x < sp_dq_dwt->size(); x++)
			{
				none_main_thread_buffers->push_back(sp_dq_dwt->at(x).video_frame_buffer);
				none_main_thread_buffers->back()->ResetBufferForReuse();

			}

			thread_communicators.at(i)->ClearOldInformation();
		}
	}

	if(!disk_writing_task_queue->empty())
	{
		for(unsigned int i = 0; i < thread_communicators.size(); i++)
		{
			if(!thread_communicators.at(i)->ThreadRunning() && !thread_communicators.at(i)->HaveNewTaskToRun())
			{
				if(thread_communicators.at(i)->AddWritingTasks(disk_writing_task_queue))
					disk_writing_task_queue->clear();
			}
		}
	}
	else if(stop_writing_after_current_frames)
		for(unsigned int i = 0; i < thread_communicators.size(); i++)
			thread_communicators.at(i)->FinishWriting();

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
	disk_writing_task_queue->push_back(DiskWritingTask(video_frame_buffer, flip));
}

void ThreadPool::BeginWriting( const std::string& video_path, int fourcc, double fps, 
							   cv::Size frame_size, bool isColor /*= true*/ )
{
	for(unsigned int i = 0; i < thread_communicators.size(); i++)
	{
		if(thread_communicators.at(i)->Get_Status() == IDLE)
		{
			thread_communicators.at(i)->BeginWriting(video_path, fourcc, fps, frame_size, isColor);

			thread_communicators.at(i)->ClearOldInformation();
		}
	}
	stop_writing_after_current_frames = false;
}

void ThreadPool::FinishWriting()
{
	stop_writing_after_current_frames = true;
}


void Writer_thread_main( std::shared_ptr<ThreadedEncodeWriter> thread_task_object )
{
	bool run_thread = true;
	boost::thread::id thread_id = boost::this_thread::get_id();

	while(run_thread)
	{	
		if(thread_task_object->HaveNewTaskToRun())
		{
			thread_task_object->SetThreadRunning();
		
			thread_task_object->WriteFramesToDisk(thread_id);

			thread_task_object->SetThreadFinished();
		}
		//else
			//boost::this_thread::sleep(boost::posix_time::millisec(10));//sleep for 10 millisec
	}
}
