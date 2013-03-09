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
#include "ThreadingException.h"

#include "ThreadTask.h"

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

	void StartThreads();

	void Update(std::vector<vfb_ptr>* none_main_thread_buffers);

	void ScheduleAllocation(vfb_ptr	video_frame_buffer);

	void ScheduleWriteToDisk(vfb_ptr video_frame_buffer, vwm_ptr video_writer, bool flip);


private:
	std::deque<std::shared_ptr<DiskWritingTask>> disk_writing_task_queue;
	std::deque<std::shared_ptr<AllocationTask>> allocation_task_queue;

	std::vector<std::shared_ptr<DiskWritingTask>> disk_writing_tasks_finished;
	std::vector<std::shared_ptr<AllocationTask>> allocation_tasks_finished;


	std::shared_ptr<ThreadTask> thread_1_task;
	std::shared_ptr<ThreadTask> thread_2_task;
	std::vector<std::shared_ptr<ThreadTask>> thread_communicators;

	std::shared_ptr<boost::thread> thread_1;
	std::shared_ptr<boost::thread> thread_2;
};

void Thread_main(std::shared_ptr<ThreadTask> thread_queue);

#endif // ThreadPool_h__