#pragma once
#include "UObject/Class.h"
#include "RecordableSession.h"
#include "AnalyticsPacket.h"
#include "AnalyticsSession.generated.h"

class LocalPacketSerializer;

UENUM(BlueprintType)
enum class EBPRecordingMode : uint8
{
	RM_Buffered	UMETA(DisplayName = "Buffered"),
	RM_Streamed	UMETA(DisplayName = "Streamed")
};

UCLASS(Blueprintable)
class DATAWISE_API UAnalyticsSession : public UObject
{
GENERATED_BODY()
public:

	UFUNCTION(BLUEPRINTCALLABLE, BLUEPRINTPURE, Category = "Analytics Session")
	static void GetSession(int32 index, UAnalyticsSession*& session, bool& valid);

	UFUNCTION(BLUEPRINTCALLABLE, BLUEPRINTPURE, Category = "Analytics Session")
	static TArray<UAnalyticsSession*> GetActiveSessions();

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics Session", meta = (WorldContext = WorldContextObject))
	static void BeginSession(UObject* WorldContextObject, FString StorageName, FString address, int port, EBPRecordingMode RecordingMode, UAnalyticsSession*& session, int32& index);

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics Session")
	void EndSession();

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics Session")
	void SetMetaInteger(FString Tag, int Value);

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics Session")
	void SetMetaFloat(FString Tag, float Value);

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics Session")
	void SetMetaString(FString Tag, FString Value);

	virtual void BeginDestroy() override;

	RecordableSession* GetNativeSession() { return session; }

private:
	TMap<FString, FString> meta_data;

	RecordableSession* session;

	bool running_ = false;
};