#include "VideoThreadedUtil.h"



void AllocateFramesBuffer(vfb_ptr video_frame_buffer)
{
	//// Locking the VideoFrameBuffer to be sure noone else is messing with it
	//video_frame_buffer->vfb_mutex.lock();

	//// Creating a local version/ptr to the needed information for readability
	//int				_type	=	video_frame_buffer->_type;
	//cv::Size*		size	=	&video_frame_buffer->window_size;
	//vf_deque_ptr	dq		=	video_frame_buffer->video_frames_buffer;

	//// Creating the VideoFrame objects
	//for(unsigned int i=0; i < video_frame_buffer->alloc_size; i++)
	//	dq->push_back(std::make_shared<VideoFrame>(*size, _type));

	////cleanup
	//size	= NULL;
	//dq		= NULL;

	//// Unlocking the mutex, allowing other threads to mess with the VideoFrameBuffer
	//video_frame_buffer->vfb_mutex.unlock();

	// placeholder thread. Use actual thread !
	boost::thread::id thread_id;
	video_frame_buffer->Lock_AllocAndInit(thread_id);
}


void WriteFramesToDisk( vfb_ptr video_frame_buffer, vwm_ptr video_writer, bool flip )
{
	//placeholder for compiling. Change thread to actual thread:
	boost::thread::id thread_id;
	video_writer_ptr vid_writer = video_writer->Lock_GetVideoWriter(thread_id);
	vf_deque_ptr vf_dq_ptr = video_frame_buffer->Lock_GetVideoFrameBuffer(thread_id);

	for(unsigned int i = 0; i < video_frame_buffer->video_frames_buffer->size(); i++)
	{
		vf_ptr p = video_frame_buffer->video_frames_buffer->at(i);

		if(flip)
			p->Flip();

		*video_writer->video_writer << *p->image;
	}

	video_writer->vw_mutex.unlock();
	video_frame_buffer->vfb_mutex.unlock();
}
