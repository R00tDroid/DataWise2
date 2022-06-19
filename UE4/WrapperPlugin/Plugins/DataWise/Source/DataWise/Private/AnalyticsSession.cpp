#include "AnalyticsSession.h"
#include "DataWise.h"
#include "Engine/Engine.h"
#include "PacketBuilder.h"

TArray<UAnalyticsSession*> active_sessions_;

void UAnalyticsSession::GetSession(int32 index, UAnalyticsSession*& session, bool& valid)
{
	valid = active_sessions_.IsValidIndex(index);
	if (valid) session = active_sessions_[index]; else session = nullptr;
}

TArray<UAnalyticsSession*> UAnalyticsSession::GetActiveSessions()
{
	return active_sessions_;
}

void UAnalyticsSession::BeginSession(UObject* WorldContextObject, FString StorageName, FString address, int port, EBPRecordingMode RecordingMode, UAnalyticsSession*& session, int32& index)
{
	UWorld * world = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	session = NewObject<UAnalyticsSession>(world);
	session->SetFlags(RF_Standalone);
	session->running_ = true;
	session->session = RecordableSession::Begin(TCHAR_TO_UTF8(*StorageName), TCHAR_TO_UTF8(*address), port, static_cast<ERecordingMode>(RecordingMode));
	index = active_sessions_.Add(session);
}

void UAnalyticsSession::EndSession()
{
	session->End();
	active_sessions_.Remove(this);
	running_ = false;
}

void UAnalyticsSession::SetMetaInteger(FString Tag, int Value)
{
	session->SetMeta<int>(TCHAR_TO_UTF8(*Tag), Value);
}

void UAnalyticsSession::SetMetaFloat(FString Tag, float Value)
{
	session->SetMeta<float>(TCHAR_TO_UTF8(*Tag), Value);
}

void UAnalyticsSession::SetMetaString(FString Tag, FString Value)
{
	session->SetMeta<char*>(TCHAR_TO_UTF8(*Tag), TCHAR_TO_UTF8(*Value));
}

void UAnalyticsSession::BeginDestroy()
{
	Super::BeginDestroy();
	if (running_) EndSession();
}
