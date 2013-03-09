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
			
			Mat* img = video_frame_buffers.front()->GetNextFrame();
			
			glPixelStorei(GL_PACK_ALIGNMENT, (img->step & 3) ? 1 : 4);
			glPixelStorei(GL_PACK_ROW_LENGTH, img->step/img->elemSize());

			glReadPixels(0, 0, img->cols, img->rows, GL_BGR, GL_UNSIGNED_BYTE, img->data);

			if(video_frame_buffers.front()->BufferFilled())
			{
				vfb_ptr p = video_frame_buffers.front();
				video_frame_buffers.pop_front();

				thread_pool.ScheduleWriteToDisk(p, video_writer, true);
			}
			img = NULL;
		}
	}
	return true;	
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

	vfb_ptr p = std::make_shared<VideoFrameBuffer>(cv::Size(window_width, window_height), _type, frame_buffer_size);
	buffers_being_allocated.push_back(p);
	thread_pool.ScheduleAllocation(p);
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
