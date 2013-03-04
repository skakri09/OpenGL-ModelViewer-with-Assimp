#include "Video.h"


Video::Video()
{
	recording = false;
}

Video::~Video()
{
}

void Video::PrepVideoRecording( unsigned int window_width, unsigned int window_height, 
	unsigned int image_components, unsigned int fps, 
	unsigned int secondsToRecord )
{
	//allocating data for the recording
	//Frames.resize(fps*secondsToRecord, Frame(window_width, window_height, image_components));
	Frames.resize(fps*secondsToRecord);
	for(unsigned int i = 0; i < fps*secondsToRecord; i++)
		Frames[i] = new Frame(window_width, window_height, image_components);

	frameCounter = 0;
	recording = true;
	this->window_width = window_width;
	this->window_height = window_height;
	this->components = image_components;
	this->fps = fps;
	
	recordTimer = 0.0f;
}

bool Video::StoreFrame(float deltaTime)
{
	if(recording)
	{
		recordTimer += deltaTime;
		float shallWe = (float)1/(float)fps;
		if(recordTimer > shallWe)
		{
			recordTimer = 0.0f;
			glReadBuffer(GL_FRONT);
			glReadPixels(0, 0, window_width, window_height, GL_RGB, GL_UNSIGNED_BYTE, &Frames[frameCounter]->data[0]);

			++frameCounter;

			if(frameCounter > Frames.size()-1)
			{
				FinishRecordingAndSave();
				return false;
			}
		}
	}
	return true;	
}

void Video::FinishRecordingAndSave()
{
	//recording = false;
	
	DumpFramesToDisk();
	//free memory in Frames
}

void Video::DumpFramesToDisk()
{
	std::deque<Frame*> oldFrames = Frames;
	Frames.clear();

	WriteThread1 = boost::thread(WriteFramesToDisk, oldFrames, window_width, window_height);
}

void WriteFramesToDisk(std::deque<Frame*> Frames,
						unsigned int window_width, 
						unsigned int window_height)
{
	for(unsigned int i = 0; i < Frames.size(); i++)
	{
		Frame* f = Frames[i];

		ILuint ImageName;

		ilGenImages(1, &ImageName); 
		ilBindImage(ImageName);
		ilTexImage(window_width, window_height, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, &f->data[0]);

		std::stringstream str;
		std::string path = "video/frame";
		str<<path<<i<<".bmp";

		ilSaveImage(str.str().c_str());
		delete f;
	}
}