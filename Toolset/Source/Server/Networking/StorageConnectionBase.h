#pragma once

#include "../../shared/Dependencies/picojson.h"
#include "../../shared/IO/Session.h"

class IStorageConnection
{
public:
	void Init(std::string, picojson::object&);
	virtual void Open() {}
	virtual void Close();

	virtual void LoadSettings() {}
	virtual void SaveSettings() {}

	virtual SessionInfo StoreSession(Session&) = 0; 
	virtual Session RetrieveSession(SessionInfo&) = 0;

	virtual std::vector<SessionInfo> GetSessionList() = 0;

	static IStorageConnection* SpawnConnectionForType(std::string Type);

	std::string GetDatabaseName();

protected:
	template<class T>
	T GetSetting(std::string name, T default_value)
	{
		if (settings_data_.count(name) == 0) return default_value;
		picojson::value value = settings_data_[name];
		if (!value.is<T>()) return default_value;
		return value.get<T>();
	}

	template<class T>
	void SetSetting(std::string name, T value)
	{
		settings_data_[name] = picojson::value(value);
	}

private:
	picojson::object settings_data_;
	std::string database_name_;
};

#define REGISTER_STORAGECONNECTION_CLASS(TYPE, NAME) namespace { static IStorageConnection* SpawnFunction##TYPE(){return new TYPE();} StorageConnectionClassRegistration TYPE ## _implreg(&SpawnFunction##TYPE, NAME); };

struct StorageConnectionClassRegistration
{
	StorageConnectionClassRegistration(IStorageConnection*(*create_function)(), std::string name);
};