#pragma once
#include <string>

namespace DevTool 
{
	class DevtoolConfigurationManager
	{
	public:
		static DevtoolConfigurationManager& Get();

		void Load();
		void Save();

		int ServerPort;
		std::string ServerAddress;
		std::string LoginUserName;
		std::string LoginPassword;

	private:
		static DevtoolConfigurationManager instance_;
	};
}