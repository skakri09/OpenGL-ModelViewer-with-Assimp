#include "Video.h"


Video::Video()
{
	recording = false;
	namedWindow( "testwinName" );
}

Video::~Video()
{
}

void Video::PrepVideoRecording( unsigned int window_width, unsigned int window_height, 
								unsigned int image_components, unsigned int fps)
{
	//allocating data for the recording
	//Frames.resize(fps*secondsToRecord, Frame(window_width, window_height, image_components));
	Frames.resize(fps*4);
	for(unsigned int i = 0; i < Frames.size(); i++)
	{
		frames.push_back(new Mat(window_width, window_height, CV_8UC3));//change CV_8U3 to get format based on img_components param
		frames.back()->create(window_height, window_width, CV_8UC3);	
	}	
	//Frames[i] = new Frame(window_width, window_height, image_components);

	frameCounter = 0;
	recording = true;
	this->window_width = window_width;
	this->window_height = window_height;
	this->components = image_components;
	this->fps = fps;
	
	diskStoredFramesCounter = 0;

	//currentVideoSubFolder = CreateFramesDirectory();
	Size s(window_width, window_height);

	/*------------------------------------------------
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
	*
	*/

	int codec =CV_FOURCC('X','V','I','D');
	std::string vidName = "video/VideoTest.avi";
	vw = new VideoWriter(vidName, codec, 30, s, true);
	vw->open(vidName, codec, 30, s, true);
	if(vw->isOpened())
		int i = 55;
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
			//glReadPixels(0, 0, window_width, window_height, GL_RGB, GL_UNSIGNED_BYTE, &Frames[frameCounter]->data[0]);

			//Mat m(window_width, window_height, CV_FOURCC('r','g','b','a'), &Frames.at(i)->data[0]);
			//Mat img(window_width, window_height, CV_8UC3);
			Mat* img = frames[frameCounter];
			//img.create(window_height, window_width, CV_8UC3);
			//use fast 4-byte alignment (default anyway) if possible
			glPixelStorei(GL_PACK_ALIGNMENT, (img->step & 3) ? 1 : 4);

			//set length of one complete row in destination data (doesn't need to equal img.cols)
			glPixelStorei(GL_PACK_ROW_LENGTH, img->step/img->elemSize());

			glReadPixels(0, 0, img->cols, img->rows, GL_BGR, GL_UNSIGNED_BYTE, img->data);

			cv::flip(*img, *img, 0);
			*vw << *img;
			//vw->write(img);
			//imshow("framename", img);
			++frameCounter;
			
			if(frameCounter > Frames.size()-1)
			{
				vw->release();
				//FinishRecordingAndSave();
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
	ss << now.date().day() << "." << static_cast<int>(now.date().month())
		<< "." << now.date().year() << " - " << time.time_of_day();
	
	std::string ssCpy(ss.str());
	std::string sResult = boost::replace_all_copy(ssCpy, ":", ".");
	while(sResult.back() != '.'){
		sResult.pop_back();
	}sResult.pop_back();
	
	
	path p = "video/" + sResult;
	if(!is_directory(p))
	{
		create_directory(p);
		return sResult;
	}
	else 
		throw std::exception("Something is rlllly wrong");
}

void WriteFramesToDisk(std::deque<Frame*> Frames,
						unsigned int window_width, 
						unsigned int window_height,
						unsigned int startSaveindex,
						std::string videoSubFolder)
{
	//std::string path = "video/"+videoSubFolder+"/"+"videotest.avi";
	std::string path = "testVid.avi";
	CvSize size;
	size.height = window_height;
	size.width = window_width;

	cv::VideoWriter vw(path, -1, 30, Size(window_width, window_height), true);

	//CvVideoWriter* vw = cvCreateVideoWriter(path.c_str(), 0, 30, size);
	// CV_FOURCC('j','p','e','g')
	//CvVideoWriter* vwcv = cvCreateVideoWriter(path.c_str(), -1, 30, size, 1);
	
	/*if(!vw->isOpened())
		int breakhere = 1;*/
	
	/*for(unsigned int i = 0; i < Frames.size(); i++)*/
	for(unsigned int i = 0; i < 2; i++)
	{
		char* imgName = "imagenamelol";
		
		//CV_8UC3

		


		

		//namedWindow( "Gray image", CV_WINDOW_AUTOSIZE );
		
		
	//	IplImage* img = cvCreateImageHeader(size, IPL_DEPTH_8U, 1);
		//img->imageData = (char*)Frames.at(i)->data[0];
		//img->imageDataOrigin = img->imageData;
		
		//cvWriteFrame(vw,img);

		//cvReleaseVideoWriter(&vw);
		//vw << m;
		//vw->write(m);
	}
	cvWaitKey(0);
	//vw->release();
	


	/*for(unsigned int i = 0; i < Frames.size(); i++)
	{
	Frame* f = Frames[i];

	ILuint ImageName;

	ilGenImages(1, &ImageName); 
	ilBindImage(ImageName);
	ilTexImage(window_width, window_height, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, &f->data[0]);

	std::stringstream str;
	std::string path = "video/"+videoSubFolder+"/"+"frame";
	str<<path<<startSaveindex+i<<".bmp";

	ilSaveImage(str.str().c_str());
	delete f;
	}*/
}

std::vector<Frame*>* AllocateFrameBuffer( unsigned int window_width, unsigned int window_height, 
										  unsigned int image_components, unsigned int allocationSize )
{
	std::vector<Frame*>* frameBuffer = new std::vector<Frame*>();
	frameBuffer->resize(allocationSize);
	for(unsigned int i=0; i < frameBuffer->size(); i++)
		(*frameBuffer)[i] = new Frame(window_width, window_height, image_components);

	return frameBuffer;
}
