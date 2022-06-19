#pragma once
#include "../../Shared/Dependencies/clsocket/PassiveSocket.h"
#include "../../Shared/Platform/ThreadBase.h"
#include <vector>
#include "TCPConnection.h"

class TCPHostThread : public ThreadBase
{
public:
	void Start() override;
	void Loop() override;
	void Cleanup() override;

	std::string address;
	uint16 port;

	std::vector<TCPConnection*> connections;

private:
	CPassiveSocket socket;
};

class TCPHost
{
public:
	void Init();

	void Loop();

	void Cleanup();

private:
	TCPHostThread thread;
};
