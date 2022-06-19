#pragma once
#include "../Dependencies/clsocket/ActiveSocket.h"
#include "TCPMessages.h"

struct PacketHeader
{
	char id[2];
	uint16 packet_size;
	uint8 packet_type;

	uint8 padding_;
};

inline PacketHeader CreatePacketHeader(PacketType type, uint16 data_size)
{
	PacketHeader header;
	header.id[0] = 'D';
	header.id[1] = 'W';
	header.packet_type = type;
	header.packet_size = sizeof(PacketHeader) + data_size;

	return header;
}