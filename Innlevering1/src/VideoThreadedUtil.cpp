#include "VideoThreadedUtil.h"


void WriteFramesToDisk(std::deque<cv::Mat*> Frames,
						unsigned int window_width, 
						unsigned int window_height,
						unsigned int startSaveindex,
						std::string videoSubFolder)
{

}

std::deque<cv::Mat*>* AllocateFramesBuffer( unsigned int window_width, unsigned int window_height, 
										  unsigned int image_components, unsigned int allocationSize )
{
	std::deque<cv::Mat*>* frameBuffer = new std::deque<cv::Mat*>();
	frameBuffer->resize(allocationSize);
	for(unsigned int i=0; i < frameBuffer->size(); i++)
		(*frameBuffer)[i] = new cv::Mat(window_width, window_height, image_components);

	return frameBuffer;
}

void AllocateFramesBuffer( std::deque<VideoFrame*>* target_buffer, 
						   cv::Size window_size, 
						   int _type, 
						   unsigned int alloc_size )
{

	target_buffer->resize(alloc_size);
}
