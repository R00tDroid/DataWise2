#pragma once
#include <map>
#include <vector>

struct MetaFilter
{
	enum FilterOperator
	{
		EXIST,
		NOT_EXIST,
		EQUALS,
		GREATER,
		LESS,
		GREATER_EQUALS,
		LESS_EQUALS,
		NOT_EQUALS
	};

	MetaFilter(std::string Tag, std::string Value, FilterOperator Operator) : Parameter(Tag), Filter(Value), Operator(Operator) {}

	std::string Parameter;
	std::string Filter;
	FilterOperator Operator;

	static bool MatchesFilter(std::string Filter, std::string Value, FilterOperator Operator);
	bool Matches(std::map<std::string, std::string>&);

	static std::vector<MetaFilter> Parse(std::string);
};

struct SessionInfo
{
	std::string Database;
	std::string Name;
	std::map<std::string, std::string> Meta;
};

struct Session
{
	std::string Name;
	std::vector<std::string> Packets;
	std::map<std::string, std::string> Meta;

	std::vector<std::string> GetPacketsOfType(std::string);
	std::vector<std::string> GetPacketsContaining(std::string);
};
