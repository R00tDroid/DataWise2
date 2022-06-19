#include "DataSheet.h"
#include "../Compiler.h"
#include "../../Platform/Platform.h"

DataSheet* DataSheet::Create(ICompilationStage* stage, std::string name, std::vector<std::string> columns, std::vector<DataType> types)
{
	if (columns.size() != types.size())
	{
		Compiler::CompilerLog("Invalid column signature! Rows and types do not match");
		return nullptr;
	}

	DataSheet* chart = new DataSheet();
	stage->AddExporter(chart);

	chart->name_ = name;
	chart->column_names_ = columns;
	chart->column_types_ = types;
	chart->seperator_ = ';';
	chart->decimal_seperator_ = ',';

	return chart;
}

void DataSheet::AddRow(std::vector<std::string> data)
{
	if (data.size() != column_types_.size())
	{
		Compiler::CompilerLog("Added row does not match column signature");
		return;
	}

	rows_.push_back(data);
}

void DataSheet::Export(ExportCollector& collector)
{
	std::string data;

	for (std::string& column : column_names_)
	{
		data += column + seperator_;
	}
	data += "\n";

	for (std::vector<std::string>& row : rows_)
	{
		for (int id = 0; id < row.size(); id++)
		{
			switch (column_types_[id])
			{
			case String: { data += row[id] + seperator_; break; }
			case Number: { data += ReplaceString(row[id], ".", std::string(1, decimal_seperator_)) + seperator_; break; }
			case Boolean: { data += row[id] + seperator_; break; }
			case Time:
			{
				int seconds = std::atoi(row[id].c_str());
				int minutes = std::floor(seconds / 60.0f);
				int hours = std::floor(minutes / 60.0f);

				seconds -= minutes * 60;
				minutes -= hours * 60;

				data += std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(seconds) + seperator_; break;
			}
			}
		}
		data += "\n";
	}

	FILE* f;
	if(fopen_s(&f, (collector.GetDataDirectory() + "\\" + name_ + ".csv").c_str(), "w")==0)
	{
		fwrite(data.c_str(), sizeof(char), data.length(), f);
		fclose(f);
	}
}

