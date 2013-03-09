/********************************************************************
    created:    9:3:2013   18:47
    filename:   ThreadTask.h
    author:     Kristian Skarseth
    
    purpose:    The file holds the ThreadTask class that is sent to a thread
				function as parameter, to later function as the access point
				to the thread.
				The threadpool will place tasks to be performed in the TheadTask
				object, and the thread will perform the task
*********************************************************************/
#ifndef ThreadTask_h__
#define ThreadTask_h__

#include "VideoThreadedUtil.h"
#include "ThreadingException.h"

/**
* A struct wrapper around the necessary objects to write
* a buffer to disk
*/
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

/**
* A struct wrapper around a video frame buffer object
*/
struct AllocationTask
{
	AllocationTask(vfb_ptr vfb)
	{
		video_frame_buffer = vfb;
	}

	vfb_ptr	video_frame_buffer;
};

enum CurrentOrPreviousTask
{
	NO_TASK,
	ALLOCATION_TASK,
	WRITING_TASK
};
/**
* A class that should be passed to a thread as it's access point to 
* the main program thread. The class will recieve tasks from the 
* threadpool object being run by the main thread.
*/
class ThreadTask
{
public:
	ThreadTask();

	/**
	* @Return: True if the thread is currently running a task
	*/
	bool ThreadRunning();

	/**
	* The thread using the class will call this function when it's 
	* completed the task it was assigned
	*/
	void SetThreadFinished();
	
	/**
	* Thet hread using the class will call this function once it
	* has been assigned a task
	*/
	void SetThreadRunning();

	/**
	* Place an allocation task in this threads' queue of tasks
	*/
	void AddAllocationTask(std::shared_ptr<AllocationTask> task);

	/**
	* Place a disk writing task in this threads' queue of tasks
	*/
	void AddDiskTask(std::shared_ptr<DiskWritingTask> task);

	/**
	* @Returns: true when the queue associated with this class
	* have a new task it can run
	*/
	bool HaveNewTaskToRun();

	CurrentOrPreviousTask WhatTaskToRun();

	void ClearOldInformation();

	/**
	* @Returns: the Disk writing task assigned to this class
	*/
	std::shared_ptr<DiskWritingTask> GetDiskTask();

	/**
	* @Returns: the allocation task assigned to this class
	*/
	std::shared_ptr<AllocationTask> GetAllocationTask();

	/**
	* Writes the frames stored in the DiskWritingTask assigned to this class
	*/
	void WriteFramesToDisk(boost::thread::id thread_id);

	std::shared_ptr<DiskWritingTask> GetDiskWritingTask();

private:
	bool thread_running;

	bool have_task;

	CurrentOrPreviousTask current_or_previous_task;

	std::shared_ptr<DiskWritingTask> disk_writing_task;
	
	std::shared_ptr<AllocationTask>  allocation_task;

};

#endif // ThreadTask_h__