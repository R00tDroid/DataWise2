#pragma once
#include <algorithm>
#include <vector>

namespace ParamaterParser
{
	struct Parameter
	{
		Parameter(std::string Name) : Name(Name){}

		std::string Name;
		std::vector<std::string> Arguments;

		bool operator == (const std::string& other)
		{
			return Name == other;
		}
	};

	struct ParameterList
	{
		std::vector<Parameter> Data;

		inline bool Contains(std::string to_check)
		{
			for (Parameter& parameter : Data)
			{
				if (parameter == to_check) return true;
			} 
			return false;
		}

		inline Parameter Get(std::string to_check) {
			for (Parameter& parameter : Data)
			{
				if (parameter == to_check) return parameter;
			}
			return Parameter(""); 
		}

		inline int HasArguments(std::string to_check) { return Get(to_check).Arguments.size(); }

		inline std::string GetArgument(std::string to_check) { if (HasArguments(to_check)) return Get(to_check).Arguments[0]; else return std::string(); }
		inline std::vector<std::string> GetArguments(std::string to_check) { return  Get(to_check).Arguments; }
		
	};

	inline ParameterList ParseParameters(int argc, char** arg)
	{
		ParameterList parameters;

		int i = 0;
		std::string entry;

		while (i < argc) 
		{
			entry = arg[i];
			
			if(std::string(entry)[0] == '-')
			{
				std::transform(entry.begin(), entry.end(), entry.begin(), ::tolower);
				parameters.Data.push_back(Parameter(entry.substr(1, entry.size() - 1)));
			} 
			else
			{
				if (!parameters.Data.empty())
				{
					Parameter& last_parameter = parameters.Data[parameters.Data.size() - 1];
					last_parameter.Arguments.push_back(entry);
				}
			}

			i++;
		}

		return parameters;
	}
}
