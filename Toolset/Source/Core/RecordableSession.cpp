#include "RecordableSession.h"
#include "SessionThread.h"
#include "Log.h"

#define max_time_res 2

RecordableSession* RecordableSession::Begin(std::string storage, std::string address, int port, ERecordingMode mode)
{
	RecordableSession* session = new RecordableSession();
	session->mode_ = mode;
	session->address_ = address;
	session->port_ = port;
	session->storage_target_ = storage;
	session->thread_ = new SessionThread();
	session->Initialize();

	return session;
}

void RecordableSession::Initialize()
{
	session_start_ = std::chrono::high_resolution_clock::now();

	thread_->parent_session = this;
	recording_ = true;

	if (mode_ == Streamed) thread_->Dispatch();

	DWLog("Session started");
}

void SetLogCallback(void(* callback)(int, char*))
{
	SetLogReceiver(callback);
}

RecordableSession::~RecordableSession()
{
	if(IsRecording())
	{
		End();
	}

	while(thread_->IsRunning()){}
	delete thread_;

	DWLog("Session destroyed");
}

void RecordableSession::End()
{
	queued_meta_data_ = meta_data_;

	thread_->Lock();
	recording_ = false;
	thread_->Unlock();

	if (!thread_->IsRunning()) thread_->Dispatch();

	DWLog("Session ended");
}

bool RecordableSession::HasFinished()
{
	return !thread_->IsRunning() && !recording_;
}

void RecordableSession::ReportEvent(std::string data)
{
	DWLog("Event reported\n\t" + data);

	thread_->Lock();
	data_.push_back(data);
	thread_->Unlock();
}

void RecordableSession::SetMeta(char* Name, std::string Type, std::string Value, bool Stringify)
{
	std::string data = "[\"" + Type + "\"," + (Stringify ? "\"" : "") + Value + (Stringify ? "\"" : "") + "]";

	auto it = meta_data_.find(Name);
	if(it == meta_data_.end())
	{
		meta_data_.insert(std::pair<std::string, std::string>(Name, data));
	} 
	else
	{
		it->second = data;
	}
}

float RecordableSession::GetTime()
{
	std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
	double res = std::pow(10, max_time_res);
	return std::floor(std::chrono::duration<double>(now - session_start_).count() * res) / res;
}

bool RecordableSession::IsRecording()
{
	bool state;
	thread_->Lock();
	state = recording_;
	thread_->Unlock();

	return state;
}

