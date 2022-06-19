#include "SimpleRecordableSession.h"
#include "RecordableSession.h"
#include <map>
#include "SessionThread.h"
#include "Log.h"

std::map<int, RecordableSession*> sessions;

int UniqueId()
{
	while(true)
	{
		int id = 1000 + rand() % 8999;
		if (sessions.find(id) == sessions.end()) return id;
	}
}

RecordableSession* GetSession(int id)
{
	if (sessions.find(id) == sessions.end()) { DWLog("Session does not exist: " + std::to_string(id)); return nullptr; }
	return sessions[id];
}


int BeginSession(char* storage, char* address, int port, int mode)
{
	int id = UniqueId();
	sessions.insert(std::pair<int, RecordableSession*>(id, RecordableSession::Begin(storage, address, port, static_cast<ERecordingMode>(mode))));

	return id;
}

void EndSession(int id)
{
	RecordableSession* session = GetSession(id);
	if (session == nullptr) return; 

	session->End();
}

void CleanupSession(int id)
{
	RecordableSession* session = GetSession(id);
	if (session == nullptr) return;

	delete session;

	sessions.erase(sessions.find(id));
}

bool HasSessionFinished(int id)
{
	RecordableSession* session = GetSession(id);
	if (session == nullptr) return true;

	return session->HasFinished();
}

void ReportEvent(int id, char* event)
{
	RecordableSession* session = GetSession(id);
	if (session == nullptr) return;

	session->ReportEvent(event);
}

void* GetSessionPtr(int id)
{
	return GetSession(id);
}

void SetMetaInt(int id, char* name, int value)
{
	RecordableSession* session = GetSession(id);
	if (session == nullptr) return;
	session->SetMeta<int>(name, value);
}

void SetMetaFloat(int id, char* name, float value)
{
	RecordableSession* session = GetSession(id);
	if (session == nullptr) return;
	session->SetMeta<float>(name, value);
}

void SetMetaString(int id, char* name, char* value)
{
	RecordableSession* session = GetSession(id);
	if (session == nullptr) return;
	session->SetMeta<char*>(name, value);
}
