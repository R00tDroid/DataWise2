#include "Log.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

void Log(std::string text)
{
	std::cout << text.c_str() << std::endl;

#ifdef _WIN32
	OutputDebugStringA((text + "\n").c_str());
#endif
}
