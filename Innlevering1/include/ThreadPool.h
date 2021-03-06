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

	void ScheduleWriteToDisk(vfb_ptr video_frame_buffer, bool flip);

	void BeginWriting(const std::string& video_path, int fourcc, double fps,
						cv::Size frame_size, bool isColor = true);

	void FinishWriting();

private:
	std::deque<DiskWritingTask>* disk_writing_task_queue;
	std::deque<std::shared_ptr<AllocationTask>> allocation_task_queue;

	bool stop_writing_after_current_frames;

	std::shared_ptr<ThreadedEncodeWriter> thread_1_task;
	std::shared_ptr<ThreadedEncodeWriter> thread_2_task;
	std::vector<std::shared_ptr<ThreadedEncodeWriter>> thread_communicators;

	std::shared_ptr<boost::thread> thread_1;
	std::shared_ptr<boost::thread> thread_2;
};

void Writer_thread_main(std::shared_ptr<ThreadedEncodeWriter> thread_queue);

#endif // ThreadPool_h__