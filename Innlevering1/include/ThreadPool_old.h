/********************************************************************
    created:    5:3:2013   18:21
    filename:   ThreadPool.h
    author:     Kristian Skarseth
    
    purpose:    
*********************************************************************/
#ifndef ThreadPool_old_h__
#define ThreadPool_old_h__

#include <deque>

#include <boost/thread/thread.hpp>


class ThreadPool_old
{
public:
	static ThreadPool_old* Inst();

	void Init();

	std::shared_ptr<boost::thread> GetThread();

	boost::mutex* GetThreadPoolMutex();

	void PushBackThreadToPool(std::shared_ptr<boost::thread> thread);

protected:

private:
	ThreadPool_old();
	~ThreadPool_old();
	
	boost::thread threadHandler;

	boost::mutex thread_pool_mutex;
	unsigned int core_number;
	std::deque<std::shared_ptr<boost::thread>> thread_pool;
};

void HandleThreadInUse(std::shared_ptr<boost::thread> thread);

#endif // ThreadPool_old_h__