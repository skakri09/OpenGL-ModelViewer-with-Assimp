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

struct VideoWriterMutexed
{
	std::shared_ptr<cv::VideoWriter> video_writer;

	boost::mutex vwm_mutex;
};
typedef std::shared_ptr<VideoWriterMutexed> video_writer_ptr;

struct VideoFrame
{
	VideoFrame(cv::Size window_size, int _type)
	{
		image = new cv::Mat(window_size, _type); 
		image->create(window_size, _type);
	}

	void Flip()
	{
		cv::flip(*image, *image, 0);
	}

	cv::Mat* image;
};

typedef std::shared_ptr<VideoFrame> video_frame_ptr;
typedef std::shared_ptr<std::deque<video_frame_ptr>> video_frame_deque_ptr;

struct VideoFrameBuffer
{
	video_frame_deque_ptr video_frames_buffer;
	cv::Size window_size;
	int _type;
	unsigned int alloc_size;

	boost::mutex vfb_mutex;
};

typedef std::shared_ptr<VideoFrameBuffer> video_frame_buffer_ptr;


/**
* Writes a the VideoFrameBuffer to disk with the provided VideoWriter
* @Param video_Frame_buffer: The buffer to write from
* @Param video_writer: the VideoWriter object to write with
*/
void WriteFramesToDisk(video_frame_buffer_ptr video_frame_buffer,
					   cv::VideoWriter* video_writer, bool flip=true);


/**
* Allocates a given amount of Frames in the target buffer
* @Param video_frame_buffer: shared ptr to a VideoFrameBuffer object
* that should be allocated
*/
void AllocateFramesBuffer(video_writer_ptr video_frame_buffer);


#endif // VideoThreadedUtil_h__