#pragma once
#include "../CompilationStage.h"


class DataSheet : public ICompilationExporter
{
public:

	enum DataType 
	{
		String,
		Number,
		Boolean,
		Time
	};

	static DataSheet* Create(ICompilationStage* stage, std::string name, std::vector<std::string> columns, std::vector<DataType> types);

	void AddRow(std::vector<std::string>);

	void Export(ExportCollector&) override;

private:
	std::string name_;

	std::vector<std::string> column_names_;
	std::vector<DataType> column_types_;
	char seperator_;
	char decimal_seperator_;
	std::vector<std::vector<std::string>> rows_;
};