#pragma once
#include "Export.h"
#include <string>

extern "C" 
{
	int EXPORTED CreatePacketBuilder(int session, char* PacketName);

	void EXPORTED AddInt(int id, int value);

	void EXPORTED AddFloat(int id, float value);

	void EXPORTED AddString(int id, char* value);

	void EXPORTED AddTime(int id);

	void EXPORTED SendPacket(int id);
}