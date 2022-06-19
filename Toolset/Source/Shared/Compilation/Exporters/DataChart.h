#pragma once
#include "../CompilationStage.h"


class DataChart : public ICompilationExporter
{
public:

	enum Type 
	{
		Table,
		Line,
		Bar,
		Column,
		Pie,
		Scatter,
		Timeline
	};

	enum DataType 
	{
		String,
		Number,
		Boolean,
		Seconds,
		Milliseconds
	};

	static DataChart* Create(ICompilationStage* stage, std::string name, std::vector<Type> charts, std::vector<std::string> columns, std::vector<DataType> types);

	void AddRow(std::vector<std::string>);

	void Export(ExportCollector&) override;

private:
	std::string name_;

	std::vector<std::string> column_names_;
	std::vector<DataType> column_types_;

	std::vector<std::vector<std::string>> rows_;

	std::vector<Type> chart_types_;
};