/********************************************************************
    created:    9:3:2013   18:47
    filename:   ThreadTask.h
    author:     Kristian Skarseth
    
    purpose:    
*********************************************************************/
#ifndef ThreadTask_h__
#define ThreadTask_h__

#include "VideoThreadedUtil.h"
#include "ThreadingException.h"

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
	ThreadTask();

	bool ThreadRunning();

	void SetThreadFinished();
	
	void SetThreadRunning();

	void AddAllocationTask(std::shared_ptr<AllocationTask> task);

	void AddDiskTask(std::shared_ptr<DiskWritingTask> task);

	bool NewTaskToRun();

	std::shared_ptr<DiskWritingTask> GetDiskTask();

	std::shared_ptr<AllocationTask> GetAllocationTask();

	void WriteFramesToDisk(boost::thread::id thread_id);

private:
	bool thread_running;
	std::shared_ptr<DiskWritingTask> disk_writing_task;
	std::shared_ptr<AllocationTask>  allocation_task;

};

#endif // ThreadTask_h__