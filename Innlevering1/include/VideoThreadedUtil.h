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
	
	boost::mutex vfb_mutex;
};

void WriteFramesToDisk(std::deque<VideoFrame*> Frames, 
						unsigned int window_width, 
						unsigned int window_height,
						unsigned int startSaveindex,
						std::string videoSubFolder);

/**
* Allocates a given amount of Frames in the target buffer
* @Param target_buffer: deque to place the Frame*s in
* @Param window_size: size of each frame (should be same size as FBO)
* @Param type: Represents the pixel-packing type. example: CV_8UC3 (3 unsigned chars/bytes)
* @Param alloc_size: The amount of Frames to initialize
*/
void AllocateFramesBuffer(std::deque<VideoFrame*>* target_buffer, 
						cv::Size window_size, 
						int _type, 
						unsigned int alloc_size);


#endif // VideoThreadedUtil_h__