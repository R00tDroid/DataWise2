#pragma once
#include "../Shared/Platform/ThreadBase.h"
#include "../Shared/Compilation/CompilationStage.h"
#include "../Shared/Compilation/Compiler.h"
#include <ctime>

struct CompilationTask
{
	std::string ExportName;
	std::vector<std::string> Databases;
	std::vector<ICompilationStage*> Stages;
	std::vector<CompilerLogReceiver*> Receivers;
	std::vector<MetaFilter> Filters;
};

class ScheduledCompilationTask
{
friend class CompilationManager;
public:
	enum Frequency
	{
		Timed,
		Daily,
		Weekly
	};

	ScheduledCompilationTask(Frequency);
	Frequency GetFrequencyType();
	bool NeedsUpdate();
	void UpdateScheduling();
	
	std::chrono::system_clock::time_point GetNextUpdateTarget();

	std::string ExportName;
	std::string Filter;
	std::vector<std::string> Databases;
	std::vector<CompilationStageInfo> Stages;

	union
	{
		unsigned int UpdateDelay;	// Minutes between triggers
		unsigned int TimeOfDay;		// Time from beginning of day in hours
		unsigned char WeekDay;		// Target day of week (starts at 0: Sunday)
		unsigned short MonthDay;	// Target day of month (starts at 0)
	};

protected:
	void ResetScheduling();

private:
	Frequency execution_frequency_;
	std::chrono::system_clock::time_point last_update_;
	std::chrono::system_clock::time_point update_target_;
};

class CompilationThread : public ThreadBase
{
public:
	void Start() override;
	void Loop() override;
	void Cleanup() override;

	void AddTask(CompilationTask& task);

private:
	std::vector<CompilationTask> tasks_;
};

class CompilationManager
{
public:
	static CompilationManager& Get();

	void Init();
	void Loop();
	void Cleanup();

	void AddTask(CompilationTask& task);
	
private:
	static CompilationManager instance_;
	CompilationThread thread;
	std::vector<ScheduledCompilationTask> scheduled_tasks_;
};