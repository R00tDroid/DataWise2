#include <iostream>
#include "Application/Log.h"
#include "Networking/TCPHost.h"
#include "Application/ConfigurationManager.h"
#include "Application/Application.h"
#include "../Shared/InstanceLock.h"
#include "../Shared/Compilation/Compiler.h"
#include "../Shared/Compilation/EcmaCompilationStage.h"
#include "Networking/StorageManager.h"
#include "CompilationManager.h"

int main()
{
	Application::Get().Start();

	Log("Starting server...");

	InstanceLock lock("server");

	if(lock.IsLocked())
	{
		Log("A server instance is already running\nServer startup canceled");
	}
	else 
	{
		lock.Aquire();

		ConfigurationManager::Get().Load();

		TCPHost tcp;

		tcp.Init();
		CompilationManager::Get().Init();

		Log("Server started");

		while (!Application::Get().ShouldStop())
		{
			tcp.Loop();
			CompilationManager::Get().Loop();

			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
		}

		Log("Stopping server...");

		CompilationManager::Get().Cleanup();

		tcp.Cleanup();

		lock.Release();

		Log("Server stopped");
	}

	Application::Get().Stop();

	StopLog();

	return 0;

}
