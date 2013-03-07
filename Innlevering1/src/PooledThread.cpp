#include "PooledThread.h"


PooledThread::PooledThread()
{
	/*thread = std::make_shared<boost::thread>(ThreadLoop, this);*/
	thread = std::make_shared<boost::thread>(threadFunction);

}


PooledThread::~PooledThread()
{

}



void ThreadLoop(PooledThread* thread)
{
	bool running = true;

	while(running)
	{



	}
}
