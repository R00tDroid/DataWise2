#include "EcmaCompilationStage.h"
#include "Compiler.h"
#include "../Dependencies/picojson.h"
#include "../Platform/Platform.h"
#include "../../Server/Networking/StorageConnectionBase.h"

REGISTER_COMPILATIONSTAGE_TYPE(EcmaStageRegistrator, "ecma")

std::map<duk_context*, EcmaCompilationstage*> stages;
EcmaCompilationstage* GetStage(duk_context* ctx)
{
	return stages[ctx];
}

void DumpStack(duk_context *ctx)
{
	duk_push_context_dump(ctx);
	GetStage(ctx)->Log("Ecma > " + std::string(duk_to_string(ctx, -1)));
	duk_pop(ctx);
}

std::string GenerateObjectId()
{
	std::string object_id = "";
	for (int i = 0; i < 12; i++)
	{
		object_id += char(97 + rand() % 26);
	}
	return object_id;
}

duk_ret_t ecma_print(duk_context *ctx) 
{
	if(duk_is_string(ctx, 0) || duk_is_number(ctx, 0)) GetStage(ctx)->Log("Ecma > " + std::string(duk_to_string(ctx, 0)));
	return 0;
}

std::vector<std::string> ArrayToVector(duk_context *ctx, int stack)
{
	std::vector<std::string> data;

	duk_size_t i, n;
	n = duk_get_length(ctx, stack);
	for (i = 0; i < n; i++)
	{
		duk_get_prop_index(ctx, stack, i);
		data.push_back(duk_to_string(ctx, -1));
		duk_pop(ctx);
	}

	return data;
}

ECMAOBJECT_IMPLEMENT_REGISTRY(EcmaDataMap)
ECMAOBJECT_IMPLEMENT_FUNCTION_STATIC(EcmaDataMap, Construct)
ECMAOBJECT_IMPLEMENT_FUNCTION(EcmaDataMap, AddDensityChannel)
ECMAOBJECT_IMPLEMENT_FUNCTION(EcmaDataMap, SetResolution)
ECMAOBJECT_IMPLEMENT_FUNCTION(EcmaDataMap, SetView)
ECMAOBJECT_IMPLEMENT_FUNCTION(EcmaDataMap, SetImage)

ECMAOBJECT_IMPLEMENT_REGISTRY(EcmaDataMapDensity)
ECMAOBJECT_IMPLEMENT_FUNCTION(EcmaDataMapDensity, AddDensity)
ECMAOBJECT_IMPLEMENT_FUNCTION(EcmaDataMapDensity, SetDensity)
ECMAOBJECT_IMPLEMENT_FUNCTION(EcmaDataMapDensity, SetCellSize)

ECMAOBJECT_IMPLEMENT_REGISTRY(EcmaDataChart)
ECMAOBJECT_IMPLEMENT_FUNCTION_STATIC(EcmaDataChart, Construct)
ECMAOBJECT_IMPLEMENT_FUNCTION(EcmaDataChart, AddRow)

ECMAOBJECT_IMPLEMENT_REGISTRY(EcmaDataSheet)
ECMAOBJECT_IMPLEMENT_FUNCTION_STATIC(EcmaDataSheet, Construct)
ECMAOBJECT_IMPLEMENT_FUNCTION(EcmaDataSheet, AddRow)

ECMASTAGE_IMPLEMENT_FUNCTION(GetPacketsOfType)
ECMASTAGE_IMPLEMENT_FUNCTION(GetPacketsContaining)
ECMASTAGE_IMPLEMENT_FUNCTION(GetSessionName)

duk_ret_t EcmaDataMap::Construct(duk_context* ctx)
{
	if (!duk_is_constructor_call(ctx)) {
		return DUK_RET_TYPE_ERROR;
	}

	std::string object_id = GenerateObjectId();

	duk_push_this(ctx);

	duk_dup(ctx, 0);
	std::string name = duk_safe_to_string(ctx, 0);

	duk_put_prop_string(ctx, -2, "name");

	duk_push_string(ctx, object_id.c_str());
	duk_put_prop_string(ctx, -2, "id");

	EcmaDataMap* new_map = new EcmaDataMap();
	EcmaDataMap::Register(object_id, new_map);

	new_map->map = DataMap::Create(GetStage(ctx), name, { 512, 512 }, { 10, 10 }, { 0, 0 });
	GetStage(ctx)->specialized_exporters.push_back(new_map);

	return 0;
}

