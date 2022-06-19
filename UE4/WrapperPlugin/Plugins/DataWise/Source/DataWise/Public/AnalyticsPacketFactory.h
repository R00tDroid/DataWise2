#pragma once
#include "UnrealEd.h"
#include "AnalyticsPacketFactory.generated.h"

UCLASS()
class DATAWISE_API UAnalyticsPacketFactory : public UFactory
{
	GENERATED_UCLASS_BODY()
public:
	uint32 GetMenuCategories() const override;

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	FText GetDisplayName() const override;

	FText GetToolTip() const override;
};