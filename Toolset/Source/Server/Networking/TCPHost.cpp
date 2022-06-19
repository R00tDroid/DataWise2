#include "TCPHost.h"
#include "../Application/Log.h"
#include "../Application/ConfigurationManager.h"


void TCPHostThread::Start()
{
	socket.Initialize();
	socket.Listen(address.c_str(), port);
	socket.SetNonblocking();

	Log("TCP thread started on port " + std::to_string(port));
}

void TCPHostThread::Loop()
{
	CActiveSocket* client = nullptr;

	if ((client = socket.Accept()) != nullptr) // Check if client wants to connect
	{
		TCPConnection* new_connection = new TCPConnection();
		new_connection->socket = client;
		new_connection->parent_thread = this;
		Lock();
		connections.push_back(new_connection);
		Unlock();
		new_connection->Dispatch();
	}
}

void TCPHostThread::Cleanup()
{
	Log("Stopping TCP thread...");

	socket.Close();

	while(true)
	{
		bool can_stop = false;

		Lock();
		can_stop = connections.size() == 0;
		Unlock();

		if (can_stop) break;
	}

	Log("TCP thread has stopped");
}



void TCPHost::Init()
{
	thread.address = "127.0.0.1";
	thread.port = ConfigurationManager::Get().ServerPort;

	thread.Dispatch();
}

void TCPHost::Loop()
{
	
}

void TCPHost::Cleanup()
{
	thread.RequestEnd();
	while (thread.IsRunning()) {}
}
