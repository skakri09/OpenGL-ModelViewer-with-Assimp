/********************************************************************
    created:    5:3:2013   21:26
    filename:   ThreadPool.h
    author:     Kristian Skarseth
    
    purpose:    
*********************************************************************/
#ifndef ThreadPool_h__
#define ThreadPool_h__

#include <boost/thread/thread.hpp>
#include "VideoThreadedUtil.h"



struct DiskWritingTask
{
	DiskWritingTask(vfb_ptr vfb, vwm_ptr vw, bool flip)
	{
		this->video_frame_buffer = vfb;
		this->video_writer = vw;
		this->flip = flip;
	}

	vfb_ptr video_frame_buffer;
	vwm_ptr video_writer; 
	bool flip;
};

struct AllocationTask
{
	AllocationTask(vfb_ptr vfb)
	{
		video_frame_buffer = vfb;
	}

	vfb_ptr	video_frame_buffer;
};

class ThreadTask
{
public:
	ThreadTask(std::shared_ptr<boost::thread> owner_thread)
	{
		this->owner_thread = owner_thread;
	}

	bool ThreadRunning()
	{
		return thread_running;
	}

	void SetThreadSleeping()
	{
		thread_running = false;
	}

	void SetThreadRunning()
	{
		thread_running = true;
	}

	void AddAllocationTask(std::shared_ptr<AllocationTask> task)
	{
		if(!thread_running)
			allocation_task = task;
		else
			throw std::exception("thread already running");
	}
	
	void AddDiskTask(std::shared_ptr<DiskWritingTask> task)
	{
		if(!thread_running)
			disk_writing_task = task;
		else
			throw std::exception("thread already running");
	}

	bool NewTaskToRun()
	{
		if(disk_writing_task != NULL || allocation_task != NULL)
			return true;
		else 
			return false;
	}
	
	std::shared_ptr<DiskWritingTask> GetDiskTask()
	{
		return disk_writing_task;
	}

	std::shared_ptr<AllocationTask> GetAllocationTask()
	{
		return allocation_task;
	}

	void WriteFramesToDisk()
	{
		vfb_ptr video_frame_buffer = disk_writing_task->video_frame_buffer;
		vwm_ptr video_writer = disk_writing_task->video_writer;

		video_writer_ptr vid_writer = video_writer->Lock_GetVideoWriter(GetThreadID());
		vf_deque_ptr vf_dq_ptr = video_frame_buffer->Lock_GetVideoFrameBuffer(GetThreadID());

		for(unsigned int i = 0; i < vf_dq_ptr->size(); i++)
		{
			vf_ptr p = vf_dq_ptr->at(i);

			if(disk_writing_task->flip)
				p->Flip();

			*vid_writer << *p->image;
		}

		video_writer->ReleaseMutex(GetThreadID());
		video_frame_buffer->ReleaseMutex(GetThreadID());
	}


	boost::thread::id GetThreadID()
	{
		return owner_thread->get_id();
	}

private:
	bool thread_running;
	std::shared_ptr<DiskWritingTask> disk_writing_task;
	std::shared_ptr<AllocationTask>  allocation_task;

	std::shared_ptr<boost::thread> owner_thread;
};


class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

	void StartThreads();

	void Update();

	void ScheduleAllocation(vfb_ptr	video_frame_buffer);

	void ScheduleWriteToDisk(vfb_ptr video_frame_buffer, vwm_ptr video_writer, bool flip);


private:
	std::deque<std::shared_ptr<DiskWritingTask>> disk_writing_task_queue;
	std::deque<std::shared_ptr<AllocationTask>> allocation_task_queue;

	std::shared_ptr<ThreadTask> thread_1_task;
	std::shared_ptr<ThreadTask> thread_2_task;
	std::vector<std::shared_ptr<ThreadTask>> thread_tasks;

	std::shared_ptr<boost::thread> thread_1;
	std::shared_ptr<boost::thread> thread_2;
};

void Thread_1_func(std::shared_ptr<ThreadTask> thread_queue);

void Thread_2_func(std::shared_ptr<ThreadTask> thread_queue);

#endif // ThreadPool_h__