#include "ExportCollector.h"
#include "../Platform/Platform.h"

const char* html_base = "<!DOCTYPE html><html><head><title><!-- title --></title><style>.chart_parent{ width:100%; } .chart{ display:inline-block; } .map_layer{ cursor: grab; } .map_parent{ position: relative; } .map_holder{ float: left; position: relative;  overflow:hidden; background: black; } .map_image{ position: absolute; top : 0; right : 0; } .layer_list > span{ user-select: none; margin-left: 10px; } .layer_list{ float: left; padding-left: 10px; }.layer_switch{ position: relative; display: inline-block; width: 40px; height: 20px; } .layer_switch input{ opacity: 0; width: 0; height: 0; } .slider{ position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; -webkit-transition: .2s; transition: .2s; border-radius: 34px; } .slider:before{ position: absolute; content: \"\"; height: 16px; width: 16px; left: 2px; bottom: 2px; background-color: white; -webkit-transition: .4s; transition: .4s; border-radius: 50%; } input:checked + .slider{ background-color: #2196F3; } input:focus + .slider{ box-shadow: 0 0 1px #2196F3; } input:checked + .slider:before{ -webkit-transform: translateX(20px); -ms-transform: translateX(20px); transform: translateX(20px); } .map_parent > button{ position: absolute; z-index: 10; left: 15px; width:20px; height:20px; } .btn_zoomin{ top: 15px; } .btn_zoomout{ top: 40px; } <!-- style --></style></head><body><!-- content --><script type = 'text/javascript' src = 'https://www.gstatic.com/charts/loader.js'></script><script type = 'text/javascript'>google.charts.load('current', { 'packages':['corechart'] }); google.charts.load('current', { 'packages':['table'] }); google.charts.load('current', {'packages':['timeline']}); google.charts.setOnLoadCallback(drawCharts);function drawCharts() {<!-- chart_script -->} window.onload = function() {<!-- script -->}</script></html>";

ExportCollector::ExportCollector(std::string name)
{
	name_ = name;
}

void ExportCollector::Export()
{
	if (content_.empty() && script_.empty()) return;

	std::string path = GetPreferencesPath() + "\\Output";
	CreateDirectoryTree(path);
	std::string filename = path + "\\" + name_ + ".html";

	std::string export_data = html_base;
	export_data = ReplaceString(export_data, "<!-- title -->", name_);
	export_data = ReplaceString(export_data, "<!-- style -->", style_);
	export_data = ReplaceString(export_data, "<!-- script -->", script_);
	export_data = ReplaceString(export_data, "<!-- chart_script -->", chart_load_);
	export_data = ReplaceString(export_data, "<!-- content -->", content_);

	FILE* f;
	if (fopen_s(&f, filename.c_str(), "w") != 0) return;
	fwrite(export_data.c_str(), sizeof(char), export_data.length(), f);
	fclose(f);
}

std::string ExportCollector::GetDataDirectory()
{
	std::string path = GetPreferencesPath() + "\\Output\\" + name_;
	CreateDirectoryTree(path);
	return path;
}

std::string ExportCollector::GetRelativeDataDirectory()
{
	std::string path = GetPreferencesPath() + "\\Output\\" + name_;
	CreateDirectoryTree(path);

	return name_;
}

std::string ExportCollector::GenerateId()
{
	std::string object_id = "";
	for (int i = 0; i < 12; i++)
	{
		object_id += char(97 + rand() % 26);
	}
	return object_id;
}
