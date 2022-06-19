#include "ConfigurationManager.h"
#include "../../Shared/Dependencies/picojson.h"
#include "../../Shared/Platform/Platform.h"
#include "Log.h"

ConfigurationManager ConfigurationManager::instance_;

ConfigurationManager& ConfigurationManager::Get()
{
	return instance_;
}

void ConfigurationManager::Load()
{
	// Defaults:
	ServerPort = 5438;
	Users = {};
	Connections = {};
	CompilationSchedule = {};

	// Load settings if possible
	std::string file = GetPreferencesPath() + "\\server.json";

	FILE* f;
	if(!fopen_s(&f, file.c_str(), "rb"))
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

		if(blob.is<picojson::object>())
		{
			picojson::object root = blob.get<picojson::object>();

			for (picojson::value::object::const_iterator root_section = root.begin(); root_section != root.end(); ++root_section)
			{
				std::string name = root_section->first;
				picojson::value data = root_section->second;

				if(name == std::string("TCP") && data.is<picojson::object>())
				{
					picojson::object server_section = data.get<picojson::object>();

					for (picojson::value::object::const_iterator server_entry = server_section.begin(); server_entry != server_section.end(); ++server_entry)
					{
						std::string entry_name = server_entry->first;
						picojson::value entry_data = server_entry->second;

						if(entry_name == std::string("port") && entry_data.is<double>())
						{
							ServerPort = entry_data.get<double>();
						}
						else
						{
							Log("Unknown server configuration entry for TCP: " + entry_name);
						}
					}
				}

				else if (name == std::string("Storage") && data.is<picojson::array>())
				{
					picojson::array storage_array = data.get<picojson::array>();
					for (picojson::value storage_entry : storage_array)
					{
						if(!storage_entry.is<picojson::value::object>()) { Log("Invalid storage object"); continue; }
						picojson::object storage_object = storage_entry.get<picojson::value::object>();

						if (storage_object.count("Name") == 0 || storage_object.count("Type") == 0 || storage_object.count("Settings") == 0 || storage_object.count("Packets") == 0) { Log("Invalid storage definition: missing element\n" + storage_entry.serialize(true)); continue; }
						if (!storage_object["Name"].is<std::string>() || !storage_object["Type"].is<std::string>() || !storage_object["Settings"].is<picojson::value::object>() || !storage_object["Packets"].is<picojson::array>()) { Log("Invalid storage definition: incorrect type\n" + storage_entry.serialize(true)); continue; }

						ConnectionInfo connection;
						connection.TypeName = storage_object["Type"].get<std::string>();
						connection.Settings = storage_object["Settings"].get<picojson::value::object>();

						picojson::array packet_types = storage_object["Packets"].get<picojson::array>();
						for(picojson::value& packet : packet_types)
						{
							if(packet.is<picojson::object>())
							{
								picojson::object packet_object = packet.get<picojson::object>();
								if (packet_object.find("Name") == packet_object.end() || packet_object.find("Members") == packet_object.end()) { Log("Invalid packet definition: missing member\n" + packet.serialize(true)); continue; }
								if (!packet_object["Name"].is<std::string>() || !packet_object["Members"].is<picojson::array>()) { Log("Invalid packet definition: incorrect member type\n" + packet.serialize(true)); continue; }

								PacketDefinition packet_def;
								packet_def.Name = packet_object["Name"].get<std::string>();

								for(picojson::value packet_member : packet_object["Members"].get<picojson::array>())
								{
									if(!packet_member.is<picojson::object>())
									{
										Log("Invalid packet member definition: incorrect member definition in " + packet_def.Name);
										continue;
									}

									picojson::object packet_member_object = packet_member.get<picojson::object>();
									if (packet_member_object.find("Name") == packet_member_object.end() || packet_member_object.find("Type") == packet_member_object.end()) { Log("Invalid packet member definition: missing data point\n" + packet_member.serialize(true)); continue; }
									if (!packet_member_object["Name"].is<std::string>() || !packet_member_object["Type"].is<std::string>()) { Log("Invalid packet member definition: incorrect data type\n" + packet_member.serialize(true)); continue; }

									PacketMemberDefinition packet_member_def;
									packet_member_def.Name = packet_member_object["Name"].get<std::string>();
									packet_member_def.Type = packet_member_object["Type"].get<std::string>();

									packet_def.Members.push_back(packet_member_def);
								}

								connection.PacketDefinitions.insert(std::pair<std::string, PacketDefinition>(packet_def.Name, packet_def));
							} else
							{
								Log("Invalid packet definition: incorrect structure");
							}
						}

						Connections.insert(std::pair<std::string, ConnectionInfo>(storage_object["Name"].get<std::string>(), connection));
					}
				}
				else if (name == std::string("User") && data.is<picojson::array>())
				{
					picojson::array user_array = data.get<picojson::array>();
					for(picojson::value user_value : user_array)
					{
						if (user_value.is<picojson::object>()) 
						{
							picojson::object user_object = user_value.get<picojson::value::object>();

							User target_user;

							for (picojson::value::object::const_iterator user_entry = user_object.begin(); user_entry != user_object.end(); ++user_entry)
							{
								std::string entry_name = user_entry->first;
								picojson::value entry_data = user_entry->second;

								if(entry_name == std::string("Name") && entry_data.is<std::string>())
								{
									target_user.Name = entry_data.get<std::string>();
								}
								else 
								if (entry_name == std::string("Password") && entry_data.is<std::string>())
								{
									target_user.Password = entry_data.get<std::string>();
								}
								else
								{
									Log("Invalid user data: " + entry_name);
								}
							}

							Users.push_back(target_user);
						} 
						else
						{
							Log("Invalid user entry");
						}
					}
				}
				else if (name == std::string("Compilation") && data.is<picojson::array>())
				{
				picojson::array compilation_schedule = data.get<picojson::array>();
				for (picojson::value schedule_value : compilation_schedule)
				{
					if (schedule_value.is<picojson::object>())
					{
						picojson::object schedule_object = schedule_value.get<picojson::value::object>();
						ScheduledCompilationTask* task = nullptr;

						if(schedule_object.find("Type") != schedule_object.end() && schedule_object.find("Frequency") != schedule_object.end())
						{
							if(schedule_object["Type"].is<std::string>() && schedule_object["Frequency"].is<double>() && schedule_object["ExportName"].is<std::string>())
							{
								std::string task_type = schedule_object["Type"].get<std::string>();

								if(task_type == std::string("Timed"))
								{
									task = new ScheduledCompilationTask(ScheduledCompilationTask::Timed);
									task->UpdateDelay = schedule_object["Frequency"].get<double>();
								}
								else if (task_type == std::string("Daily"))
								{
									task = new ScheduledCompilationTask(ScheduledCompilationTask::Daily);
									task->TimeOfDay = schedule_object["Frequency"].get<double>();
								}
								else if (task_type == std::string("Weekly"))
								{
									task = new ScheduledCompilationTask(ScheduledCompilationTask::Weekly);
									task->WeekDay = schedule_object["Frequency"].get<double>();
								}
							}
						}

						if (schedule_object.find("Filter") != schedule_object.end() && task != nullptr)
						{
							if (schedule_object["Filter"].is<std::string>())
							{
								task->Filter = schedule_object["Filter"].get<std::string>();
							}
						}

						if (schedule_object.find("ExportName") != schedule_object.end() && task != nullptr)
						{
							if (schedule_object["ExportName"].is<std::string>())
							{
								task->ExportName = schedule_object["ExportName"].get<std::string>();
							}
						}

						if (schedule_object.find("Databases") != schedule_object.end() && task!=nullptr)
						{
							if (schedule_object["Databases"].is<picojson::array>())
							{
								picojson::array& db_list = schedule_object["Databases"].get<picojson::array>();
								for(picojson::value& db_value : db_list)
								{
									if(db_value.is<std::string>())
									{
										task->Databases.push_back(db_value.get<std::string>());
									}
								}
							}
						}

						if (schedule_object.find("Stages") != schedule_object.end() && task != nullptr)
						{
							if (schedule_object["Stages"].is<picojson::array>())
							{
								picojson::array& stage_list = schedule_object["Stages"].get<picojson::array>();
								for (picojson::value& stage_value : stage_list)
								{
									if (stage_value.is<picojson::object>())
									{
										picojson::object& stage_object = stage_value.get<picojson::object>();
										if (stage_object.find("Type") != stage_object.end() && stage_object.find("Name") != stage_object.end())
										{
											if (stage_object["Type"].is<std::string>() && stage_object["Name"].is<std::string>())
											{
												task->Stages.push_back({ stage_object["Type"].get<std::string>(), stage_object["Name"].get<std::string>() });
											}
										}
									}
								}
							}
						}

						if (task != nullptr)
						{
							task->UpdateScheduling();
							CompilationSchedule.push_back(*task);
						}
					}
					else
					{
						Log("Invalid compilation entry");
					}
				}
				}
				else 
				{
					Log("Unknown server configuration section: " + name);
				}
			}
		}
		else
		{
			Log("Could not read server configuration");
		}
	}
	else
	{
		Save();
	}
}

