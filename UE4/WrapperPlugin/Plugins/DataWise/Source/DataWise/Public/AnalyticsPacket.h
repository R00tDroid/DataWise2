#pragma once
#include "Engine/DataAsset.h"
#include "AnalyticsPacket.generated.h"

UENUM(BlueprintType)
enum class EPacketMemberType : uint8
{
	PMT_Int		UMETA(DisplayName = "Int"),
	PMT_Float	UMETA(DisplayName = "Float"),
	PMT_String	UMETA(DisplayName = "String"),
	PMT_Time	UMETA(DisplayName = "Time"),
	PMT_Vector2	UMETA(DisplayName = "Vector 2"),
	PMT_Vector3	UMETA(DisplayName = "Vector 3")
};

UCLASS()
class DATAWISE_API UAnalyticsPacket : public UDataAsset
{
GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere)
	FString PacketName;

	UPROPERTY(EditAnywhere)
	TMap<FString, EPacketMemberType> members;
};