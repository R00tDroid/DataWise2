#pragma once

enum PacketType
{
	DevCommand,
	SessionEventData
};


enum EventDataType
{
	PacketData,
	StorageTarget,
	MetaTag
};

enum DevCommandTypes
{
	Login,
	CheckAuthority,
	Shutdown,
	RequestCompile
};

enum ResultTypes
{
	AUTHORIZED_NO,
	AUTHORIZED_YES,
	REQUEST_DENIED,
	REQUEST_ACCEPTED,
	REQUEST_ERROR
};
