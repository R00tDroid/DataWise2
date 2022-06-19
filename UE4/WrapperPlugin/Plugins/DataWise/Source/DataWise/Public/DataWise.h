#pragma once

#include "Modules/ModuleInterface.h"
#include "Logging/LogMacros.h"
#include "SlateStyle.h"

DECLARE_LOG_CATEGORY_EXTERN(AnalyticsLog, Log, All);

class DataWiseModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void* library_ = nullptr;
	TSharedPtr<FSlateStyleSet> packet_style_;
};