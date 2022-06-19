#pragma once
#include "Platform/Platform.h"
#include <string>

class InstanceLock
{
public:
	InstanceLock(std::string name)
	{
		filename_ = GetPreferencesPath() + "\\" + name + ".lock";
	}

	bool Aquire()
	{
		bool success = fopen_s(&file_, filename_.c_str(), "w") == 0;
		if (!success) file_ = nullptr;
		return success;
	}

	void Release()
	{
		if (file_ != nullptr) fclose(file_);
		file_ = nullptr;
		remove(filename_.c_str());
	}

	bool IsLocked()
	{
		FILE* f;
		bool success = fopen_s(&f, filename_.c_str(), "w") == 0;
		if(success) fclose(f);

		return !success;
	}

private:
	std::string filename_;
	FILE* file_ = nullptr;
};
