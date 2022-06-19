#pragma once
#include <string>
#include <functional>

namespace DevTool {
	extern void Log(std::string);
	extern void SetLogCallback(std::function<void(std::string)>);
}