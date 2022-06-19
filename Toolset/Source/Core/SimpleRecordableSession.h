#pragma once
#include "Export.h"
#include <string>

extern "C" 
{
	int EXPORTED BeginSession(char* storage, char* address, int port, int mode);

	void EXPORTED EndSession(int id);

	void EXPORTED CleanupSession(int id);

	bool EXPORTED HasSessionFinished(int id);

	void EXPORTED ReportEvent(int id, char* event);

	void EXPORTED SetMetaInt(int id, char* name, int value);
	void EXPORTED SetMetaFloat(int id, char* name, float value);
	void EXPORTED SetMetaString(int id, char* name, char* value);

	void* GetSessionPtr(int id);
}