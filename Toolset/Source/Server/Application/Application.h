#pragma once

class Application
{
public:
	static Application& Get();

	void Start();
	void Stop();

	void RequestShutdown();

	bool ShouldStop();
	
private:
	static Application instance_;

	bool should_stop_;
	bool running_;
};