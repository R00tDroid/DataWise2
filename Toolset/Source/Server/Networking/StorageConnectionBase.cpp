#include "StorageConnectionBase.h"
#include "../Application/ConfigurationManager.h"
#include "../Application/Log.h"

std::map<std::string, IStorageConnection*(*)()> connection_spawners_;

void IStorageConnection::Init(std::string Name, picojson::object& settings)
{
	settings_data_ = settings;

	LoadSettings();
	settings_data_ = {};
	SaveSettings();
	ConfigurationManager::Get().Connections[Name].Settings = settings_data_;
	ConfigurationManager::Get().Save();

	database_name_ = Name;

	Open();
}

void IStorageConnection::Close()
{
	delete this;
}

IStorageConnection* IStorageConnection::SpawnConnectionForType(std::string Type)
{
	if (connection_spawners_.count(Type) == 0) { Log("No connection class registered for " + Type); return nullptr; }
	return connection_spawners_[Type]();
}

std::string IStorageConnection::GetDatabaseName()
{
	return database_name_;
}

StorageConnectionClassRegistration::StorageConnectionClassRegistration(IStorageConnection*(*create_function)(), std::string name)
{
	connection_spawners_.insert(std::pair<std::string, IStorageConnection*(*)()>(name, create_function));
}
