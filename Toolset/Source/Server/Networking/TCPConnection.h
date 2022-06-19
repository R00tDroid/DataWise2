#pragma once
#include "../../Shared/Platform/ThreadBase.h"
#include "../../Shared/Dependencies/clsocket/ActiveSocket.h"
#include "../../Shared/IO/Session.h"
#include "../../Shared/ByteStream.h"

class TCPHostThread;
class TCPConnection;


class SessionImporter
{
public:
	void ProcessPacket(TCPConnection*, void*, unsigned int);

	void FlushData(TCPConnection*);

private:
	Session session_;
	std::string storage_target_ = "";

	void ValidateSession(TCPConnection*);
};


class AuthorizedUserInterface
{
public:
	ByteStream ProcessCommand(TCPConnection*, void*, unsigned int);

private:
	bool authorized_ = false;
};


class TCPConnection : public ThreadBase
{
public:
	void Start() override;
	void Loop() override;
	void Cleanup() override;

	CActiveSocket* socket = nullptr;
	TCPHostThread* parent_thread;

	void TCPLog(std::string);

private:
	ByteStream data_;

	SessionImporter session_importer_;
	AuthorizedUserInterface dev_command_;
};
