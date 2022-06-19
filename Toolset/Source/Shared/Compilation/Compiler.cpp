#include "Compiler.h"
#include "../Dependencies/duktape/duktape.h"
#include <string>

std::function<void(std::string)> compiler_log_callback_ = nullptr;

CompilerLogReceiver::Status CompilerLogReceiver::Receive(std::string& log)
{
	Status result;

	lock_.lock();
	result = status_;
	log = log_;
	lock_.unlock();

	return result;
}

void CompilerLogReceiver::Notify(Status status, std::string log)
{
	lock_.lock();
	status_ = status;
	log_ = log;
	lock_.unlock();
}

void Compiler::AddLogReceiver(CompilerLogReceiver* receiver)
{
	receivers_.push_back(receiver);
}

void Compiler::AddSession(Session& session)
{
	sessions_.push_back(session);
}

void Compiler::AddStage(ICompilationStage* stage)
{
	stage->Init();
	stages_.push_back(stage);
}

bool Compiler::Compile(std::string ExportName)
{
	if (!stages_.empty() && !sessions_.empty() && !ExportName.empty()) 
	{
		for (ICompilationStage* stage : stages_)
		{
			stage->Assign(this);
			stage->Prepare();
			for (Session& session : sessions_)
			{
				stage->ProcessSession(session);
			}
			stage->Export();
			stage->OutputData(ExportName);
		}
	}

	Cleanup();

	Log("Finished compiling");

	for(CompilerLogReceiver* receiver : receivers_)
	{
		receiver->Notify(CompilerLogReceiver::Finished, log_);
	}

	return true;
}




void Compiler::SetLogCallback(std::function<void(std::string)> callback)
{
	compiler_log_callback_ = callback;
}

void Compiler::CompilerLog(std::string message)
{
	if (compiler_log_callback_ != nullptr) compiler_log_callback_(message);
}

std::vector<SessionInfo> Compiler::FilterSessions(std::vector<SessionInfo>& source, std::vector<MetaFilter> filters)
{
	std::vector<SessionInfo> result;
	for(SessionInfo& session : source)
	{
		bool valid = true;
		for (MetaFilter& filter : filters)
		{
			if (!filter.Matches(session.Meta)) 
			{
				valid = false;
				break; 
			}
		}
		if(valid) result.push_back(session);
	}

	return result;
}

void Compiler::Log(std::string message)
{
	CompilerLog(message);
	log_ += message + "\n";
}

Compiler::~Compiler()
{
	Cleanup();
}

void Compiler::Cleanup()
{
	sessions_ = {};

	for (ICompilationStage* stage : stages_)
	{
		stage->Cleanup();
		delete stage;
	}
	stages_ = {};
}
