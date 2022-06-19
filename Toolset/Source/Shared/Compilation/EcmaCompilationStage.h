#pragma once
#include "CompilationStage.h"
#include "../Dependencies/duktape/duktape.h"
#include "Exporters/DataMap.h"
#include "Exporters/DataChart.h"
#include "Exporters/DataSheet.h"

#define ECMAOBJECT_DEFINE_REGISTRY(Class)	static std::map<std::string, Class*> Class##_Registry; static Class* Get(std::string object){ return Class##::##Class##_Registry[object]; }; static void Register(std::string object, Class* instance){ Class##_Registry.insert(std::pair<std::string, Class*>(object, instance)); }; static void Unregister(Class* instance){ for(auto it = Class##_Registry.begin(); it != Class##_Registry.end(); it++) { if(it->second == instance) { Class##_Registry.erase(it); } } };
#define ECMAOBJECT_IMPLEMENT_REGISTRY(Class)	std::map<std::string, Class*> Class##::##Class##_Registry;
#define ECMAOBJECT_IMPLEMENT_FUNCTION(Class, Function)	duk_ret_t Class##_##Function##_Relay(duk_context* ctx){ duk_push_this(ctx); duk_get_prop_string(ctx, -1, "id"); std::string object = duk_safe_to_string(ctx, -1); Class* instance = Class##::Get(object); if(instance == nullptr){ return DUK_RET_ERROR; } return instance->Function(ctx); }
#define ECMAOBJECT_IMPLEMENT_FUNCTION_STATIC(Class, Function)	duk_ret_t Class##_##Function##_Relay(duk_context* ctx){ return Class##::##Function(ctx); }
#define ECMAOBJECT_EXPOSE_FUNCTION_RAW(Context, Class, Function, ArgC)	duk_push_c_function(Context, Class##_##Function##_Relay, ArgC);
#define ECMAOBJECT_EXPOSE_FUNCTION(Context, Stack, Class, Function, ArgC)	ECMAOBJECT_EXPOSE_FUNCTION_RAW(Context, Class, Function, ArgC) duk_put_prop_string(Context, Stack, #Function);
#define ECMAOBJECT_EXPOSE_FUNCTION_NAMED(Context, Stack, Class, Function, Name, ArgC)	ECMAOBJECT_EXPOSE_FUNCTION_RAW(Context, Class, Function, ArgC) duk_put_prop_string(Context, Stack, Name);

#define ECMASTAGE_IMPLEMENT_FUNCTION(Function)	duk_ret_t EcmaCompilationstage_##Function##_Relay(duk_context* ctx){ EcmaCompilationstage* instance = GetStage(ctx); if(instance == nullptr){ return DUK_RET_ERROR; } return instance->Function(ctx); }
#define ECMASTAGE_EXPOSE_FUNCTION(Context, Function, ArgC)	ECMAOBJECT_EXPOSE_FUNCTION_RAW(Context, EcmaCompilationstage, Function, ArgC) duk_put_global_string(Context, #Function);
#define ECMASTAGE_EXPOSE_FUNCTION_NAMED(Context, Function, Name, ArgC)	ECMAOBJECT_EXPOSE_FUNCTION_RAW(Context, EcmaCompilationstage, Function, ArgC) duk_put_global_string(Context, Name);

class EcmaExporter
{
public:
	virtual void Cleanup() = 0;
};

class EcmaDataMapChannel
{
public:
	virtual void Cleanup() = 0;
};


class EcmaDataMapDensity : public EcmaDataMapChannel
{
public:
	EcmaDataMapDensity(DensityChannel* c) { channel = c; }
	duk_ret_t SetDensity(duk_context *ctx);
	duk_ret_t AddDensity(duk_context *ctx);
	duk_ret_t SetCellSize(duk_context *ctx);

	void Cleanup() override {}

	ECMAOBJECT_DEFINE_REGISTRY(EcmaDataMapDensity)
	
private:
	DensityChannel* channel;
};

class EcmaDataMap : public EcmaExporter
{
public:
	static duk_ret_t Construct(duk_context *ctx);
	void Cleanup() override { Unregister(this); for (EcmaDataMapChannel* channel : channels) { channel->Cleanup(); delete channel; } channels.clear(); }

	duk_ret_t SetResolution(duk_context *ctx);
	duk_ret_t SetView(duk_context *ctx);
	duk_ret_t SetImage(duk_context *ctx);

	duk_ret_t AddDensityChannel(duk_context *ctx);

	ECMAOBJECT_DEFINE_REGISTRY(EcmaDataMap);

private:
	DataMap* map;
	std::vector<EcmaDataMapChannel*> channels;
};


class EcmaDataChart : public EcmaExporter
{
public:
	static duk_ret_t Construct(duk_context *ctx);
	void Cleanup() override { Unregister(this); }

	duk_ret_t AddRow(duk_context *ctx);

	ECMAOBJECT_DEFINE_REGISTRY(EcmaDataChart);

private:
	DataChart* chart;
};


class EcmaDataSheet : public EcmaExporter
{
public:
	static duk_ret_t Construct(duk_context *ctx);
	void Cleanup() override { Unregister(this); }

	duk_ret_t AddRow(duk_context *ctx);

	ECMAOBJECT_DEFINE_REGISTRY(EcmaDataSheet);

private:
	DataSheet* sheet;
};


class EcmaCompilationstage : public ICompilationStage
{
public:
	static EcmaCompilationstage* Create(std::string script);

	void Init() override;
	void Cleanup() override;

	void Prepare() override;
	void ProcessSession(Session&) override;
	void Export() override;

	duk_ret_t GetPacketsOfType(duk_context *ctx);
	duk_ret_t GetPacketsContaining(duk_context *ctx);
	duk_ret_t GetSessionName(duk_context *ctx);

	ECMAOBJECT_DEFINE_REGISTRY(EcmaCompilationstage);

	std::vector<EcmaExporter*> specialized_exporters;
	
private:
	duk_context* context_ = nullptr;
	std::string script_;
	Session* current_session_ = nullptr;

	bool BindFunction(std::string);
	void PushPacketList(duk_context*, std::vector<std::string>&);
};

class EcmaStageRegistrator : public ICompilationStageRegistry
{
protected:
	std::vector<std::string> GetAvailableStages_() override;
	ICompilationStage* CreateStage_(std::string) override;
};