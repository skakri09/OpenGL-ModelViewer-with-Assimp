#include "ThreadPool_old.h"

ThreadPool_old* ThreadPool_old::Inst()
{
	static ThreadPool_old* instance = new ThreadPool_old();
	return instance;
}

ThreadPool_old::ThreadPool_old()
{
}

ThreadPool_old::~ThreadPool_old()
{
	//waiting for all threads to finish execution
	while(thread_pool.size() < core_number)
		continue;

	thread_pool_mutex.lock();
	for(unsigned int i = 0; i < core_number; i++)
		thread_pool.at(i)->detach();
}

void ThreadPool_old::Init()
{
	thread_pool_mutex.lock();
	core_number = boost::thread::hardware_concurrency()-1;
	for(unsigned int i = 0; i < core_number; i++)
		thread_pool.push_back(std::make_shared<boost::thread>());
	thread_pool_mutex.unlock();
}

std::shared_ptr<boost::thread>ThreadPool_old::GetThread()
{
	std::shared_ptr<boost::thread> ret_thread;

	bool found_a_thread = false;
	while(!found_a_thread)
	{
		thread_pool_mutex.lock();
		if(thread_pool.size() < 1)
		{
			thread_pool_mutex.unlock();
			continue;
		}
		else
		{
			ret_thread = thread_pool.at(0);//getting the "front" element
			thread_pool.pop_front();
			found_a_thread = true;
			thread_pool_mutex.unlock();
		}
	}

	threadHandler = boost::thread(HandleThreadInUse, ret_thread);

	return ret_thread;
}

boost::mutex* ThreadPool_old::GetThreadPoolMutex()
{
	return &thread_pool_mutex;
}

void ThreadPool_old::PushBackThreadToPool( std::shared_ptr<boost::thread> thread )
{
	thread_pool.push_back(thread);
}


void HandleThreadInUse( std::shared_ptr<boost::thread> thread)
{
	thread->join();
	ThreadPool_old::Inst()->GetThreadPoolMutex()->lock();
	ThreadPool_old::Inst()->PushBackThreadToPool(thread);
	ThreadPool_old::Inst()->GetThreadPoolMutex()->unlock();
}
