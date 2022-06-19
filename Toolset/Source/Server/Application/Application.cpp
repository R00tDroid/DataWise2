#include "Application.h"

Application Application::instance_;

Application& Application::Get()
{
	return instance_;
}

void Application::Start()
{
	running_ = true;
	should_stop_ = false;
}

void Application::Stop()
{
	running_ = false;
	should_stop_ = false;
}

void Application::RequestShutdown()
{
	should_stop_ = true;
}

bool Application::ShouldStop()
{
	return running_ & should_stop_;
}
