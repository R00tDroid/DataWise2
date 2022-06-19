#pragma once
#include "AnalyticsSession.h"
#include "PacketBuilder.h"
#include "AnalyticsPacketBuilder.generated.h"


UCLASS()
class DATAWISE_API UAnalyticsPacketBuilder : public UObject
{
GENERATED_BODY()
public:

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics")
	static UAnalyticsPacketBuilder* CreatePacket(UAnalyticsSession* Session, FString PacketName);

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics")
	void Send();

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics")
	void AddTime();

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics")
	void AddInt(int value);

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics")
	void AddFloat(float value);

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics")
	void AddString(FString value);

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics")
	void AddFloat2(FVector2D value);

	UFUNCTION(BLUEPRINTCALLABLE, Category = "Analytics")
	void AddFloat3(FVector value);

private:
	PacketBuilder* builder_;
};