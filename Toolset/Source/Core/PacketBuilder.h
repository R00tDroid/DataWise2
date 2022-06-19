#pragma once
#include "Export.h"
#include "RecordableSession.h"
#include <sstream>

class EXPORTED PacketBuilder
{
public:
	PacketBuilder(RecordableSession* session, std::string packet_id);

	template<class T>
	void Add(T data);

	template<>
	void Add<float>(float data) { AddData("float", ToString(data)); }

	template<>
	void Add<int>(int data) { AddData("int", std::to_string(data)); }

	template<>
	void Add<std::string>(std::string data) { AddData("string", data, true); }

	template<>
	void Add<char*>(char* data) { AddData("string", data, true); }

	template<>
	void Add<char const*>(char const* data) { AddData("string", data, true); }

	void AddTimestamp();

	void Send();

private:
	void AddData(std::string type, std::string data, bool stringify = false);

	template<class T>
	std::string ToString(T data)
	{
		std::ostringstream stream;
		stream << data;
		return stream.str();
	}

	std::string data_;
	RecordableSession* session_;
};
