#include <iostream>
#include <algorithm>
#include <vector>
#include "Parameters.h"
#include "../Shared/Platform/Platform.h"
#include "../Shared/Dependencies/clsocket/ActiveSocket.h"
#include "../Shared/InstanceLock.h"
#include "../DevTools/DevConfigManager.h"
#include "../DevTools/ServerConnection.h"
#include "Log.h"

void Delay(float millis)
{
#ifdef _WIN32
	Sleep(millis);
#else
	usleep(millis * 1000);
#endif
}

int main(int argc, char** arg)
{
	ParamaterParser::ParameterList parameters = ParamaterParser::ParseParameters(argc, arg);

	if (parameters.Contains("help"))
	{
		Log("DataWise Command\n");
		Log("-start\t\t\t\tStart server");
		Log("-stop\t\t\t\tStop server");
		Log("-login <username> <password>\tSetup credentials");
		Log("-compile <database> [filter1;filter2;...] <export name> <type1:stage1,type2:stage2,...> \tRequest data compilation");

		return 0;
	}

	if (parameters.Contains("start"))
	{
		InstanceLock lock("server");

		if(lock.IsLocked())
		{
			Log("A server instance is already running");
			return -1;
		}

		DevTool::DevtoolConfigurationManager::Get().Load();

		Log("Starting server...");

		std::string file = GetExecutableDirectory() + "\\server" + GetExecutableExtension();
		RunExecutable(file);

		int tries = 0;
		while (tries < 10)
		{
			Delay(1000);

			DevTool::Server server;
			if (server.Connect())
			{
				server.Disconnect(); 
				Log("Server has started successfully"); 
				return 0;
			}

			Log("Attempted to connect to server on port " + std::to_string(DevTool::DevtoolConfigurationManager::Get().ServerPort) + ". Failure " + std::to_string(tries + 1));
			tries++;
		}

		Log("Failed to connect to server");
		return -1;
	}

	if (parameters.Contains("stop"))
	{
		DevTool::DevtoolConfigurationManager::Get().Load();
		DevTool::Server server;
		if (server.Connect())
		{
			if (server.Login())
			{
				bool result = server.Stop();
				server.Disconnect();

				if(result) Log("Requested server to shutdown"); else Log("Server refused shutdown request");
				return 0;
			}
			else
			{
				Log("Failed to authorize on server");
				return -1;
			}
		} else
		{
			Log("Could not connect to server");
			return -1;
		}
	}

	if (parameters.Contains("login"))
	{
		if (parameters.GetArguments("login").size() == 2) 
		{
			DevTool::DevtoolConfigurationManager::Get().Load();

			DevTool::DevtoolConfigurationManager::Get().LoginUserName = parameters.GetArguments("login")[0];
			DevTool::DevtoolConfigurationManager::Get().LoginPassword = parameters.GetArguments("login")[1];
			DevTool::DevtoolConfigurationManager::Get().Save();

			Log("Username and password have been saved");

			DevTool::Server server;
			if (server.Connect())
			{
				if(server.Login())
				{
					Log("Authorized successfully");
				} else
				{
					Log("Failed to authorize on server");
				}

				server.Disconnect();
			} else
			{
				Log("Could not connect to server");
			}

			return 0;
		}
		else
		{
			Log("-login requires two arguments\nSee -help for more information");
			return -1;
		}
	}

	if (parameters.Contains("compile"))
	{
		if (parameters.GetArguments("compile").size() == 3 || parameters.GetArguments("compile").size() == 4)
		{
			std::string target_database = parameters.GetArguments("compile")[0];
			std::string stage_blob;
			std::string export_name;
			std::string filters;

			if (parameters.GetArguments("compile").size() == 3)
			{
				export_name = parameters.GetArguments("compile")[1];
				stage_blob = parameters.GetArguments("compile")[2];
			}
			else if (parameters.GetArguments("compile").size() == 4)
			{
				filters = parameters.GetArguments("compile")[1];
				export_name = parameters.GetArguments("compile")[2];
				stage_blob = parameters.GetArguments("compile")[3];
			}

			std::vector<std::string> stages = SplitString(stage_blob, ',');
			std::vector<CompilationStageInfo> stage_infos;

			for(std::string stage : stages)
			{
				std::vector<std::string> stage_info = SplitString(stage, ':');
				if(stage_info.size() == 2)
				{
					stage_infos.push_back({ stage_info[0], stage_info[1] });
				}
			}

			if(stage_infos.empty())
			{
				Log("No valid compilation stage definitions found\nSee -help for more information");
				return -1;
			}

			DevTool::DevtoolConfigurationManager::Get().Load();

			DevTool::Server server;
			if (server.Connect())
			{
				if (server.Login())
				{
					std::string compiler_log;
					ResultTypes result = server.RemoteCompile(target_database, filters, export_name, stage_infos, compiler_log);
					if (result == REQUEST_ACCEPTED)
					{
						Log("Compiled successfully:");
						Log(compiler_log);
						return 0;
					}
					else if (result == REQUEST_ERROR)
					{
						Log("A compilation error occured:");
						Log(compiler_log);
						return -1;
					}

					Log("An unknown error occured");
					return -1;
				}

				Log("Failed to authorize on server");
				return -1;
			}

			Log("Failed to connect to server");
			return -1;
		}
		else
		{
			Log("-compile requires two arguments\nSee -help for more information");
			return -1;
		}
	}

	Log("No command specified\nSee -help for more information");
	return -1;
}
