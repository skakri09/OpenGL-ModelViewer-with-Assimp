#include "VideoThreadedUtil.h"



void AllocateFramesBuffer(video_frame_buffer_ptr video_frame_buffer)
{
	// Locking the VideoFrameBuffer to be sure noone else is messing with it
	video_frame_buffer->vfb_mutex.lock();

	// Creating a local version/ptr to the needed information for readability
	int						_type	=	video_frame_buffer->_type;
	cv::Size*				size	=	&video_frame_buffer->window_size;
	video_frame_deque_ptr	dq		=	video_frame_buffer->video_frames_buffer;
	
	// Creating the VideoFrame objects
	for(unsigned int i=0; i < video_frame_buffer->alloc_size; i++)
		dq->push_back(std::make_shared<VideoFrame>(*size, _type));

	//cleanup
	size	= NULL;
	dq		= NULL;
	
	// Unlocking the mutex, allowing other threads to mess with the VideoFrameBuffer
	video_frame_buffer->vfb_mutex.unlock();
}


void WriteFramesToDisk( video_frame_buffer_ptr video_frame_buffer, video_writer_ptr video_writer, bool flip )
{
	video_frame_buffer->vfb_mutex.lock();
	video_writer->vwm_mutex.lock();
	for(unsigned int i = 0; i < video_frame_buffer->video_frames_buffer->size(); i++)
	{
		video_frame_ptr p = video_frame_buffer->video_frames_buffer->at(i);
		
		if(flip)
			p->Flip();
		
		*video_writer->video_writer << *p->image;
	}

	video_writer->vwm_mutex.unlock();
	video_frame_buffer->vfb_mutex.unlock();
}
