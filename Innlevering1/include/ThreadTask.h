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

#include <deque>

#include "VideoThreadedUtil.h"
#include "ThreadingException.h"

/**
* A struct wrapper around the necessary objects to write
* a buffer to disk
*/
struct DiskWritingTask
{
	DiskWritingTask(vfb_ptr vfb, bool flip)
	{
		this->video_frame_buffer = vfb;
		this->flip = flip;
	}

	vfb_ptr video_frame_buffer;
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

enum ThreadedEncoderWriter_STATUS
{
	IDLE,
	NO_TASK,
	WORKING,
	THIS_TASK_FINISHED,
	SHUT_DOWN_WRITER_WHEN_DONE
};
/**
* A class that should be passed to a thread as it's access point to 
* the main program thread. The class will recieve tasks from the 
* threadpool object being run by the main thread.
*/
class ThreadedEncodeWriter
{
public:
	ThreadedEncodeWriter();
	~ThreadedEncodeWriter();

	void BeginWriting(const std::string& filename, int fourcc, double fps,
					cv::Size frameSize, bool isColor=true);

	void FinishWriting();
	bool IsFinished();

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
	//void AddAllocationTask(std::shared_ptr<AllocationTask> task);

	/**
	* Place a disk writing tasks in this threads' queue of tasks
	* @Return: true if the adding of tasks was successfull;
	*/
	bool AddWritingTasks(std::deque<DiskWritingTask>* tasks);

	/**
	* @Returns: true when the queue associated with this class
	* have a new task it can run
	*/
	bool HaveNewTaskToRun();

	void ClearOldInformation();

	/**
	* @Returns: the Disk writing tasks assigned to this class
	*/
	std::deque<DiskWritingTask>* GetTasks();

	std::deque<DiskWritingTask>* GetDiskWritingTasksForRecycle();

	/**
	* Writes the frames stored in the DiskWritingTask assigned to this class
	*/
	void WriteFramesToDisk(boost::thread::id thread_id);

	

	ThreadedEncoderWriter_STATUS Get_Status();

private:
	bool thread_running;

	bool have_task;

	bool finish_writing;

	ThreadedEncoderWriter_STATUS object_status;

	std::deque<DiskWritingTask>* disk_writing_tasks;
	
	//std::shared_ptr<AllocationTask>  allocation_task;

	VideoWriterMutexed video_writer;
	//cv::VideoWriter video_writer;
};

#endif // ThreadTask_h__