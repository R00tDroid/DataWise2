#pragma once
#include "Export.h"
#include <string>
#include <chrono>
#include <vector>
#include <sstream>
#include <map>

enum ERecordingMode
{
	Buffered,
	Streamed
};

class SessionThread;

class EXPORTED RecordableSession
{
	friend class SessionThread;
public:
	static RecordableSession* Begin(std::string storage, std::string address, int port = 5438, ERecordingMode mode = Streamed);

	~RecordableSession();

	void End();
	
	bool HasFinished();

	void ReportEvent(std::string);


	template<class T>
	void SetMeta(char* Name, T Value);

	template<>
	void SetMeta(char* Name, float Value) { SetMeta(Name, "float", ToString(Value)); }

	template<>
	void SetMeta(char* Name, int Value) { SetMeta(Name, "int", std::to_string(Value)); }

	template<>
	void SetMeta(char* Name, std::string Value) { SetMeta(Name, "string", Value, true); }

	template<>
	void SetMeta(char* Name, char* Value) { SetMeta(Name, "string", Value, true); }

	template<>
	void SetMeta(char* Name, const char* Value) { SetMeta(Name, "string", Value, true); }


	float GetTime();

	bool IsRecording();

private:
	void Initialize();
	void SetMeta(char* Name, std::string Type, std::string Value, bool Stringify = false);

	std::string storage_target_;
	ERecordingMode mode_;
	std::string address_;
	int port_;

	std::chrono::high_resolution_clock::time_point session_start_;

	SessionThread* thread_;

	bool recording_ = false;

	std::vector<std::string> data_;
	std::map<std::string, std::string> meta_data_;
	std::map<std::string, std::string> queued_meta_data_;

	template<class T>
	std::string ToString(T data)
	{
		std::ostringstream stream;
		stream << data;
		return stream.str();
	}
};

extern "C" void EXPORTED SetLogCallback(void (*callback)(int, char*));