duk_ret_t EcmaDataMapDensity::AddDensity(duk_context* ctx)
{
	channel->AddDensity({ (float)duk_get_number(ctx, 0), (float)duk_get_number(ctx, 1) }, duk_get_number(ctx, 2));

	return 0;
}

duk_ret_t EcmaDataMapDensity::SetDensity(duk_context* ctx)
{
	channel->SetDensity({ (float)duk_get_number(ctx, 0), (float)duk_get_number(ctx, 1) }, duk_get_number(ctx, 2));

	return 0;
}

duk_ret_t EcmaDataMapDensity::SetCellSize(duk_context* ctx)
{
	channel->SetCellSize({ (float)duk_get_number(ctx, 0), (float)duk_get_number(ctx, 1) });
	return 0;
}


duk_ret_t EcmaDataMap::SetResolution(duk_context* ctx)
{
	map->SetResolution({ (int)duk_get_number(ctx, 0), (int)duk_get_number(ctx, 1) });
	return 0;
}

duk_ret_t EcmaDataMap::SetView(duk_context* ctx)
{
	map->SetView({ (float)duk_get_number(ctx, 0), (float)duk_get_number(ctx, 1) }, {(float)duk_get_number(ctx, 2), (float)duk_get_number(ctx, 3)});
	return 0;
}

duk_ret_t EcmaDataMap::SetImage(duk_context* ctx)
{
	map->SetBackground(duk_safe_to_string(ctx, 0));
	return 0;
}

duk_ret_t EcmaDataMap::AddDensityChannel(duk_context* ctx)
{
	std::string name = duk_safe_to_string(ctx, 0);

	std::string unique_id = GenerateObjectId();

	DensityChannel* channel = map->AddChannel<DensityChannel>(name);
	EcmaDataMapDensity* channel_wrapper = new EcmaDataMapDensity(channel);
	channels.push_back(channel_wrapper);
	EcmaDataMapDensity::Register(unique_id, channel_wrapper);

	duk_idx_t object_index = duk_push_object(ctx);
	duk_push_string(ctx, unique_id.c_str());
	duk_put_prop_string(ctx, object_index, "id");

	ECMAOBJECT_EXPOSE_FUNCTION(ctx, object_index, EcmaDataMapDensity, AddDensity, 3)
	ECMAOBJECT_EXPOSE_FUNCTION(ctx, object_index, EcmaDataMapDensity, SetDensity, 3)
	ECMAOBJECT_EXPOSE_FUNCTION(ctx, object_index, EcmaDataMapDensity, SetCellSize, 2)

	return 1;
}

