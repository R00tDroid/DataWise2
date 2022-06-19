#include <iostream>
#include <windows.h>
#include "../Core/RecordableSession.h"
#include "../Core/PacketBuilder.h"

void Log(std::string text)
{
	std::cout << text.c_str();
	OutputDebugStringA(text.c_str());
}

void Delay(float millis)
{
#ifdef _WIN32
	Sleep(millis);
#else
	usleep(millis * 1000);
#endif
}

int main()
{
	Log("Starting session\n");

	RecordableSession* session = RecordableSession::Begin("DW_Test", "127.0.0.1");

	Log("Running session\n");

	int millis = 1240;
	Log("Sleeping for " + std::to_string(millis / 1000.0f) + " seconds\n");

	Delay(500);

	PacketBuilder builder(session, "TestPacket");
	builder.AddTimestamp();
	builder.Add("test string");
	builder.Add<int>(6);
	builder.Add<float>(0.24f);
	builder.Send();

	for (int i = 0; i < 10; i++) 
	{
		PacketBuilder builder(session, "TimePacket");
		builder.AddTimestamp();
		builder.Send();

		Delay(500);
	}

	session->SetMeta<int>("Session duration", 5);

	Log("Stopping client\n");

	session->End();
	delete session;

	Log("Stopped client\n");

	return 0;
}