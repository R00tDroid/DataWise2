#include "AnalyticsSessionReportNode.h"
#include "DataWiseEditor.h"
#include "KismetCompiler.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "AnalyticsSession.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintCompilationManager.h"
#include "Kismet/GameplayStatics.h"
#include "BlueprintActionFilter.h"
#include "AnalyticsPacketBuilder.h"
#include "Engine/AssetManager.h"

#define LOCTEXT_NAMESPACE "AnalyticsEditorModule"

UK2_AnalyticsSessionReportNode::UK2_AnalyticsSessionReportNode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UK2_AnalyticsSessionReportNode::AllocateDefaultPins() 
{
	UEdGraphPin* EnterPin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	UEdGraphPin* ExitPin = CreatePin(EEdGraphPinDirection::EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	UEdGraphPin* TargetPin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Object, UAnalyticsSession::StaticClass(), FName("Session"));
	UEdGraphPin* PacketPin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Object, UAnalyticsPacket::StaticClass(), FName("Packet"));

	PacketPin->bNotConnectable = true;

	Super::AllocateDefaultPins();
}

FText UK2_AnalyticsSessionReportNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString("Report Event");
}

void UK2_AnalyticsSessionReportNode::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	if (ActionRegistrar.IsOpenForRegistration(GetClass()))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(GetClass(), NodeSpawner);
	}
}

void UK2_AnalyticsSessionReportNode::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	if(Pin == FindPin(FName("Packet")))
	{
		TArray<UEdGraphPin*> OldPins = Pins;

		CreateDataPins(FindPacketTypes(Pins));

		RestoreSplitPins(OldPins);
	}
}

void UK2_AnalyticsSessionReportNode::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();

	// Restore packet reference on engine startup
	for (UEdGraphPin* packet_pin : OldPins)
	{
		if (packet_pin->GetName().Equals("Packet"))
		{
			FindPin(FName("Packet"))->DefaultObject = packet_pin->DefaultObject;
		}
	}

	CreateDataPins(FindPacketTypes(Pins));

	RestoreSplitPins(OldPins);
}

void UK2_AnalyticsSessionReportNode::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();
	
	CreateDataPins(FindPacketTypes(Pins));
}

void UK2_AnalyticsSessionReportNode::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	if (Pin == FindPin(FName("Packet")))
	{
		PinDefaultValueChanged(Pin);
	}
}

TArray<UEdGraphPin*> UK2_AnalyticsSessionReportNode::FindDataPins()
{
	TArray<UEdGraphPin*> found;
	for (UEdGraphPin* pin : Pins)
	{
		if (pin != GetExecPin() && pin != FindPin(UEdGraphSchema_K2::PN_Then) && pin != FindPin(FName("Session")) && pin != FindPin(FName("Packet")))
		{
			found.Add(pin);
		}
	}
	return found;
}

void UK2_AnalyticsSessionReportNode::CreateDataPins(TMap<FString, EPacketMemberType> types)
{
	TArray<UEdGraphPin*> data_pins = FindDataPins();
	for (UEdGraphPin* data_pin : data_pins)
	{
		data_pin->BreakAllPinLinks(true);
		Pins.Remove(data_pin);
	}

	TArray<UEdGraphPin*> OldPins = Pins;

	if (types.Num() > 0)
	{
		for (const TPair<FString, EPacketMemberType>& type : types)
		{
			UEdGraphPin* new_data_pin = nullptr;

			switch (type.Value) {
			case EPacketMemberType::PMT_Int: { new_data_pin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Int, FName(*type.Key)); break; }
			case EPacketMemberType::PMT_Float: { new_data_pin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Float, FName(*type.Key)); break; }
			case EPacketMemberType::PMT_String: { new_data_pin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_String, FName(*type.Key)); break; }
			case EPacketMemberType::PMT_Time: { new_data_pin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, FName(*type.Key)); new_data_pin->bNotConnectable = true; break; }
			case EPacketMemberType::PMT_Vector2: { new_data_pin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector2D>::Get(), FName(*type.Key)); break; }
			case EPacketMemberType::PMT_Vector3: { new_data_pin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector>::Get(), FName(*type.Key)); break; }
			}
		}
	}

	RestoreSplitPins(OldPins);
	GetGraph()->NotifyGraphChanged();
	FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
}

TMap<FString, EPacketMemberType> UK2_AnalyticsSessionReportNode::FindPacketTypes(TArray<UEdGraphPin*>& search_pins)
{
	for(UEdGraphPin* packet_pin : search_pins)
	{
		if(packet_pin->GetName().Equals("Packet"))
		{
			if (packet_pin->DefaultObject == nullptr) return {};

			UAnalyticsPacket* packet = ((UAnalyticsPacket*)packet_pin->DefaultObject);
			packet->GetLinker()->Preload(packet);

			return packet->members;
			
		}
	}

	return {};
}

FText UK2_AnalyticsSessionReportNode::GetMenuCategory() const
{
	return FText::FromString("Analytics Session");
}

bool UK2_AnalyticsSessionReportNode::IsActionFilteredOut(const FBlueprintActionFilter& filter)
{
	for(FBlueprintActionFilter::FTargetClassFilterData fd : filter.TargetClasses)
	{
		if (fd.TargetClass == UAnalyticsSession::StaticClass()) return false;
	}

	return true;
}

