#include "Session.h"
#include "../Dependencies/picojson.h"
#include "../../Server/Networking/StorageConnectionBase.h"
#include "../Platform/Platform.h"

bool MetaFilter::MatchesFilter(std::string Filter, std::string Value, FilterOperator Operator)
{
	switch (Operator)
	{
	case EQUALS: { return std::atof(Filter.c_str()) == std::atof(Value.c_str()); break; }
	case NOT_EQUALS: { return std::atof(Filter.c_str()) != std::atof(Value.c_str()); break; }
	case GREATER: { return std::atof(Value.c_str()) > std::atof(Filter.c_str()); break; }
	case LESS: { return std::atof(Value.c_str()) < std::atof(Filter.c_str()); break; }
	case GREATER_EQUALS: { return std::atof(Value.c_str()) >= std::atof(Filter.c_str()); break; }
	case LESS_EQUALS: { return std::atof(Value.c_str()) <= std::atof(Filter.c_str()); break; }
	}

	return false;
}

bool MetaFilter::Matches(std::map<std::string, std::string>& Meta)
{
	bool contains = Meta.find(Parameter) != Meta.end();

	if (Operator == EXIST)
	{
		return contains;
	}
	if (Operator == NOT_EXIST)
	{
		return !contains;
	}
	if (contains)
	{
		std::string meta_data = "0";
		picojson::value meta_value;
		picojson::parse(meta_value, Meta[Parameter]);
		if(meta_value.is<picojson::array>())
		{
			picojson::array& meta_array = meta_value.get<picojson::array>();
			if (meta_array.size() == 2)
			{
				if (meta_array[1].is<double>()) meta_data = std::to_string(meta_array[1].get<double>());
				else if (meta_array[1].is<std::string>()) meta_data = meta_array[1].get<std::string>();
			}
		}

		return MatchesFilter(Filter, meta_data, Operator);
	}
	else
	{
		return false;
	}
}

std::vector<MetaFilter> MetaFilter::Parse(std::string filter_blob)
{
	std::vector<MetaFilter> filters;
	std::vector<std::string> filter_list = SplitString(filter_blob, ';');

	for(std::string& filter : filter_list)
	{
		filter = ReplaceString(filter, " =", "=");
		filter = ReplaceString(filter, "= ", "=");
		filter = ReplaceString(filter, "! ", "!");
		filter = ReplaceString(filter, " !", "!");
		filter = ReplaceString(filter, " >", ">");
		filter = ReplaceString(filter, "> ", ">");
		filter = ReplaceString(filter, " <", "<");
		filter = ReplaceString(filter, "< ", "<");
		
		if(MatchStringPattern(filter, "*!=*"))
		{
			std::vector<std::string> params = SplitString(filter, "!=");
			filters.push_back(MetaFilter(params[0], params[1], NOT_EQUALS));
		}
		else if (MatchStringPattern(filter, "*>=*"))
		{
			std::vector<std::string> params = SplitString(filter, ">=");
			filters.push_back(MetaFilter(params[0], params[1], GREATER_EQUALS));
		}
		else if (MatchStringPattern(filter, "*<=*"))
		{
			std::vector<std::string> params = SplitString(filter, "<=");
			filters.push_back(MetaFilter(params[0], params[1], LESS_EQUALS));
		}
		else if (MatchStringPattern(filter, "*>*"))
		{
			std::vector<std::string> params = SplitString(filter, ">");
			filters.push_back(MetaFilter(params[0], params[1], GREATER));
		}
		else if (MatchStringPattern(filter, "*<*"))
		{
			std::vector<std::string> params = SplitString(filter, "<");
			filters.push_back(MetaFilter(params[0], params[1], LESS));
		}
		else if (MatchStringPattern(filter, "*=*"))
		{
			std::vector<std::string> params = SplitString(filter, "=");
			filters.push_back(MetaFilter(params[0], params[1], EQUALS));
		}
		else if (MatchStringPattern(filter, "*!"))
		{
			filters.push_back(MetaFilter(ReplaceString(filter, "!", ""), "", NOT_EXIST));
		}
		else if(filter.find("=") == std::string::npos && filter.find("!") == std::string::npos && filter.find(">") == std::string::npos && filter.find("<") == std::string::npos)
		{
			filters.push_back(MetaFilter(filter, "", EXIST));
		}
	}

	return filters;
}

std::vector<std::string> Session::GetPacketsOfType(std::string type)
{
	std::vector<std::string> packets;
	for (std::string packet_string : Packets)
	{
		picojson::value packet_value;
		picojson::parse(packet_value, packet_string);
		picojson::array packet = packet_value.get<picojson::array>();
		if (packet[0].get<std::string>() == type) packets.push_back(packet_string);
	}

	return packets;
}

std::vector<std::string> Session::GetPacketsContaining(std::string property_name)
{
	std::vector<std::string> packets;
	for (std::string packet_string : Packets)
	{
		picojson::value packet_value;
		picojson::parse(packet_value, packet_string);
		picojson::array& packet = packet_value.get<picojson::array>();
		for (int i = 1; i < packet.size(); i++)
		{
			picojson::array& property_field = packet[i].get<picojson::array>();
			if(property_field[0].is<std::string>())
			{
				if(property_field[0].get<std::string>() == property_name)
				{
					packets.push_back(packet_string);
				}
			}
		}
	}

	return packets;
}
