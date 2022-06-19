#include "CompilationStage.h"
#include "Compiler.h"

void ICompilationStage::Assign(Compiler* parent)
{
	parent_ = parent;
}

void ICompilationStage::Cleanup()
{
	for(ICompilationExporter* exporter : exporters_)
	{
		delete exporter;
	}

	exporters_ = {};
}

void ICompilationStage::Log(std::string message)
{
	if (parent_ != nullptr) parent_->Log(message); else Compiler::CompilerLog(message);
}

void ICompilationStage::OutputData(std::string output_name)
{
	ExportCollector collector(output_name); //TODO add stage name

	for(ICompilationExporter* exporter : exporters_)
	{
		exporter->Export(collector);
	}

	collector.Export();
}

void ICompilationStage::AddExporter(ICompilationExporter* exporter)
{
	exporters_.push_back(exporter);
}

std::map<std::string, std::function<ICompilationStageRegistry*()>> compilation_stage_registrations;

_CompilationStageRegistration::_CompilationStageRegistration(std::string type, std::function<ICompilationStageRegistry*()> creator)
{
	compilation_stage_registrations.insert(std::pair<std::string, std::function<ICompilationStageRegistry*()>>(type, creator));
}

ICompilationStageRegistry* FindStageRegistry(std::string name)
{
	auto stage_it = compilation_stage_registrations.find(name);
	if(stage_it == compilation_stage_registrations.end()) { Compiler::CompilerLog("Could not find compilation stage type for: " + name); return nullptr; }
	return stage_it->second();
}

std::vector<std::string> ICompilationStageRegistry::GetAvailableStages(std::string type)
{
	
	ICompilationStageRegistry* reg = FindStageRegistry(type);
	if(reg == nullptr) return std::vector<std::string>();
	std::vector<std::string>& result = reg->GetAvailableStages_();
	delete reg;
	return result;
}

ICompilationStage* ICompilationStageRegistry::CreateStage(CompilationStageInfo& info)
{
	ICompilationStageRegistry* reg = FindStageRegistry(info.Type);
	if(reg == nullptr) return nullptr;
	ICompilationStage* result = reg->CreateStage_(info.Name);
	delete reg;
	return result;
}
