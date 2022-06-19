#include "Log.h"

std::function<void(std::string)> devtool_log_callback_ = nullptr;

void DevTool::Log(std::string message)
{
	if (devtool_log_callback_ != nullptr) devtool_log_callback_(message);
}

void DevTool::SetLogCallback(std::function<void(std::string)> callback)
{
	devtool_log_callback_ = callback;
}
