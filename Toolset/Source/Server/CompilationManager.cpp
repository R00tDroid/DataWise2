#include "CompilationManager.h"
#include "Application/Log.h"
#include "../Shared/Compilation/Compiler.h"
#include "Networking/StorageManager.h"
#include <chrono>
#include "Application/ConfigurationManager.h"

CompilationManager CompilationManager::instance_;

ScheduledCompilationTask::ScheduledCompilationTask(Frequency freq)
{
	execution_frequency_ = freq;

	switch (freq) { 
	case Timed: { UpdateDelay = 5; break; }
	case Daily: { TimeOfDay = 9; break; }
	case Weekly: { WeekDay = 0; break; }
	}

	last_update_ = std::chrono::system_clock::now() - std::chrono::hours(24 * 365);
	update_target_ = GetNextUpdateTarget();
}

ScheduledCompilationTask::Frequency ScheduledCompilationTask::GetFrequencyType()
{
	return execution_frequency_;
}

bool ScheduledCompilationTask::NeedsUpdate()
{
	auto now = std::chrono::system_clock::now();

	double time_left = std::chrono::duration_cast<std::chrono::seconds>(update_target_ - now).count();

	return time_left <= 0;
}

void ScheduledCompilationTask::UpdateScheduling()
{
	update_target_ = GetNextUpdateTarget();
}

void ScheduledCompilationTask::ResetScheduling()
{
	last_update_ = std::chrono::system_clock::now();
	update_target_ = GetNextUpdateTarget();
}

std::chrono::system_clock::time_point ScheduledCompilationTask::GetNextUpdateTarget()
{
	std::chrono::system_clock::time_point target = std::chrono::system_clock::now() + std::chrono::hours(1);

	if (execution_frequency_ == Timed)
	{
		target = last_update_ + std::chrono::minutes(UpdateDelay);
	}
	else if (execution_frequency_ == Daily)
	{
		std::time_t last_day = std::chrono::system_clock::to_time_t(last_update_);
		std::tm last_day_info = *localtime(&last_day);
		last_day_info.tm_hour = 0;
		last_day_info.tm_min = 0;
		last_day_info.tm_sec = 0;

		std::chrono::system_clock::time_point rounded_last_day = std::chrono::system_clock::from_time_t(mktime(&last_day_info));

		target = rounded_last_day + std::chrono::hours(24 + TimeOfDay);
	}
	else if (execution_frequency_ == Weekly)
	{
		std::time_t last_day = std::chrono::system_clock::to_time_t(last_update_);
		std::tm last_day_info = *localtime(&last_day);
		last_day_info.tm_yday -= last_day_info.tm_wday;
		last_day_info.tm_mday -= last_day_info.tm_wday;
		last_day_info.tm_wday = 0;
		last_day_info.tm_hour = 0;
		last_day_info.tm_min = 0;
		last_day_info.tm_sec = 0;

		std::chrono::system_clock::time_point rounded_last_day = std::chrono::system_clock::from_time_t(mktime(&last_day_info));

		target = rounded_last_day + std::chrono::hours(24 * (7 + WeekDay));
	}

	return target;
}

void CompilationThread::Start()
{

}

void CompilationThread::Loop()
{
	Lock();
	bool has_tasks = !tasks_.empty();
	CompilationTask task;
	if (has_tasks)
	{
		task = tasks_[0];
		tasks_.erase(tasks_.begin());
	}
	Unlock();

	unsigned int session_count = 0, valid_session_count = 0;

	if(has_tasks)
	{
		Compiler comp;
		for (CompilerLogReceiver* receiver : task.Receivers) comp.AddLogReceiver(receiver);
		for (ICompilationStage* stage : task.Stages) comp.AddStage(stage);

		for (std::string& db : task.Databases) 
		{
			IStorageConnection* con = StorageManager::Get().OpenConnection(db);
			if (con != nullptr) 
			{
				std::vector<SessionInfo> sessions = con->GetSessionList();
				session_count += sessions.size();
				std::vector<SessionInfo> filtered_sessions = Compiler::FilterSessions(sessions, task.Filters);
				valid_session_count += filtered_sessions.size();

				for (SessionInfo& info : filtered_sessions)
				{
					Session session = con->RetrieveSession(info);
					comp.AddSession(session);
				}

				con->Close();
				con = nullptr;

			} else
			{
				comp.Log("Could not connect to database '" + db + "'");
			}
		}

		comp.Log(std::to_string(valid_session_count) + " / " + std::to_string(session_count) + " sessions selected for compilation");

		comp.Compile(task.ExportName);
	}
}

void CompilationThread::Cleanup()
{
	Lock();
	for(CompilationTask& task : tasks_)
	{
		for (CompilerLogReceiver* receiver : task.Receivers) receiver->Notify(CompilerLogReceiver::Error, "Compilation canceled");
	}
	Unlock();
}

void CompilationThread::AddTask(CompilationTask& task)
{
	Lock();
	tasks_.push_back(task);
	Unlock();
}

CompilationManager& CompilationManager::Get()
{
	return instance_;
}

void CompilationManager::Init()
{
	Compiler::SetLogCallback(&Log);

	thread.Dispatch();

	scheduled_tasks_ = ConfigurationManager::Get().CompilationSchedule;
}

void CompilationManager::Loop()
{
	for(ScheduledCompilationTask& task : scheduled_tasks_)
	{
		if(task.NeedsUpdate())
		{
			std::string databases = "";
			std::string stages = "";

			for(std::string& db : task.Databases)
			{
				databases += (databases.empty() ? "" : ", ") + db;
			}

			for (CompilationStageInfo& stage : task.Stages)
			{
				stages += (stages.empty() ? "" : ", ") + stage.Type + ":" + stage.Name;
			}
			
			Log("Preparing scheduled task for compilation [" + databases + "] {" + stages + "}");

			CompilationTask execution_task;
			execution_task.Databases = task.Databases;
			execution_task.ExportName = task.ExportName;
			execution_task.Filters = MetaFilter::Parse(task.Filter);

			for (CompilationStageInfo& stage_info : task.Stages)
			{
				ICompilationStage* stage = ICompilationStageRegistry::CreateStage(stage_info);
				if (stage != nullptr) execution_task.Stages.push_back(stage);
			}

			AddTask(execution_task);

			task.ResetScheduling();
		}
	}
}

void CompilationManager::Cleanup()
{
	thread.RequestEnd();
	while(thread.IsRunning()){}
}

void CompilationManager::AddTask(CompilationTask& task)
{
	thread.AddTask(task);
}
