#pragma once
#include <thread>
#include <mutex>

class ThreadBase
{
public:
	virtual void Start() = 0;
	virtual void Loop() = 0;
	virtual void Cleanup() = 0;

	bool IsRunning();
	void RequestEnd();

	void Dispatch();

	void Lock();
	void Unlock();

private:
	void Exec();

	bool ShouldStop();

	std::thread* thread_;
	std::mutex thread_info_lock;

	bool is_running_ = false;
	bool should_stop_ = false;
};
