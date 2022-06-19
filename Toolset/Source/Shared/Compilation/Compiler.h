#pragma once
#include <string>
#include <functional>
#include "../IO/Session.h"
#include "CompilationStage.h"
#include <mutex>

class CompilerLogReceiver
{
public:
	enum Status
	{
		Pending,
		Error,
		Finished
	};

	Status Receive(std::string& log);
	void Notify(Status status, std::string log);

private:
	std::mutex lock_;
	Status status_ = Pending;
	std::string log_;
};

class Compiler
{
public:

	void AddLogReceiver(CompilerLogReceiver*);
	void AddSession(Session&);
	void AddStage(ICompilationStage*);

	bool Compile(std::string);

	static void SetLogCallback(std::function<void(std::string)>);
	static void CompilerLog(std::string);

	static std::vector<SessionInfo> FilterSessions(std::vector<SessionInfo>&, std::vector<MetaFilter>);

	void Log(std::string);

	~Compiler();

private:
	std::vector<Session> sessions_;
	std::vector<CompilerLogReceiver*> receivers_;
	std::vector<ICompilationStage*> stages_;
	std::string log_;
	void Cleanup();
};

