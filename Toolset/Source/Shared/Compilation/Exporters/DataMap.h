#pragma once
#include "../CompilationStage.h"
#include "../../Vec2.h"

class DataMap;

class DataMapChannel
{
public:
	void Init(DataMap*, std::string);
	std::string GetName();
	virtual void Export(std::string) = 0;

protected:
	std::string name_;
	DataMap& Parent();
private:
	DataMap* parent_;
};

class DensityChannel : public DataMapChannel
{
public:
	void AddDensity(Float2 pos, float value);
	void SetDensity(Float2 pos, float value);

	void SetCellSize(Float2 size) { cell_size_ = size; }

	void Export(std::string) override;

private:
	Float2 cell_size_ = { 1, 1 };
	std::map<Int2, float> data_;
};

class DataMap : public  ICompilationExporter
{
	friend class DensityChannel;
public:
	static DataMap* Create(ICompilationStage* stage, std::string name, Int2 size, Float2 view_size, Float2 view_offset);
	void Export(ExportCollector&) override;

	void SetResolution(Int2 res) { image_size_ = res; }
	void SetView(Float2 size, Float2 offset) { view_size_ = size; view_offset_ = offset; }

	void SetBackground(std::string name);

	template<class T>
	T* AddChannel(std::string name)
	{
		DataMapChannel* channel = new T();
		channel->Init(this, name);
		layers_.push_back(channel);
		return (T*)channel;
	}

private:
	std::string name_;
	Float2 offset_ = { 0, 0 };
	Int2 ExportSize = { 700, 700 };
	Int2 image_size_;
	Float2 view_size_;
	Float2 view_offset_;

	std::string background_;

	std::vector<DataMapChannel*> layers_;
};