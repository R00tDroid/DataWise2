#include "AnalyticsPacketFactory.h"
#include "AnalyticsPacket.h"
#include "AssetTypeCategories.h"


UAnalyticsPacketFactory::UAnalyticsPacketFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UAnalyticsPacket::StaticClass();
}

uint32 UAnalyticsPacketFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Blueprint;
}

UObject* UAnalyticsPacketFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UAnalyticsPacket* NewObjectAsset = NewObject<UAnalyticsPacket>(InParent, Class, Name, Flags | RF_Transactional);
	return NewObjectAsset;
}

FText UAnalyticsPacketFactory::GetDisplayName() const
{
	return FText::FromString("Analytics Packet");
}

FText UAnalyticsPacketFactory::GetToolTip() const
{
	return FText::FromString("A packet containing event data that can be reported to an Analytics Session");
}