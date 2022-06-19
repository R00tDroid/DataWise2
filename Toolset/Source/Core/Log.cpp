#include "Log.h"
#include <iostream>
#include <windows.h>

std::function<void(int, char*)> log_callback = nullptr;

void DWLog(std::string text, LogType type)
{
	char t = ' ';

	switch (type) { 
	case Info: { t = 'I'; break; }
	case Warning: { t = 'W'; break; }
	case Error: { t = 'E'; break; }
	}

	std::cout << "_DWLog (" << t << ") > " << text.c_str() << std::endl;
	OutputDebugStringA(("_DWLog (" + std::string(1, t) + ") > " + text + "\n").c_str());

	if(log_callback != nullptr)	log_callback(type, const_cast<char*>(text.c_str()));
}

void SetLogReceiver(std::function<void(int, char*)> callback)
{
	log_callback = callback;
}
