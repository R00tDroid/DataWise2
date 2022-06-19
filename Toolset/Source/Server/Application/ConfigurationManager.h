#pragma once
#include <string>
#include <vector>
#include <map>
#include "../../Shared/Dependencies/picojson.h"
#include "../CompilationManager.h"

struct PacketMemberDefinition
{
	std::string Name;
	std::string Type;
};

struct PacketDefinition
{
	std::string Name;
	std::vector<PacketMemberDefinition> Members;
};

struct User
{
	std::string Name;
	std::string Password;
};

struct ConnectionInfo
{
	std::string TypeName;
	picojson::object Settings;

	std::map<std::string, PacketDefinition> PacketDefinitions;
};

class ConfigurationManager
{
public:
	static ConfigurationManager& Get();

	void Load();
	void Save();

	bool IsValidUser(std::string Name, std::string Password);

	int ServerPort;
	std::vector<User> Users;
	std::map<std::string, ConnectionInfo> Connections;
	std::vector<ScheduledCompilationTask> CompilationSchedule;

private:
	static ConfigurationManager instance_;
};