#include "DataWise.h"
#include "Modules/ModuleManager.h"
#include "EngineUtils.h"
#include "Interfaces/IPluginManager.h"
#include "RecordableSession.h"
#include "SlateStyleRegistry.h"

DEFINE_LOG_CATEGORY(AnalyticsLog);

IMPLEMENT_MODULE(DataWiseModule, DataWise)

void LogCallback(int type, char* message)
{
	switch (type)
	{
	case 0: { UE_LOG(AnalyticsLog, Display, TEXT("%s"), UTF8_TO_TCHAR(message)); break; }
	case 1: { UE_LOG(AnalyticsLog, Warning, TEXT("%s"), UTF8_TO_TCHAR(message)); break; }
	case 2: { UE_LOG(AnalyticsLog, Error, TEXT("%s"), UTF8_TO_TCHAR(message)); break; }
	}
	
}

void DataWiseModule::StartupModule()
{
	FString module_directory = IPluginManager::Get().FindPlugin("DataWise")->GetBaseDir();

#ifdef PLATFORM_WINDOWS
	library_ = FPlatformProcess::GetDllHandle(*(module_directory + "/ThirdParty/Bin/windows_x64/core.dll"));
#endif

	SetLogCallback(LogCallback);


	packet_style_ = MakeShareable(new FSlateStyleSet("DataWisePacketStyle"));
	packet_style_->SetContentRoot(module_directory);
	FSlateImageBrush* packeticon_brush = new FSlateImageBrush(packet_style_->RootToContentDir(TEXT("Resources/PacketIcon"), TEXT(".png")), FVector2D(128.f, 128.f));

	if (packeticon_brush)
	{
		packet_style_->Set("ClassThumbnail.AnalyticsPacket", packeticon_brush);
		FSlateStyleRegistry::RegisterSlateStyle(*packet_style_);
	}
}

void DataWiseModule::ShutdownModule()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(packet_style_->GetStyleSetName());

	if (library_ != nullptr) 
	{

#ifdef PLATFORM_WINDOWS
		FPlatformProcess::FreeDllHandle(library_);
#endif

		library_ = nullptr;
	}
}
