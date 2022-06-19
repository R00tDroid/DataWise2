#include "Log.h"
#include <iostream>
#include "../../Shared/Platform/Platform.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#endif

FILE* log_file_;
int log_day = -1;

std::mutex logging_lock;

void Log(std::string text)
{
	logging_lock.lock();

	bool should_open_log = log_file_ == nullptr;

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	time_t time = std::chrono::system_clock::to_time_t(now);
	tm local_time = *localtime(&time);

	if (local_time.tm_mday != log_day) { logging_lock.unlock(); StopLog(); logging_lock.lock(); should_open_log = true; }

	if(should_open_log)
	{
		std::stringstream filename;

		CreateDirectoryTree(GetPreferencesPath() + "\\Logs");

		filename << GetPreferencesPath() << "\\Logs\\server_" << std::setfill('0') << std::setw(2) << local_time.tm_mday << "_" << std::setfill('0') << std::setw(2) << (local_time.tm_mon + 1) << "_" << std::setfill('0') << std::setw(4) << (local_time.tm_year + 1900) << ".log";
		fopen_s(&log_file_, filename.str().c_str(), "a");
	}

	if (log_file_ != nullptr)
	{
		std::stringstream log_stream;
		log_stream << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "\t" << text << "\n";

		std::string log_line = log_stream.str();
		fwrite((void*)log_line.c_str(), sizeof(char), log_line.length(), log_file_);
		fflush(log_file_);
	}

	std::cout << text.c_str() << std::endl;

#ifdef _WIN32
	OutputDebugStringA((text + "\n").c_str());
#endif

	logging_lock.unlock();
}

void StopLog()
{
	logging_lock.lock();
	if (log_file_ != nullptr)
	{
		fclose(log_file_);
		log_file_ = nullptr;
	}
	logging_lock.unlock();
}