duk_ret_t EcmaDataChart::Construct(duk_context* ctx)
{
	if (!duk_is_constructor_call(ctx)) {
		return DUK_RET_TYPE_ERROR;
	}

	std::string object_id = GenerateObjectId();

	if (!duk_is_array(ctx, 1) || !duk_is_array(ctx, 2) || !duk_is_array(ctx, 3)) { GetStage(ctx)->Log("Invalid DataChart arguments"); return DUK_RET_TYPE_ERROR; }

	std::vector<std::string> chart_types = ArrayToVector(ctx, 1);
	std::vector<std::string> column_names = ArrayToVector(ctx, 2);
	std::vector<std::string> column_types = ArrayToVector(ctx, 3);

	std::vector<DataChart::Type> chart_types_converted;
	std::vector<DataChart::DataType> column_types_converted;

	for(std::string& type : chart_types)
	{
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
		if (type == std::string("table")) { chart_types_converted.push_back(DataChart::Type::Table); }
		else if (type == std::string("line")) { chart_types_converted.push_back(DataChart::Type::Line); }
		else if (type == std::string("bar")) { chart_types_converted.push_back(DataChart::Type::Bar); }
		else if (type == std::string("column")) { chart_types_converted.push_back(DataChart::Type::Column); }
		else if (type == std::string("pie")) { chart_types_converted.push_back(DataChart::Type::Pie); }
		else if (type == std::string("scatter")) { chart_types_converted.push_back(DataChart::Type::Scatter); }
		else if (type == std::string("timeline")) { chart_types_converted.push_back(DataChart::Type::Timeline); }
		else
		{
			GetStage(ctx)->Log("Unknown chart type: " + type);
		}
	}

	for(std::string& type : column_types)
	{
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
		if (type == std::string("string")) { column_types_converted.push_back(DataChart::DataType::String); }
		else if (type == std::string("number")) { column_types_converted.push_back(DataChart::DataType::Number); }
		else if (type == std::string("boolean")) { column_types_converted.push_back(DataChart::DataType::Boolean); }
		else if (type == std::string("seconds")) { column_types_converted.push_back(DataChart::DataType::Seconds); }
		else if (type == std::string("milliseconds")) { column_types_converted.push_back(DataChart::DataType::Milliseconds); }
		else
		{
			GetStage(ctx)->Log("Unknown chart data type: " + type);
		}
	}

	duk_push_this(ctx);

	duk_dup(ctx, 0);
	std::string name = duk_safe_to_string(ctx, 0);

	duk_put_prop_string(ctx, -2, "name");

	duk_push_string(ctx, object_id.c_str());
	duk_put_prop_string(ctx, -2, "id");

	EcmaDataChart* new_chart = new EcmaDataChart();
	EcmaDataChart::Register(object_id, new_chart);

	new_chart->chart = DataChart::Create(GetStage(ctx), name, chart_types_converted, column_names, column_types_converted);
	GetStage(ctx)->specialized_exporters.push_back(new_chart);

	return 0;
}

duk_ret_t EcmaDataChart::AddRow(duk_context* ctx)
{
	if (!duk_is_array(ctx, 0)) { GetStage(ctx)->Log("Invalid row data"); return DUK_RET_TYPE_ERROR; }
	std::vector<std::string> data = ArrayToVector(ctx, 0);
	chart->AddRow(data);
	return 0;
}

duk_ret_t EcmaDataSheet::Construct(duk_context* ctx)
{
	if (!duk_is_constructor_call(ctx)) {
		return DUK_RET_TYPE_ERROR;
	}

	std::string object_id = GenerateObjectId();

	if (!duk_is_array(ctx, 1) || !duk_is_array(ctx, 2)) { GetStage(ctx)->Log("Invalid DataSheet arguments"); return DUK_RET_TYPE_ERROR; }

	std::vector<std::string> column_names = ArrayToVector(ctx, 1);
	std::vector<std::string> column_types = ArrayToVector(ctx, 2);

	std::vector<DataSheet::DataType> column_types_converted;

	for (std::string& type : column_types)
	{
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
		if (type == std::string("string")) { column_types_converted.push_back(DataSheet::DataType::String); }
		else if (type == std::string("number")) { column_types_converted.push_back(DataSheet::DataType::Number); }
		else if (type == std::string("boolean")) { column_types_converted.push_back(DataSheet::DataType::Boolean); }
		else if (type == std::string("time")) { column_types_converted.push_back(DataSheet::DataType::Time); }
		else
		{
			GetStage(ctx)->Log("Unknown chart sheet type: " + type);
		}
	}

	duk_push_this(ctx);

	duk_dup(ctx, 0);
	std::string name = duk_safe_to_string(ctx, 0);

	duk_put_prop_string(ctx, -2, "name");

	duk_push_string(ctx, object_id.c_str());
	duk_put_prop_string(ctx, -2, "id");

	EcmaDataSheet* new_sheet = new EcmaDataSheet();
	EcmaDataSheet::Register(object_id, new_sheet);

	new_sheet->sheet = DataSheet::Create(GetStage(ctx), name, column_names, column_types_converted);
	GetStage(ctx)->specialized_exporters.push_back(new_sheet);

	return 0;
}

duk_ret_t EcmaDataSheet::AddRow(duk_context* ctx)
{
	if (!duk_is_array(ctx, 0)) { GetStage(ctx)->Log("Invalid row data"); return DUK_RET_TYPE_ERROR; }
	std::vector<std::string> data = ArrayToVector(ctx, 0);
	sheet->AddRow(data);
	return 0;
}