void ConfigurationManager::Save()
{
	std::string file = GetPreferencesPath() + "\\server.json";

	FILE* f;
	if (!fopen_s(&f, file.c_str(), "wb"))
	{
		picojson::object root;
		picojson::object tcp_section;
		picojson::array user_section;
		picojson::array storage_section;
		picojson::array compilation_section;

		tcp_section["port"] = picojson::value((double)ServerPort);

		for (int i = 0; i < Users.size(); i++) 
		{
			picojson::object user;
			user["Name"] = picojson::value(Users[i].Name);
			user["Password"] = picojson::value(Users[i].Password);
			user_section.push_back(picojson::value(user));
		}

		for (auto connection_it = Connections.begin(); connection_it != Connections.end(); connection_it++)
		{
			picojson::object connection_object;
			connection_object["Name"] = picojson::value(connection_it->first);
			connection_object["Type"] = picojson::value(connection_it->second.TypeName);
			connection_object["Settings"] = picojson::value(connection_it->second.Settings);

			picojson::array packet_definitions;

			for (auto packet_def_it = connection_it->second.PacketDefinitions.begin(); packet_def_it != connection_it->second.PacketDefinitions.end(); packet_def_it++)
			{
				picojson::object packet_def;
				packet_def["Name"] = picojson::value(packet_def_it->first);
				picojson::array packet_members;

				for (PacketMemberDefinition& packet_member : packet_def_it->second.Members)
				{
					picojson::object packet_member_def;
					packet_member_def["Name"] = picojson::value(packet_member.Name);
					packet_member_def["Type"] = picojson::value(packet_member.Type);

					packet_members.push_back(picojson::value(packet_member_def));
				}

				packet_def["Members"] = picojson::value(packet_members);
				packet_definitions.push_back(picojson::value(packet_def));
			}

			connection_object["Packets"] = picojson::value(packet_definitions);


			storage_section.push_back(picojson::value(connection_object));
		}

		for (ScheduledCompilationTask& task : CompilationSchedule)
		{
			picojson::object task_object;
			picojson::array db_list;
			picojson::array stage_list;

			task_object["ExportName"] = picojson::value(task.ExportName);
			task_object["Filter"] = picojson::value(task.Filter);
			task_object["Type"] = picojson::value(std::string());
			task_object["Frequency"] = picojson::value();

			switch (task.GetFrequencyType()) {
			case ScheduledCompilationTask::Timed: {
					task_object["Type"] = picojson::value("Timed");
					task_object["Frequency"] = picojson::value((double)task.UpdateDelay);
					break; }
			case ScheduledCompilationTask::Daily: { 
					task_object["Type"] = picojson::value("Daily");
					task_object["Frequency"] = picojson::value((double)task.TimeOfDay);
					break; }
			case ScheduledCompilationTask::Weekly: {
					task_object["Type"] = picojson::value("Weekly");
					task_object["Frequency"] = picojson::value((double)task.WeekDay);
					break; }
			}

			for (std::string& db : task.Databases)
			{
				db_list.push_back(picojson::value(db));
			}

			for (CompilationStageInfo& stage : task.Stages)
			{
				picojson::object stage_object;
				stage_object["Type"] = picojson::value(stage.Type);
				stage_object["Name"] = picojson::value(stage.Name);
				stage_list.push_back(picojson::value(stage_object));
			}

			task_object["Databases"] = picojson::value(db_list);
			task_object["Stages"] = picojson::value(stage_list);

			compilation_section.push_back(picojson::value(task_object));
		}

		root["TCP"] = picojson::value(tcp_section);
		root["User"] = picojson::value(user_section);
		root["Storage"] = picojson::value(storage_section);
		root["Compilation"] = picojson::value(compilation_section);

		std::string data = picojson::value(root).serialize(true);

		fwrite(data.c_str(), sizeof(char), data.length(), f);
		fclose(f);
	}
	else
	{
		Log("Could not save server configuration to: " + file);
	}
}

bool ConfigurationManager::IsValidUser(std::string Name, std::string Password)
{
	for(User user : Users)
	{
		if (user.Name == Name && user.Password == Password) return true;
	}

	return false;
}
