/********************************************************************
    created:    4:3:2013   17:25
    filename:   Video.h
    author:     Kristian Skarseth
    
    purpose:    
*********************************************************************/
#ifndef Video_h__
#define Video_h__

#include <vector>
#include <deque>

#include <gl/glew.h>
#include <strstream>
#include <iostream>
#include <sstream>
#include <IL/il.h>
#include <IL/ilu.h>
#include <boost/thread/thread.hpp>

#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iomanip>

#include "ThreadPool.h"
#include "VideoThreadedUtil.h"
#include "ThreadingException.h"

using namespace boost::filesystem;
using namespace cv;

class Video
{
public:
    Video();
    ~Video();

	void Init(unsigned int window_width, unsigned int window_height);

	/*
	* Prepare the class for recording.
	* @Param window_width/window_height: Dimensions of the screen
	* @Param image_components: Currently only 3 supported (RGB)
	* @Param target_fps: target frames per seconds for the recording
	*/
	void PrepVideoRecording(unsigned int window_width, unsigned int window_height,
							unsigned int image_components, unsigned int target_fps);

	void ToggleRecording(unsigned int target_fps);

	/**
	* Takes care of getting new storage space for frames when needed.
	*/
	void Update();

	/*
	* Returns false when the last frame of prepared storing was stored
	*/
	bool StoreFrame(float deltaTime);

	void FinishRecordingAndSave();
		void OrderNewFrameBuffer();
private:
	ThreadPool thread_pool;
	//When the size of video_frame_buffers hits this number,
	//we allocate another buffer.
	static const unsigned int min_allocated_buffers = 4;

	//If the size of allocated memory exceeds this number (bytes),
	//an exception is thrown.
	static const unsigned int max_preallocated_bytes = 10000000;

	//The amount of frames stored in each video_frame_buffer
	static const unsigned int frame_buffer_size = 1;

	// Storage type used for each color component in a frame buffer (unsigned char)
	static const int _type = CV_8UC3;

	static const int image_components = 3;
	std::string CreateVideoName(std::string folder, std::string format);

	/**
	* Frame buffers ready to receive image data
	*/
	std::deque<vfb_ptr> video_frame_buffers;

	/**
	* Local vector of vfb_ptrs to all vfb objects currently being 
	* allocated and initialized on some thread
	*/
	std::vector<vfb_ptr> buffers_being_allocated;

	std::vector<Mat*> frames;
	std::deque<VideoFrame*> Frames;

	unsigned int window_width, window_height;
	unsigned int fps;
	unsigned int frameCounter;
	unsigned int components;
	float recordTimer;

	unsigned int diskStoredFramesCounter;
	std::string currentVideoSubFolder;

	bool recording;

	void DumpFramesToDisk();

	vwm_ptr video_writer;

	cv::VideoWriter* vw;
};





#endif // Video_h__