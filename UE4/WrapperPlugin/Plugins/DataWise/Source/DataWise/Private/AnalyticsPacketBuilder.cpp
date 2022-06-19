#include "AnalyticsPacketBuilder.h"

UAnalyticsPacketBuilder* UAnalyticsPacketBuilder::CreatePacket(UAnalyticsSession* Session, FString PacketName)
{
	UAnalyticsPacketBuilder* builder = NewObject<UAnalyticsPacketBuilder>();
	builder->AddToRoot();
	builder->builder_ = new PacketBuilder(Session->GetNativeSession(), TCHAR_TO_UTF8(*PacketName));
	return builder;
}

void UAnalyticsPacketBuilder::Send()
{
	builder_->Send();
	delete builder_;
	builder_ = nullptr;

	RemoveFromRoot();
	ConditionalBeginDestroy();
}

void UAnalyticsPacketBuilder::AddTime()
{
	builder_->AddTimestamp();
}

void UAnalyticsPacketBuilder::AddInt(int value)
{
	builder_->Add(value);
}

void UAnalyticsPacketBuilder::AddFloat(float value)
{
	builder_->Add(value);
}

void UAnalyticsPacketBuilder::AddString(FString value)
{
	builder_->Add(TCHAR_TO_UTF8(*value));
}

void UAnalyticsPacketBuilder::AddFloat2(FVector2D value)
{
	builder_->Add(value.X);
	builder_->Add(value.Y);
}

void UAnalyticsPacketBuilder::AddFloat3(FVector value)
{
	builder_->Add(value.X);
	builder_->Add(value.Y);
	builder_->Add(value.Z);
}
