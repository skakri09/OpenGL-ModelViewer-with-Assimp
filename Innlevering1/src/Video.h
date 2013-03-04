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

using namespace boost::filesystem;

struct Frame{
	Frame(unsigned int window_width, 
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
					  unsigned int image_components, unsigned int fps, 
					  unsigned int secondsToRecord);



	/*
	* Returns false when the last frame of prepared storing was stored
	*/
	bool StoreFrame(float deltaTime);

	void FinishRecordingAndSave();

protected:

private:
	std::string CreateFramesDirectory();


	std::deque<Frame*> Frames;

	unsigned int window_width, window_height;
	unsigned int fps;
	unsigned int frameCounter;
	unsigned int components;
	float recordTimer;

	unsigned int diskStoredFramesCounter;
	std::string currentVideoSubFolder;

	bool recording;

	void DumpFramesToDisk();

	boost::thread WriteThread1;
	boost::thread WriteThread2;
};


void WriteFramesToDisk(std::deque<Frame*> Frames, 
					   unsigned int window_width, 
					   unsigned int window_height,
					   unsigned int startSaveindex,
					   std::string videoSubFolder);


#endif // Video_h__