EcmaCompilationstage* EcmaCompilationstage::Create(std::string script)
{
	EcmaCompilationstage* stage = new EcmaCompilationstage();
	stage->script_ = script;
	return stage;
}

void EcmaCompilationstage::Init()
{
	context_ = duk_create_heap_default();
	if (context_ == nullptr)
	{
		Log("Could not create Ecma environment");
		Cleanup();
		return;
	}

	stages.insert(std::pair<duk_context*, EcmaCompilationstage*>(context_, this));

	duk_push_lstring(context_, script_.c_str(), script_.length());

	if (duk_peval(context_) != 0)
	{
		Log("Ecma parsing failed:");
		Log("> " + std::string(duk_safe_to_string(context_, -1)));
		Cleanup();
		return;
	}

	duk_pop(context_);

	duk_push_c_function(context_, ecma_print, 1);
	duk_put_global_string(context_, "print");

	ECMASTAGE_EXPOSE_FUNCTION(context_, GetPacketsOfType, 1)
	ECMASTAGE_EXPOSE_FUNCTION(context_, GetPacketsContaining, 1)
	ECMASTAGE_EXPOSE_FUNCTION(context_, GetSessionName, 0)

	ECMAOBJECT_EXPOSE_FUNCTION_RAW(context_, EcmaDataMap, Construct, 1)
	duk_push_object(context_);
	ECMAOBJECT_EXPOSE_FUNCTION(context_, -2, EcmaDataMap, AddDensityChannel, 1)
	ECMAOBJECT_EXPOSE_FUNCTION(context_, -2, EcmaDataMap, SetResolution, 2)
	ECMAOBJECT_EXPOSE_FUNCTION(context_, -2, EcmaDataMap, SetView, 4)
	ECMAOBJECT_EXPOSE_FUNCTION(context_, -2, EcmaDataMap, SetImage, 1)
	duk_put_prop_string(context_, -2, "prototype");
	duk_put_global_string(context_, "DataMap");

	ECMAOBJECT_EXPOSE_FUNCTION_RAW(context_, EcmaDataChart, Construct, 4)
	duk_push_object(context_);
	ECMAOBJECT_EXPOSE_FUNCTION(context_, -2, EcmaDataChart, AddRow, 1)
	duk_put_prop_string(context_, -2, "prototype");
	duk_put_global_string(context_, "DataChart");

	ECMAOBJECT_EXPOSE_FUNCTION_RAW(context_, EcmaDataSheet, Construct, 4)
	duk_push_object(context_);
	ECMAOBJECT_EXPOSE_FUNCTION(context_, -2, EcmaDataSheet, AddRow, 1)
	duk_put_prop_string(context_, -2, "prototype");
	duk_put_global_string(context_, "DataSheet");

	DataMap* WorldMap = new DataMap();
	WorldMap->SetView(Float2(32000, 32000), Float2(0, -4000));
	WorldMap->SetResolution(Int2(2048, 2048));
	WorldMap->SetBackground("aether");
}

void EcmaCompilationstage::Cleanup()
{
	if (context_ != nullptr) 
	{
		if (stages.find(context_) != stages.end()) stages.erase(stages.find(context_));
		duk_destroy_heap(context_); 
		context_ = nullptr;
	}

	for(EcmaExporter* exporter : specialized_exporters)
	{
		exporter->Cleanup();
		delete exporter;
	}
	specialized_exporters = {};

	ICompilationStage::Cleanup();
}


void EcmaCompilationstage::Prepare()
{
	if (context_ != nullptr) 
	{
		if (BindFunction("Prepare"))
		{
			if (duk_pcall(context_, 0) != DUK_EXEC_SUCCESS)
			{
				Log("Ecma execution (Prepare) failed:");
				Log("> " + std::string(duk_safe_to_string(context_, -1)));
			}
		}
	}
}

void EcmaCompilationstage::ProcessSession(Session& session)
{
	current_session_ = &session;

	if (context_ != nullptr)
	{
		if (BindFunction("Process"))
		{
			if (duk_pcall(context_, 0) != DUK_EXEC_SUCCESS)
			{
				Log("Ecma execution (Process) failed:");
				Log("> " + std::string(duk_safe_to_string(context_, -1)));
			}
		}
	}

	current_session_ = nullptr;
}

