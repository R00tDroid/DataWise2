#pragma once
#include <string>


class ExportCollector
{
public:
	ExportCollector(std::string name);
	void Export();

	std::string GetDataDirectory();
	std::string GetRelativeDataDirectory();

	std::string GenerateId();

	void AddStyle(std::string data) { style_ += data; }
	void AddScript(std::string data) { script_ += data; }
	void AddChartData(std::string data) { chart_load_ += data; }
	void AddContent(std::string data) { content_ += data; }

private:
	std::string style_;
	std::string chart_load_;
	std::string script_;
	std::string content_;

	std::string name_;
};
