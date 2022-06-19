#include "DataChart.h"
#include "../Compiler.h"

DataChart* DataChart::Create(ICompilationStage* stage, std::string name, std::vector<Type> charts, std::vector<std::string> columns, std::vector<DataType> types)
{
	if (columns.size() != types.size())
	{
		Compiler::CompilerLog("Invalid column signature! Rows and types do not match");
		return nullptr;
	}

	DataChart* chart = new DataChart();
	stage->AddExporter(chart);

	chart->name_ = name;
	chart->column_names_ = columns;
	chart->column_types_ = types;
	chart->chart_types_ = charts;

	return chart;
}

void DataChart::AddRow(std::vector<std::string> data)
{
	if (data.size() != column_types_.size())
	{
		Compiler::CompilerLog("Added row does not match column signature");
		return;
	}

	rows_.push_back(data);
}

void DataChart::Export(ExportCollector& collector)
{
	if (rows_.empty()) return;

	std::string id = collector.GenerateId();

	std::string data_name = "data" + id;

	collector.AddChartData("var " + data_name + " = new google.visualization.DataTable();");

	for (int i = 0; i < column_names_.size(); i++) 
	{
		std::string type;
		switch (column_types_[i])
		{
		case String: { type = "string"; break; }
		case Number: { type = "number"; break; }
		case Boolean: { type = "boolean"; break; }
		case Seconds: { type = "timeofday"; break; }
		case Milliseconds: { type = "number"; break; }
		}
		collector.AddChartData(data_name + ".addColumn('" + type + "', '" + column_names_[i] + "');");
	}


	collector.AddChartData(data_name + ".addRows([");
	for (std::vector<std::string>& row : rows_)
	{
		collector.AddChartData("[");

		for (int id = 0; id < row.size(); id++)
		{
			switch (column_types_[id])
			{
			case String: { collector.AddChartData("'" + row[id] + "'"); break; }
			case Number: { collector.AddChartData("{v: " + row[id] + ", f: '" + row[id] + "'}");  break; }
			case Boolean: { collector.AddChartData(row[id]); break; }
			case Seconds:
			{
				int seconds = std::stoi(row[id]);
				int minutes = std::floor(seconds / 60.0f);
				int hours = std::floor(minutes / 60.0f);

				seconds -= minutes * 60;
				minutes -= hours * 60;

				collector.AddChartData("[" + std::to_string(hours) + "," + std::to_string(minutes) + "," + std::to_string(seconds) + ",0]"); break;
			}

			case Milliseconds: { collector.AddChartData("{v: " + row[id] + ", f: '" + row[id] + "'}");  break; }
			}

			if (id < row.size() - 1) { collector.AddChartData(","); }
		}

		collector.AddChartData("],");
	}
	collector.AddChartData("]);");



	collector.AddContent("<div class='chart_parent' style='max-width: 100%; overflow-x: auto; overflow-y: hidden;'>");
	collector.AddContent("<h1>" + name_ + "</h1>");

	collector.AddContent("<div style='position:relative; line-height: 500px;'>");

	int chart_id = 0;
	for (Type chart : chart_types_)
	{
		std::string chart_name = "chart" + id + "_" + std::to_string(chart_id);
		std::string chart_id_name = "chart_id" + id + "_" + std::to_string(chart_id);
		collector.AddContent("<div class='chart' id='" + chart_name + "' style='" + ((chart == Table) ? "max-" : "") + "width: 800px; height: 500px; vertical-align: middle; line-height: normal;'></div>");

		switch (chart) {
		case Table: { collector.AddChartData("var " + chart_id_name + "=new google.visualization.Table(document.getElementById('" + chart_name + "')); " + chart_id_name + ".draw(" + data_name + "); "); break; }
		case Line: { collector.AddChartData("var " + chart_id_name + "=new google.visualization.LineChart(document.getElementById('" + chart_name + "')); " + chart_id_name + ".draw(" + data_name + ", {legend: 'bottom'}); "); break; }
		case Bar: { collector.AddChartData("var " + chart_id_name + "=new google.visualization.BarChart(document.getElementById('" + chart_name + "')); " + chart_id_name + ".draw(" + data_name + ", {bar: {groupWidth: '95%'}}); "); break; }
		case Column: { collector.AddChartData("var " + chart_id_name + "=new google.visualization.ColumnChart(document.getElementById('" + chart_name + "')); " + chart_id_name + ".draw(" + data_name + ", {bar: {groupWidth: '95%'}}); "); break; }
		case Pie: { collector.AddChartData("var " + chart_id_name + "=new google.visualization.PieChart(document.getElementById('" + chart_name + "')); " + chart_id_name + ".draw(" + data_name + "); "); break; }
		case Scatter: { collector.AddChartData("var " + chart_id_name + "=new google.visualization.ScatterChart(document.getElementById('" + chart_name + "')); " + chart_id_name + ".draw(" + data_name + "); "); break; }
		case Timeline: { collector.AddChartData("var " + chart_id_name + "=new google.visualization.Timeline(document.getElementById('" + chart_name + "')); " + chart_id_name + ".draw(" + data_name + "); "); break; } //TODO implement datetime for timline
		}

		chart_id++;
	}

	collector.AddContent("</div></div><hr>");
}

