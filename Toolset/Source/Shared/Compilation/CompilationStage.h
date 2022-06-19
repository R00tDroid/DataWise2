#pragma once
#include "../IO/Session.h"
#include "ExportCollector.h"
#include <functional>

class Compiler;

class ICompilationExporter
{
public:
	virtual void Export(ExportCollector&) = 0;
};

struct CompilationStageInfo
{
	std::string Type;
	std::string Name;
};

class ICompilationStage
{
public:
	void Assign(Compiler*);

	virtual void Init() = 0;

	virtual void Prepare() = 0;
	virtual void ProcessSession(Session&) = 0;
	virtual void Export() = 0;

	virtual void Cleanup();

	void Log(std::string);

	void OutputData(std::string);

	void AddExporter(ICompilationExporter*);

private:
	std::vector<ICompilationExporter*> exporters_;
	Compiler* parent_;
};

class ICompilationStageRegistry{
public:
	static std::vector<std::string> GetAvailableStages(std::string type);
	static ICompilationStage* CreateStage(CompilationStageInfo&);

protected:
	virtual std::vector<std::string> GetAvailableStages_() = 0;
	virtual ICompilationStage* CreateStage_(std::string) = 0;
};

#define REGISTER_COMPILATIONSTAGE_TYPE(TYPE, NAME)  ICompilationStageRegistry* TYPE##_Creator(){ return new TYPE(); }; _CompilationStageRegistration TYPE##_implreg(NAME, TYPE##_Creator);

struct _CompilationStageRegistration
{
	_CompilationStageRegistration(std::string, std::function<ICompilationStageRegistry*()>);
};