#include "Video.h"


Video::Video()
{
	recording = false;
	
	//namedWindow( "testwinName" );
	
}

Video::~Video()
{
}


void Video::Init(unsigned int window_width, unsigned int window_height)
{
	thread_pool.StartThreads();
	this->window_width = window_width;
	this->window_height = window_height;

	video_writer = std::make_shared<VideoWriterMutexed>();
}

void Video::PrepVideoRecording( unsigned int window_width, unsigned int window_height, 
								unsigned int image_components, unsigned int target_fps)
{
	//allocating data for the recording
	//Frames.resize(fps*secondsToRecord, Frame(window_width, window_height, image_components));
	Frames.resize(target_fps*4);
	for(unsigned int i = 0; i < Frames.size(); i++)
	{
		frames.push_back(new Mat(window_width, window_height, CV_8UC3));//change CV_8U3 to get format based on img_components param
		frames.back()->create(window_height, window_width, CV_8UC3);	
	}	

	frameCounter = 0;
	recording = true;
	this->window_width = window_width;
	this->window_height = window_height;
	this->components = image_components;
	this->fps = target_fps;
	
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


void Video::ToggleRecording( unsigned int target_fps )
{
	if(!recording)
	{
		
		std::shared_ptr<cv::VideoWriter> vw = video_writer->Try_Lock_GetVideoWriter(boost::this_thread::get_id());
		if(vw != NULL)
		{
			recording = true;
			this->fps = target_fps;
			std::string outPath = CreateVideoName("video/", ".avi");
			recordTimer = 0.0f;
			int codec =CV_FOURCC('X','V','I','D');
			vw->open(outPath, codec, fps, cv::Size(window_width, window_height));
			video_writer->ReleaseMutex(boost::this_thread::get_id());
		}
		else
			THREADING_EXCEPTION("video writer object is in use");
	}
	else
	{
		recording = false;
		video_writer->Lock_GetVideoWriter(boost::this_thread::get_id())->release();
		//stop the recording
	}
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


			Mat* img = video_frame_buffers.front()->GetNextFrame(); //frames[frameCounter];

			//img.create(window_height, window_width, CV_8UC3);
			//use fast 4-byte alignment (default anyway) if possible
			glPixelStorei(GL_PACK_ALIGNMENT, (img->step & 3) ? 1 : 4);

			//set length of one complete row in destination data (doesn't need to equal img.cols)
			glPixelStorei(GL_PACK_ROW_LENGTH, img->step/img->elemSize());

			glReadPixels(0, 0, img->cols, img->rows, GL_BGR, GL_UNSIGNED_BYTE, img->data);

			//cv::flip(*img, *img, 0);
			//*vw << *img;
			//vw->write(img);
			//imshow("framename", img);
			//++frameCounter;
			
			if(video_frame_buffers.front()->BufferFilled())
			{
				vfb_ptr p = video_frame_buffers.front();
				video_frame_buffers.pop_front();

				thread_pool.ScheduleWriteToDisk(p, video_writer, true);
			}

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
	std::deque<VideoFrame*> oldFrames = Frames;
	unsigned int framesCount = oldFrames.size();
	Frames.clear();

	//boost::thread WriteThread1(WriteFramesToDisk, oldFrames, window_width, window_height,
	//							diskStoredFramesCounter, currentVideoSubFolder);
	diskStoredFramesCounter += framesCount;
}

std::string Video::CreateVideoName(std::string folder, std::string format)
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
	
	
	path p = folder + sResult+format;
	if(!is_directory(p) && !is_regular_file(p))
	{
		//create_directory(p);
		return p.string();
	}
	else 
		THREADING_EXCEPTION("Something is rlllly wrong");
}

void Video::Update()
{
	thread_pool.Update();
	if( (video_frame_buffers.size() + buffers_being_allocated.size()) < min_allocated_buffers)
	{
		OrderNewFrameBuffer();
	}

	for(std::vector<vfb_ptr>::iterator i = buffers_being_allocated.begin(); i != buffers_being_allocated.end();)
	{
		if( (*i)->ReadyGet() != NULL )
		{
				video_frame_buffers.push_back((*i));

				i = buffers_being_allocated.erase(i);

				continue;
		}
		else
		{
			++i;
		}
	}
}

void Video::OrderNewFrameBuffer()
{
	unsigned int allocated_memory_usage = window_width*window_height*image_components;
	allocated_memory_usage *= (video_frame_buffers.size() + buffers_being_allocated.size() + 1);
	if( allocated_memory_usage >= max_preallocated_bytes)
	{
		THREADING_EXCEPTION("too much memory usage");
	}

	vfb_ptr p = std::make_shared<VideoFrameBuffer>(cv::Size(window_width, window_height), 
													_type, frame_buffer_size);
	buffers_being_allocated.push_back(p);
	thread_pool.ScheduleAllocation(p);
}
