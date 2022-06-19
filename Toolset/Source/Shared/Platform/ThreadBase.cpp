#include "ThreadBase.h"

bool ThreadBase::IsRunning()
{
	bool state;
	Lock();
	state = is_running_;
	Unlock();

	return state;
}

void ThreadBase::RequestEnd()
{
	Lock();
	should_stop_ = true;
	Unlock();
}

void ThreadBase::Dispatch()
{
	thread_ = new std::thread(&ThreadBase::Exec, this);
}

void ThreadBase::Lock()
{
	thread_info_lock.lock();
}

void ThreadBase::Unlock()
{
	thread_info_lock.unlock();
}

void ThreadBase::Exec()
{
	Lock();
	is_running_ = true;
	Unlock();

	Start();

	while(!ShouldStop())
	{
		Loop();
		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
	}

	Cleanup();

	Lock();
	is_running_ = false;
	Unlock();
}

bool ThreadBase::ShouldStop()
{
	bool state;
	Lock();
	state = should_stop_;
	Unlock();

	return state;
}
