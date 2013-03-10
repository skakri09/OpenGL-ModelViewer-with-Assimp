/********************************************************************
    created:    4:3:2013   17:25
    filename:   Video.h
    author:     Kristian Skarseth
    
    purpose:   

	*--------------------------------------------------------------------
	* Possible list of codecs: http://www.fourcc.org/codecs.php
	* 
	* List of current successful codecs, followed
	* by the filesize on a 800*600, 30fps 4sec clip:
	* ------------------------------------------------
	* 0 (default out, uncompressed .avi)	81	 MB
	* CV_FOURCC('X','V','I','D')			1.41 MB
	* CV_FOURCC('D','X','5','0')			1.56 MB
	* CV_FOURCC('F','L','V','1')			1.73 MB
	* CV_FOURCC('D','I','V','X')			1.50 MB
	* CV_FOURCC('M','J','P','G')			3.66 MB
	*
	* -----------------------------------------------
	* Equal test with 1920*1080 resolution:
	* -----------------------------------------------
	* 0 (default out, uncompressed .avi)	355	 MB, 200 FPS @ 1 core(3.8Ghz)
	* CV_FOURCC('X','V','I','D')			4.42 MB, 34  FPS @ 1 core(3.8Ghz)
	* CV_FOURCC('M','J','P','G')			12.9 MB, 23  FPS @ 1 core(3.8Ghz)
	*
	* -----------------------------------------------
	* 1920*1080 resolution, preallocated and created Matrices:
	* -----------------------------------------------
	* CV_FOURCC('X','V','I','D')			3.29 MB, 130 FPS @ 1 core(3.8Ghz)
	*
	*--------------------------------------------------------------------

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

	void ToggleRecording(unsigned int target_fps);

	/**
	* Takes care of getting new storage space for frames when needed.
	*/
	void Update();

	/*
	* Returns false when the last frame of prepared storing was stored
	*/
	void StoreFrame(float deltaTime);

private:
	ThreadPool thread_pool;

	//When the size of video_frame_buffers hits this number,
	//we allocate another buffer.
	static const unsigned int min_allocated_buffers = 20;

	//If the size of allocated memory exceeds this number (bytes),
	//an exception is thrown.
	static const unsigned int max_preallocated_bytes = 100000000;

	//The amount of frames stored in each video_frame_buffer
	static const unsigned int frame_buffer_size = 2;

	// Storage type used for each color component in a frame buffer (unsigned char)
	static const int _type = CV_8UC3;

	static const int image_components = 3;

	/**
	* Frame buffers ready to receive image data
	*/
	std::deque<vfb_ptr> video_frame_buffers;

	/**
	* Local vector of vfb_ptrs to all vfb objects currently being 
	* allocated and initialized on some thread
	*/
	std::vector<vfb_ptr> buffers_being_allocated;


	unsigned int window_width, window_height, fps;

	float recordTimer;

	bool recording;

	void OrderNewFrameBuffer();

	std::string CreateVideoName(std::string folder, std::string format);


	/**
	* PBOs
	*/
	static const GLuint NUM_PBO = 2;
	int vram_to_system_index;
	int gpu_to_vram_index;
	unsigned int mem_size;
	GLuint pbos[NUM_PBO];
};





#endif // Video_h__