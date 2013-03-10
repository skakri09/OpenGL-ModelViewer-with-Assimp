/********************************************************************
    created:    7:3:2013   16:10
    filename:   VideoThreadedUtil.h
    author:     Kristian Skarseth
    
    purpose:    
*********************************************************************/
#ifndef VideoThreadedUtil_h__
#define VideoThreadedUtil_h__

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <boost/thread/thread.hpp>

#include <vector>
#include <deque>

#include "ThreadingException.h"

#pragma region VideoWriterStruct

struct VideoWriterMutexed
{
public:
	VideoWriterMutexed()
	{
		locked = false;
		video_writer = std::make_shared<cv::VideoWriter>();
	}
	
	/**
	* Halts the caller thread until it can lock the thread. When it's successfully
	* in locking the thread, or if it already owned this object, the video_writer 
	* object is returned
	* 
	* @Returns: a cv::VideoWriter object that can write frames to disk
	*/
	std::shared_ptr<cv::VideoWriter> Lock_GetVideoWriter(boost::thread::id thread_id)
	{
		/// If thread is locked, and we already own it, just return the video_writer
		if(locked && thread_id == locked_in_ID)
			return video_writer;
		else
		{ //Otherwise we do a normal lock(), halting the thread untill we can obtain ownership
			vw_mutex.lock();
			locked = true;
			locked_in_ID = thread_id;
			return video_writer;
		}
	}

	/**
	* Does a try_lock on this class. If it's successfully in locking the thread,
	* or if it already owned this object, the video_writer object is returned
	* 
	* @Returns: a cv::VideoWriter object that can write frames to disk
	*/
	std::shared_ptr<cv::VideoWriter> Try_Lock_GetVideoWriter(boost::thread::id thread_id)
	{
		// If thread is locked, and we already own it, just return the video_writer
		if(locked && thread_id == locked_in_ID)
			return video_writer;
		//If it's not locked, we perform a try_lock
		else if(!locked)
			locked = vw_mutex.try_lock();

		//If the try_lock did lock the thread, we update the struct vars and return 
		if(locked)
		{
			locked_in_ID = thread_id;
			return video_writer;
		}
		//If the try_lock was unsuccessfully we return null
		else 
			return NULL;
	}

	void ReleaseMutex(boost::thread::id thread_id)
	{
		// Only allowing unlocking of the mutex if it's the owner thread 
		// doing the function call
		if(locked && thread_id == locked_in_ID)
		{
			locked = false;
			vw_mutex.unlock();
		}
	}

private:
	std::shared_ptr<cv::VideoWriter> video_writer;

	bool locked;
	boost::thread::id locked_in_ID;
	boost::mutex vw_mutex;
};

/**
* Shared ptr to a VideoWriterMutexed object
* std::shared_ptr<VideoWriterMutexed> 
*/
typedef std::shared_ptr<VideoWriterMutexed> vwm_ptr;

/**
* Shared ptr to a cv::VideoWriter object
* std::shared_ptr<cv::VideoWriter> 
*/
typedef std::shared_ptr<cv::VideoWriter> video_writer_ptr;

#pragma endregion

#pragma region VideoFrame

struct VideoFrame
{
	VideoFrame(cv::Size window_size, int _type)
		:image(window_size, _type)
	{
		image.create(window_size, _type);
	}
	
	void Flip()
	{
		cv::flip(image, image, 0);
	}

	cv::Mat image;
};

/*
* std::shared_ptr<VideoFrame>
*/
typedef std::shared_ptr<VideoFrame> vf_ptr;

/*
* std::shared_ptr<std::deque<std::shared_ptr<VideoFrame>>> 
*/
typedef std::shared_ptr<std::deque<vf_ptr>> vf_deque_ptr;

#pragma endregion

enum VideoFrameBufferStatus
{
	NOT_INITIALIZED,
	FULL,
	READ_FOR_REUSE
};


struct VideoFrameBuffer
{
public:
	VideoFrameBuffer(cv::Size window_size, int _type, unsigned int alloc_size)
	{
		this->window_size = window_size;
		this->_type = _type;
		this->alloc_size = alloc_size;
		
		ready = false;
		locked = false;
		filled_frames = 0;
		buffer_status = NOT_INITIALIZED;
	}
	
