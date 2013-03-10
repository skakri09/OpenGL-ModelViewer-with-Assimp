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

	for(unsigned int i =0; i < min_allocated_buffers; i++)
		video_frame_buffers.push_back(std::make_shared<VideoFrameBuffer>(cv::Size(window_width, window_height), _type, frame_buffer_size, true));

	mem_size = window_width*window_height*image_components;

	glGenBuffers(NUM_PBO, pbos);

	for(unsigned int i = 0; i < NUM_PBO; i++)
	{
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[i]);
		glBufferData(GL_PIXEL_PACK_BUFFER, mem_size, NULL, GL_STATIC_READ);
	}

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	vram_to_system_index = 0;
	gpu_to_vram_index = NUM_PBO-1;

}

void Video::ToggleRecording( unsigned int target_fps )
{
	if(!recording)
	{
		recording = true;
		this->fps = target_fps;
		std::string outPath = CreateVideoName("video/", ".avi");
		recordTimer = 0.0f;
		int codec =CV_FOURCC('X','V','I','D');

		thread_pool.BeginWriting(outPath, codec, fps, cv::Size(window_width, window_height));
		/*OrderNewFrameBuffer();
		OrderNewFrameBuffer();
		OrderNewFrameBuffer();
		OrderNewFrameBuffer();
		OrderNewFrameBuffer();*/
	}
	else
	{
		recording = false;
		thread_pool.FinishWriting();
	}
}

void Video::StoreFrame(float deltaTime)
{
	if(recording)
	{
		recordTimer += deltaTime;
		if(fps == 0 || (recordTimer > (1.0f/(float)fps)) )
		{
			if(video_frame_buffers.size() == 0)
			{
				//THREADING_EXCEPTION("no frame buffers ready to be written to");
				std::cout << "THREADING_EXCEPTION(\"no frame buffers ready to be written to\");" << std::endl;
				//OrderNewFrameBuffer();
				return;
			}
			recordTimer = 0.0f;

			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[gpu_to_vram_index]);

			glReadBuffer(GL_FRONT);

			Mat* img = video_frame_buffers.front()->GetNextFrame();
			
			glPixelStorei(GL_PACK_ALIGNMENT, (img->step & 3) ? 1 : 4);
			glPixelStorei(GL_PACK_ROW_LENGTH, img->step/img->elemSize());

			glReadPixels(0, 0, window_width, window_height, GL_BGR, GL_UNSIGNED_BYTE, 0);

			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[vram_to_system_index]);
			void* _data = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

			if(_data != NULL)
				memcpy(img->data, _data, mem_size);
			
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

			GLuint _temp = pbos[0];
			for(int i = 1; i < NUM_PBO; i++)
				pbos[i-1] = pbos[i];
			pbos[NUM_PBO - 1] = _temp;

			if(video_frame_buffers.front()->BufferFilled())
			{
				vfb_ptr p = video_frame_buffers.front();
				video_frame_buffers.pop_front();

				thread_pool.ScheduleWriteToDisk(p, true);
			}
			//img = NULL;
		}
	}
}


void Video::Update()
{
	thread_pool.Update(&buffers_being_allocated);
	
	if( (video_frame_buffers.size() + buffers_being_allocated.size()) < min_allocated_buffers)
	{
		
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
	//unsigned int allocated_memory_usage = window_width*window_height*image_components;
	//allocated_memory_usage *= (video_frame_buffers.size() + buffers_being_allocated.size() + 1);
	//if( allocated_memory_usage >= max_preallocated_bytes)
	//{
		//THREADING_EXCEPTION("too much memory usage");
	//}

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
