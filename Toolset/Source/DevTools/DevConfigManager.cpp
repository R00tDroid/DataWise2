#include "DevConfigManager.h"
#include "../Shared/Dependencies/picojson.h"
#include "../Shared/Platform/Platform.h"
#include "Log.h"

DevTool::DevtoolConfigurationManager DevTool::DevtoolConfigurationManager::instance_;

DevTool::DevtoolConfigurationManager& DevTool::DevtoolConfigurationManager::Get()
{
	return instance_;
}

void DevTool::DevtoolConfigurationManager::Load()
{
	// Defaults:
	ServerPort = 5438;
	ServerAddress = "127.0.0.1";
	LoginUserName = "";
	LoginPassword = "";


	// Load settings if possible
	std::string file = GetPreferencesPath() + "\\dev.json";

	FILE* f;
	if (!fopen_s(&f, file.c_str(), "rb"))
	{
		fseek(f, 0, SEEK_END);
		long file_length = ftell(f);
		fseek(f, 0, SEEK_SET);

		std::string data;
		data.resize(file_length);

		fread((void*)data.data(), sizeof(char), file_length, f);

		fclose(f);

		picojson::value blob;
		picojson::parse(blob, data);

		if (blob.is<picojson::object>())
		{
			picojson::object root = blob.get<picojson::object>();

			for (picojson::value::object::const_iterator root_section = root.begin(); root_section != root.end(); ++root_section)
			{
				std::string name = root_section->first;
				picojson::value data = root_section->second;

				if (name == std::string("port") && data.is<double>())
				{
					ServerPort = data.get<double>();
				}
				else if (name == std::string("address") && data.is<std::string>())
				{
					ServerAddress = data.get<std::string>();
				}
				else if (name == std::string("User") && data.is<picojson::object>())
				{
					picojson::object user_section = data.get<picojson::object>();

					for (picojson::value::object::const_iterator server_entry = user_section.begin(); server_entry != user_section.end(); ++server_entry)
					{
						std::string entry_name = server_entry->first;
						picojson::value entry_data = server_entry->second;

						if (entry_name == std::string("name") && entry_data.is<std::string>())
						{
							LoginUserName = entry_data.get<std::string>();
						}
						else if (entry_name == std::string("password") && entry_data.is<std::string>())
						{
							LoginPassword = entry_data.get<std::string>();
						}
						else
						{
							DevTool::Log("Unknown server configuration entry for user: " + entry_name);
						}
					}
				}
				else
				{
					DevTool::Log("Unknown server configuration section: " + name);
				}
			}
		}
		else
		{
			DevTool::Log("Could not read server configuration");
		}
	}
	else
	{
		Save();
	}


}

void DevTool::DevtoolConfigurationManager::Save()
{
	std::string file = GetPreferencesPath() + "\\dev.json";

	FILE* f;
	if (!fopen_s(&f, file.c_str(), "wb"))
	{
		picojson::object root;
		picojson::object user_section;

		root["port"] = picojson::value((double)ServerPort);
		root["address"] = picojson::value(ServerAddress);
		user_section["name"] = picojson::value(LoginUserName);
		user_section["password"] = picojson::value(LoginPassword);

		root["User"] = picojson::value(user_section);

		std::string data = picojson::value(root).serialize();

		fwrite(data.c_str(), sizeof(char), data.length(), f);
		fclose(f);
	}
	else
	{
		Log("Could not save server configuration to: " + file);
	}
}
