#include "SimplePacketBuilder.h"
#include "PacketBuilder.h"
#include "SimpleRecordableSession.h"
#include <map>
#include "Log.h"

std::map<int, PacketBuilder*> builders;

int UniqueBuilderId()
{
	while(true)
	{
		int id = 1000 + rand() % 8999;
		if (builders.find(id) == builders.end()) return id;
	}
}

PacketBuilder* GetBuilder(int id)
{
	if (builders.find(id) == builders.end()) { DWLog("Packet builder does not exist: " + std::to_string(id)); return nullptr; }
	return builders[id];
}


int CreatePacketBuilder(int session, char* PacketName)
{
	if(GetSessionPtr(session) == nullptr)
	{
		DWLog("Session does not exist: " + std::to_string(session));
		return -1;
	}

	int id = UniqueBuilderId();
	builders.insert(std::pair<int, PacketBuilder*>(id, new PacketBuilder((RecordableSession*)GetSessionPtr(session), std::string(PacketName))));

	return id;
}

void SendPacket(int id)
{
	PacketBuilder* builder = GetBuilder(id);
	if (builder == nullptr) return;

	builder->Send();

	delete builder;

	builders.erase(builders.find(id));
}

void AddInt(int id, int value)
{
	PacketBuilder* builder = GetBuilder(id);
	if (builder == nullptr) return;

	builder->Add(value);
}

void AddFloat(int id, float value)
{
	PacketBuilder* builder = GetBuilder(id);
	if (builder == nullptr) return;

	builder->Add(value);
}

void AddString(int id, char* value)
{
	PacketBuilder* builder = GetBuilder(id);
	if (builder == nullptr) return;

	builder->Add(value);
}

void AddTime(int id)
{
	PacketBuilder* builder = GetBuilder(id);
	if (builder == nullptr) return;

	builder->AddTimestamp();
}
