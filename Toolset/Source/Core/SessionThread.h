#pragma once
#include "Export.h"
#include "../Shared/Platform/ThreadBase.h"

class ByteStream;
class RecordableSession;
struct SocketInfo;

class SessionThread : public ThreadBase
{
public:
	void Start() override;
	void Loop() override;
	void Cleanup() override;

	RecordableSession* parent_session;
	SocketInfo* socket_info;

private:
	bool Send(uint8_t, ByteStream&);
};
