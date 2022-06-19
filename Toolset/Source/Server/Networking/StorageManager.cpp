#include "StorageManager.h"
#include "../Application/ConfigurationManager.h"
#include "../Application/Log.h"
#include "StorageConnectionFTP.h"

StorageManager StorageManager::instance_;

StorageManager& StorageManager::Get()
{
	return instance_;
}

IStorageConnection* StorageManager::OpenConnection(std::string database_name)
{
	if(ConfigurationManager::Get().Connections.count(database_name) != 0)
	{
		ConnectionInfo info = ConfigurationManager::Get().Connections[database_name];

		IStorageConnection* con = IStorageConnection::SpawnConnectionForType(info.TypeName);
		if (con == nullptr) { return  nullptr; }
		con->Init(database_name, info.Settings);
		return con;
	}

	Log("Could not find connection configuration for '" + database_name + "'");
	return nullptr;
}
