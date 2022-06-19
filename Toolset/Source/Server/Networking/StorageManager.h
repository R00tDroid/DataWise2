#pragma once
#include "../Networking/StorageConnectionBase.h"
#include <string>

class StorageManager
{
public:
	static StorageManager& Get();

	IStorageConnection* OpenConnection(std::string database_name);

private:
	static StorageManager instance_;
};