	/**
	* Halts the caller thread until it can lock the thread. When it's successfully
	* in locking the thread, or if it already owned this object, the video_writer 
	* object is returned
	* 
	* @Returns: a cv::VideoWriter object that can write frames to disk
	*/
	vf_deque_ptr Lock_GetVideoFrameBuffer(boost::thread::id thread_id)
	{
		/// If thread is locked, and we already own it, just return the video_writer
		if(locked && thread_id == locked_in_ID)
			return video_frames_buffer;
		else
		{ //Otherwise we do a normal lock(), halting the thread untill we can obtain ownership
			vfb_mutex.lock();
			locked = true;
			locked_in_ID = thread_id;
			return video_frames_buffer;
		}
	}

	/**
	* Does a try_lock on this class. If it's successfully in locking the thread,
	* or if it already owned this object, the video_writer object is returned
	* 
	* @Returns: a cv::VideoWriter object that can write frames to disk
	*/
	vf_deque_ptr Try_Lock_GetVideoFrameBuffer(boost::thread::id thread_id)
	{
		// If thread is locked, and we already own it, just return the video_writer
		if(locked && thread_id == locked_in_ID)
			return video_frames_buffer;
		//If it's not locked, we perform a try_lock
		else if(!locked)
			locked = vfb_mutex.try_lock();

		//If the try_lock did lock the thread, we update the struct vars and return 
		if(locked)
		{
			locked_in_ID = thread_id;
			return video_frames_buffer;
		}
		//If the try_lock was unsuccessfully we return null
		else 
			return NULL;
	}

	/**
	* Allocates and initializes the video_frames_buffer in this object.
	* The function will perform a lock before it starts allocating.
	*/
	void Lock_AllocAndInit(boost::thread::id thread_id)
	{
		if(ready)
			return;

		if(!locked)
		{
			vfb_mutex.lock();
			locked_in_ID = thread_id;
			locked = true;
		}
		if(locked && thread_id == locked_in_ID)
		{
			video_frames_buffer = std::make_shared<std::deque<std::shared_ptr<VideoFrame>>>();
			// Creating the VideoFrame objects
			for(unsigned int i=0; i < alloc_size; i++)
			{
				vf_ptr p = std::make_shared<VideoFrame>(window_size, _type);
				video_frames_buffer->push_back(std::make_shared<VideoFrame>(window_size, _type));
			}
		}

		buffer_status = READ_FOR_REUSE;
		filled_frames = 0;
		ready = true;
		locked = false;
		vfb_mutex.unlock();
	}

	/*
	* If the object is marked as ready, we can get the video_frames_buffer 
	* without locking, as only the main thread will be using it. 
	* We will however return null as long as it's already marked as locked.
	*/
	vf_deque_ptr ReadyGet()
	{
		if(ready && !locked)
			return video_frames_buffer;
		else
			return NULL;
	}

	void ReleaseMutex(boost::thread::id thread_id)
	{
		// Only allowing unlocking of the mutex if it's the owner thread 
		// doing the function call
		if(locked && thread_id == locked_in_ID)
		{
			locked = false;
			vfb_mutex.unlock();
		}
	}

	bool BufferReady()
	{
		return ready;
	}

	cv::Mat* GetNextFrame()
	{
		if(buffer_status == FULL)
			THREADING_EXCEPTION("trying to access index buffer.size()+1");
		std::shared_ptr<VideoFrame> vf = video_frames_buffer->at(filled_frames);
		++filled_frames;
		if(filled_frames == video_frames_buffer->size())
			buffer_status = FULL;
		return &vf->image;
	}

	bool BufferFilled()
	{
		return filled_frames >= video_frames_buffer->size();
	}

	void ResetBufferForReuse()
	{
		ready = true;
		locked = false;
		filled_frames = 0;
		buffer_status = READ_FOR_REUSE;
	}

private: 
	vf_deque_ptr video_frames_buffer; //< Primary buffer, each entry in the buffer can store a frame
	VideoFrameBufferStatus buffer_status;

	cv::Size window_size; //< Window size, is used to set the size of each frame

	int _type; //< The type of each color component (CV_8UC3 equals unsigned char)

	unsigned int alloc_size; //< Amount of frames to allocate and intitialize

	unsigned int filled_frames; //< Number of frames already filled with image data

	bool ready; //< Is the VideoFrameBuffer ready to be written to?

	bool locked;
	boost::thread::id locked_in_ID;
	boost::mutex vfb_mutex; //< Mutex object to lock access to the video_frames_buffer
};

typedef std::shared_ptr<VideoFrameBuffer> vfb_ptr;



#endif // VideoThreadedUtil_h__