void EcmaCompilationstage::Export()
{
	if (context_ != nullptr)
	{
		if (BindFunction("Export"))
		{
			if (duk_pcall(context_, 0) != DUK_EXEC_SUCCESS)
			{
				Log("Ecma execution (Export) failed:");
				Log("> " + std::string(duk_safe_to_string(context_, -1)));
			}
		}
	}
}

bool EcmaCompilationstage::BindFunction(std::string name)
{
	duk_push_global_object(context_);

	if (duk_get_global_string(context_, name.c_str()) == 0)
	{
		Log("Could not find function '" + name + "'");
		return false;
	}

	return true;
}

std::vector<std::string> EcmaStageRegistrator::GetAvailableStages_()
{
	std::vector<std::string> files = FindFiles(GetPreferencesPath() + "\\CompilerScripts", "*.js");
	for(std::string& file : files)
	{
		file = ReplaceString(file, ".js", "");
	}

	return files;
}

ICompilationStage* EcmaStageRegistrator::CreateStage_(std::string stage_name)
{
	std::string path = GetPreferencesPath() + "\\CompilerScripts\\" + stage_name + ".js";

	FILE* f;
	fopen_s(&f, path.c_str(), "r");

	if (f == nullptr) return nullptr;

	fseek(f, 0, SEEK_END);
	int length = ftell(f);
	fseek(f, 0, SEEK_SET);

	std::string data;
	data.resize(length);

	fread((void*)data.data(), sizeof(char), length, f);

	fclose(f);

	std::string converted_data(data.c_str());

	return EcmaCompilationstage::Create(converted_data);
}

duk_ret_t EcmaCompilationstage::GetPacketsOfType(duk_context* ctx)
{
	if (current_session_ == nullptr) return -1;

	std::string type = duk_to_string(ctx, 0);

	std::vector<std::string> packets = current_session_->GetPacketsOfType(type);

	PushPacketList(ctx, packets);

	return 1;
}

duk_ret_t EcmaCompilationstage::GetPacketsContaining(duk_context* ctx)
{
	if (current_session_ == nullptr) return -1;

	std::string property_name = duk_to_string(ctx, 0);

	std::vector<std::string> packets = current_session_->GetPacketsContaining(property_name);

	PushPacketList(ctx, packets);

	return 1;
}

duk_ret_t EcmaCompilationstage::GetSessionName(duk_context* ctx)
{
	duk_push_string(ctx, current_session_->Name.c_str());
	return 1;
}

void EcmaCompilationstage::PushPacketList(duk_context* ctx, std::vector<std::string>& packets)
{
	duk_idx_t array_index = duk_push_array(ctx);

	int packet_index = 0;
	for (std::string& packet_string : packets)
	{
		picojson::value packet_value;
		picojson::parse(packet_value, packet_string);
		if (!packet_value.is<picojson::array>()) { Log("Invalid packet structure: " + packet_string); break; }

		picojson::array& packet = packet_value.get<picojson::array>();
		duk_idx_t object_index = duk_push_object(ctx);
		duk_push_string(ctx, packet[0].get<std::string>().c_str());
		duk_put_prop_string(ctx, object_index, "PacketType");

		for (int i = 1; i < packet.size(); i++)
		{
			picojson::array member = packet[i].get<picojson::value::array>();
			std::string member_type = member[1].get<std::string>();

			if (member_type == std::string("string"))
			{
				duk_push_string(ctx, member[2].get<std::string>().c_str());
			}
			else if (member_type == std::string("int"))
			{
				duk_push_int(ctx, member[2].get<double>());
			}
			else if (member_type == std::string("float") || member_type == std::string("time"))
			{
				duk_push_number(ctx, member[2].get<double>());
			}
			else
			{
				Log("Unknown object member type: " + member_type);
				break;
			}
			duk_put_prop_string(ctx, object_index, member[0].get<std::string>().c_str());
		}

		duk_put_prop_index(ctx, array_index, packet_index);
		packet_index++;
	}
}
