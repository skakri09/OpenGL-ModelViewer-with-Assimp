/********************************************************************
    created:    5:3:2013   21:28
    filename:   PooledThread.h
    author:     Kristian Skarseth
    
    purpose:    
*********************************************************************/
#ifndef PooledThread_h__
#define PooledThread_h__

#include <boost/thread/thread.hpp>
#include <boost/function.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>

class PooledThread
{
public:
    PooledThread();
    ~PooledThread();

	void ScheduleTask();
protected:

private:
	std::shared_ptr<boost::thread> thread;

	boost::function<void()> threadFunction;
	boost::asio::io_service service;
};

void ThreadLoop(PooledThread* thread);

#endif // PooledThread_h__