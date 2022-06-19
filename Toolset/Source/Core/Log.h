#pragma once
#include <string>
#include <functional>

enum LogType
{
	Info,
	Warning,
	Error
};

extern void DWLog(std::string, LogType type = Info);
extern void SetLogReceiver(std::function<void(int, char*)>);