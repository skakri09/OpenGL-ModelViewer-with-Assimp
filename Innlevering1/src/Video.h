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

using namespace boost::filesystem;
using namespace cv;

struct VideoFrame{
	VideoFrame(unsigned int window_width, 
		unsigned int window_height,
		unsigned int image_components)
	{
		data.resize(window_width*window_height*image_components);
	}

	std::vector<unsigned char> data;
};

class Video
{
public:
    Video();
    ~Video();

	/*
	* Prepare the class for recording. fps and secondsToRecord is limited to 30
	*/
	void PrepVideoRecording(unsigned int window_width, unsigned int window_height,
							unsigned int image_components, unsigned int fps);



	/*
	* Returns false when the last frame of prepared storing was stored
	*/
	bool StoreFrame(float deltaTime);

	void FinishRecordingAndSave();

protected:

private:
	std::string CreateFramesDirectory();

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

	cv::VideoWriter* vw;
};





#endif // Video_h__