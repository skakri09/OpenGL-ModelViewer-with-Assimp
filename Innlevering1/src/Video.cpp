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
	
	diskStoredFramesCounter = 0;

	currentVideoSubFolder = CreateFramesDirectory();

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
	unsigned int framesCount = oldFrames.size();
	Frames.clear();

	boost::thread WriteThread1(WriteFramesToDisk, oldFrames, window_width, window_height,
								diskStoredFramesCounter, currentVideoSubFolder);
	diskStoredFramesCounter += framesCount;
}

std::string Video::CreateFramesDirectory()
{
	std::string framesDir;

	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	namespace pt = boost::posix_time;
	pt::ptime time =  pt::microsec_clock::local_time();
	std::stringstream ss;
	ss << static_cast<int>(now.date().month()) << "/" << now.date().day()
		<< "/" << now.date().year() << " " << time.time_of_day();
	std::cout << ss.str() << std::endl;

	

	path p = "video";
	if(exists(p))
	{
		directory_iterator end;
		//Looping trough the contents of the current directory
		for(directory_iterator iter(p); iter != end; ++iter)
		{
			if(is_directory(*iter))	
			{
				path p = *iter;
				std::string pathName = p.string();
				
				
				std::cout << p.string() << "\\" << "\t\t" << "folder" << std::endl;
			}
		}
	}
	return framesDir;
}

void WriteFramesToDisk(std::deque<Frame*> Frames,
						unsigned int window_width, 
						unsigned int window_height,
						unsigned int startSaveindex,
						std::string videoSubFolder)
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
		str<<path<<startSaveindex+i<<".bmp";

		ilSaveImage(str.str().c_str());
		delete f;
	}
}