void UK2_AnalyticsSessionReportNode::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) 
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UEdGraphPin* pin_exec = GetExecPin();
	UEdGraphPin* pin_then = FindPin(UEdGraphSchema_K2::PN_Then);
	UEdGraphPin* pin_session = FindPin(FName("Session"));
	UEdGraphPin* pin_packet = FindPin(FName("Packet"));

	UAnalyticsPacket* packet = (UAnalyticsPacket*)pin_packet->DefaultObject;

	if (packet == nullptr)
	{
		CompilerContext.MessageLog.Warning(*FString("No packet type selected for @@"), this);
		return;
	}

	// Ensure asset is loaded when during engine startup
	packet->GetLinker()->Preload(packet);


	// Create construction node

	UK2Node_CallFunction* PacketCreateNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	PacketCreateNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UAnalyticsPacketBuilder, CreatePacket), UAnalyticsPacketBuilder::StaticClass());
	PacketCreateNode->AllocateDefaultPins();

	UEdGraphPin* pin_packet_exec = PacketCreateNode->GetExecPin();
	UEdGraphPin* pin_packet_then = PacketCreateNode->GetThenPin();
	UEdGraphPin* pin_packet_session = PacketCreateNode->FindPin(TEXT("Session"));
	UEdGraphPin* pin_packet_name = PacketCreateNode->FindPin(TEXT("PacketName"));
	UEdGraphPin* pin_packet_builder = PacketCreateNode->GetReturnValuePin();

	pin_packet_name->DefaultValue = packet->PacketName;

	CompilerContext.MovePinLinksToIntermediate(*pin_exec, *pin_packet_exec);
	CompilerContext.MovePinLinksToIntermediate(*pin_session, *pin_packet_session);

	// Create assignment nodes

	UEdGraphPin* pin_then_assignment = pin_packet_then;
	TArray<UEdGraphPin*> data_pins = FindDataPins();

	if (packet->members.Num() != data_pins.Num())
	{
		CompilerContext.MessageLog.Error(*FString("An error in @@ occured while expanding: Packet and pin misalignment"), this);
		return;
	}

	TArray<FString> member_names;
	packet->members.GetKeys(member_names);

	for (int i = 0; i < data_pins.Num(); i++)
	{
		EPacketMemberType pin_type = packet->members[member_names[i]];

		UK2Node_CallFunction* assignment_node = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);

		switch (pin_type) { 
		case EPacketMemberType::PMT_Int: { assignment_node->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UAnalyticsPacketBuilder, AddInt), UAnalyticsPacketBuilder::StaticClass()); break; }
		case EPacketMemberType::PMT_Float: { assignment_node->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UAnalyticsPacketBuilder, AddFloat), UAnalyticsPacketBuilder::StaticClass()); break; }
		case EPacketMemberType::PMT_String: { assignment_node->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UAnalyticsPacketBuilder, AddString), UAnalyticsPacketBuilder::StaticClass()); break; }
		case EPacketMemberType::PMT_Time: { assignment_node->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UAnalyticsPacketBuilder, AddTime), UAnalyticsPacketBuilder::StaticClass()); break; }
		case EPacketMemberType::PMT_Vector2: { assignment_node->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UAnalyticsPacketBuilder, AddFloat2), UAnalyticsPacketBuilder::StaticClass()); break; }
		case EPacketMemberType::PMT_Vector3: { assignment_node->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UAnalyticsPacketBuilder, AddFloat3), UAnalyticsPacketBuilder::StaticClass()); break; }
		}

		assignment_node->AllocateDefaultPins();

		pin_then_assignment->MakeLinkTo(assignment_node->GetExecPin());
		pin_then_assignment = assignment_node->GetThenPin();

		UEdGraphPin* ObjectPin = assignment_node->FindPin(UEdGraphSchema_K2::PN_Self);
		pin_packet_builder->MakeLinkTo(ObjectPin);

		UEdGraphPin* pin_value_assignment = assignment_node->FindPin(TEXT("Value"));

		if (pin_value_assignment != nullptr) 
		{
			if (data_pins[i]->LinkedTo.Num() == 0)
			{
				pin_value_assignment->DefaultValue = data_pins[i]->DefaultValue;
			}
			else
			{
				pin_value_assignment->PinType.PinCategory = data_pins[i]->PinType.PinCategory;
				pin_value_assignment->PinType.PinSubCategory = data_pins[i]->PinType.PinSubCategory;
				pin_value_assignment->PinType.PinSubCategoryObject = data_pins[i]->PinType.PinSubCategoryObject;
				CompilerContext.MovePinLinksToIntermediate(*data_pins[i], *pin_value_assignment);
			}
		}
	}

	// Create send node

	UK2Node_CallFunction* PacketSendNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	PacketSendNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UAnalyticsPacketBuilder, Send), UAnalyticsPacketBuilder::StaticClass());
	PacketSendNode->AllocateDefaultPins();

	UEdGraphPin* pin_send_exec = PacketSendNode->GetExecPin();
	UEdGraphPin* pin_send_then = PacketSendNode->GetThenPin();
	UEdGraphPin* pin_send_builder = PacketSendNode->FindPin(UEdGraphSchema_K2::PN_Self);

	pin_then_assignment->MakeLinkTo(pin_send_exec);
	pin_packet_builder->MakeLinkTo(pin_send_builder);
	CompilerContext.MovePinLinksToIntermediate(*pin_then, *pin_send_then);

	BreakAllNodeLinks();
}

#undef LOCTEXT_NAMESPACE