#pragma once
#include "K2Node.h"
#include "K2Node_ConstructObjectFromClass.h"
#include "AnalyticsPacket.h"
#include "AnalyticsSessionReportNode.generated.h"


UCLASS(BlueprintType, Blueprintable) 
class UK2_AnalyticsSessionReportNode : public UK2Node {

	GENERATED_UCLASS_BODY()

	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;

	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual void PostPlacedNewNode() override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin);

	virtual bool IsActionFilteredOut(const FBlueprintActionFilter&) override;

private:
	TArray<UEdGraphPin*> FindDataPins();

	void CreateDataPins(TMap<FString, EPacketMemberType> types);
	TMap<FString, EPacketMemberType> FindPacketTypes(TArray<UEdGraphPin*>& pins);
};