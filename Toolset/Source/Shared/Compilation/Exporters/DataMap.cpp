#include "DataMap.h"
#include "../../IO/BitmapRenderer.h"
#include "../../Platform/Platform.h"
#include "../../Compilation/Compiler.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../Dependencies/stb/stb_image.h"

void DataMapChannel::Init(DataMap* parent, std::string name)
{
	name_ = name;
	parent_ = parent;
}

std::string DataMapChannel::GetName()
{
	return name_;
}

DataMap* DataMap::Create(ICompilationStage* stage, std::string name, Int2 size, Float2 view_size, Float2 view_offset)
{
	DataMap* map = new DataMap();
	stage->AddExporter(map);

	map->name_ = name;
	map->image_size_ = size;
	map->view_size_ = view_size;
	map->view_offset_ = view_offset;
	map->background_ = "";

	return map;
}

void DataMap::Export(ExportCollector& collector)
{
	std::string unique_id = collector.GenerateId();

	collector.AddContent("<h1>" + name_ + "</h1>");
	collector.AddContent("<div class='map_parent' id='map_parent_" + unique_id + "'>");
	collector.AddContent("<button class = 'btn_zoomin'> </button><button class='btn_zoomout'> </button>");
	collector.AddContent("<div class='map_holder'>");
	collector.AddContent("<div class='map_layer'>");

	collector.AddStyle("#map_parent_" + unique_id + " > .map_holder > .map_layer > img {max-width: " + std::to_string(ExportSize.x) + "px; max-height: " + std::to_string(ExportSize.y) + "px; }");
	collector.AddStyle("#map_parent_" + unique_id + " > .map_holder > .map_layer {width: " + std::to_string(ExportSize.x) + "px; height: " + std::to_string(ExportSize.y) + "px; }");

	BitmapRenderer bg(image_size_);
	bg.Clear(Color(.9f, .9f, .9f));

	if(!background_.empty())
	{
		std::string path = GetPreferencesPath() + "\\CompilerScripts\\" + background_ + ".png";

		FILE* f;
		fopen_s(&f, path.c_str(), "rb");
		if (f != nullptr)
		{
			fseek(f, 0, SEEK_END);
			int length = ftell(f);
			fseek(f, 0, SEEK_SET);

			unsigned char* raw_image = (unsigned char*)malloc(length * sizeof(char));
			fread(raw_image, sizeof(char), length, f);
			fclose(f);

			int image_w, image_h, image_channels;

			unsigned char* image_data = stbi_load_from_memory(raw_image, length, &image_w, &image_h, &image_channels, 3);
			free(raw_image);

			unsigned char* full_image;
			if(image_channels==4)
			{
				full_image = image_data;
			}
			else 
			{
				full_image = (unsigned char*)malloc(image_w * image_h * 4 * sizeof(char));

				for (unsigned int i = 0; i < image_w * image_h; i++)
				{
					unsigned int base_index = i * image_channels;
					unsigned int target_index = i * 4;

					if (image_channels == 1) 
					{
						full_image[target_index] = image_data[base_index];
						full_image[target_index + 1] = image_data[base_index];
						full_image[target_index + 2] = image_data[base_index];
						full_image[target_index + 3] = 255;
					}

					else if (image_channels == 2)
					{
						full_image[target_index] = image_data[base_index];
						full_image[target_index + 1] = image_data[base_index];
						full_image[target_index + 2] = image_data[base_index];
						full_image[target_index + 3] = image_data[base_index + 1];
					}

					else if (image_channels == 3)
					{
						full_image[target_index] = image_data[base_index];
						full_image[target_index + 1] = image_data[base_index + 1];
						full_image[target_index + 2] = image_data[base_index + 2];
						full_image[target_index + 3] = 255;
					}
					else
					{
						full_image[target_index] = 0;
						full_image[target_index + 1] = 0;
						full_image[target_index + 2] = 0;
						full_image[target_index + 3] = 255;
					}
				}
			}

			bg.DrawImage({ 0, 0 }, image_size_, full_image, { image_w , image_h });

			if (image_channels != 4)
			{
				free(full_image);
			}

			stbi_image_free(image_data);
		}
	}

	bg.Export(collector.GetDataDirectory() + "\\" + name_ + ".png");

	collector.AddContent("<img src='" + collector.GetRelativeDataDirectory() + "\\" + name_ + ".png" + "' draggable='false'>");

	for (DataMapChannel* layer : layers_)
	{
		layer->Export(collector.GetDataDirectory() + "\\" + name_ + "_" + layer->GetName() + ".png");
		collector.AddContent("<img src='" + collector.GetRelativeDataDirectory() + "\\" + name_ + "_" + layer->GetName() + ".png' class='map_image' draggable='false'>");
	}

	collector.AddContent("</div></div>");

	collector.AddScript("var map_root" + unique_id + " = document.getElementById('map_parent_" + unique_id + "'); var map_list" + unique_id + " = map_root" + unique_id + ".getElementsByClassName('map_layer')[0]; var map" + unique_id + "_zoomlevel = 0; var map" + unique_id + "_center = [.5, .5]; var map" + unique_id + "_panning = false; var map" + unique_id + "_lastmouseposition = [0, 0]; map" + unique_id + "_zoom(); map_list" + unique_id + ".addEventListener('mousedown', (event) => { map" + unique_id + "_panning = true; map" + unique_id + "_lastmouseposition = [event.clientX, event.clientY]; }); map_list" + unique_id + ".addEventListener('mouseup', (event) => { map" + unique_id + "_panning = false; }); map_list" + unique_id + ".addEventListener('mousemove', (event) => { if(map" + unique_id + "_panning) { var delta_pos = [event.clientX - map" + unique_id + "_lastmouseposition[0], event.clientY - map" + unique_id + "_lastmouseposition[1]]; map" + unique_id + "_lastmouseposition = [event.clientX, event.clientY]; delta_pos[0] /= map_list" + unique_id + ".clientWidth; delta_pos[1] /= map_list" + unique_id + ".clientHeight;  var scale = 1; for(var i=0;i < map" + unique_id + "_zoomlevel;i+=1){ scale += scale; }  delta_pos[0] /= scale; delta_pos[1] /= scale;  map" + unique_id + "_center[0]-=delta_pos[0]; map" + unique_id + "_center[1]-=delta_pos[1];  map" + unique_id + "_zoom(); } }); function map" + unique_id + "_zoom(){ map" + unique_id + "_center[0] = Math.max(0, Math.min(1, map" + unique_id + "_center[0])); map" + unique_id + "_center[1] = Math.max(0, Math.min(1, map" + unique_id + "_center[1])); var scale = 1; for(var i=0;i < map" + unique_id + "_zoomlevel;i+=1){ scale += scale; } map_list" + unique_id + ".style.transform = 'scale(' + scale + ')';  var x, y; x = map" + unique_id + "_center[0] - 0.5; y = map" + unique_id + "_center[1] - 0.5; x *= -map_list" + unique_id + ".clientWidth; y *= -map_list" + unique_id + ".clientHeight;  map_list" + unique_id + ".style.transform += 'translate(' + x + 'px, ' + y + 'px)'; }  map_root" + unique_id + ".getElementsByClassName('btn_zoomin')[0].onclick = function(){ map" + unique_id + "_zoomlevel += 1; map" + unique_id + "_zoom(); }; map_root" + unique_id + ".getElementsByClassName('btn_zoomout')[0].onclick = function(){ map" + unique_id + "_zoomlevel = Math.max(0, map" + unique_id + "_zoomlevel-1); map" + unique_id + "_zoom(); };");

	collector.AddContent("<div class='layer_list'>");

	int layer_index = 0;
	for (DataMapChannel* layer : layers_)
	{
		collector.AddContent("<label class='layer_switch'><input type='checkbox' checked><span class='slider'></span></label><span>" + layer->GetName() + "</span><br><br>");
		collector.AddScript("map_root" + unique_id + ".getElementsByClassName('layer_list')[0].getElementsByClassName('layer_switch')[" + std::to_string(layer_index) + "].getElementsByTagName('input')[0].addEventListener('change', (event) => {map_list" + unique_id + ".getElementsByClassName('map_image')[" + std::to_string(layer_index) + "].style.visibility = event.target.checked ? 'visible' : 'hidden';});");
		layer_index++;
	}
	
	collector.AddContent("</div></div><br style='clear:both'/><hr>");

	for (DataMapChannel* layer : layers_)
	{
		delete layer;
	}
	layers_.clear();
}

void DataMap::SetBackground(std::string name)
{
	background_ = name;
}

DataMap& DataMapChannel::Parent()
{
	return *parent_;
}

void DensityChannel::AddDensity(Float2 pos, float value)
{
	pos /= cell_size_;
	pos = { std::floor(pos.x), std::floor(pos.y) };

	if (data_.find(pos.To<int>()) != data_.end())
	{
		data_[pos.To<int>()] += value;
	}
	else
	{
		data_.insert(std::pair<Int2, float>(pos.To<int>(), value));
	}
}

void DensityChannel::SetDensity(Float2 pos, float value)
{
	pos /= cell_size_;
	pos = { std::floor(pos.x), std::floor(pos.y) };

	if (data_.find(pos.To<int>()) != data_.end())
	{
		data_[pos.To<int>()] = value;
	}
	else
	{
		data_.insert(std::pair<Int2, float>(pos.To<int>(), value));
	}
}

void DensityChannel::Export(std::string filename)
{
	Float2 cell_count = Parent().view_size_ / cell_size_;
	Int2 cell_pixel_size = (Parent().image_size_.To<float>() / cell_count).To<int>();

	Int2 image_size = cell_count.To<int>() * cell_pixel_size;

	float density_min = 0;
	float density_max = 0;
	float density_average = 0;
	long double density_total = 0;
	bool initiated = false;

	for (auto tile : data_)
	{
		if (!initiated)
		{
			initiated = true;
			density_min = tile.second;
			density_max = tile.second;
			density_total += tile.second;
		}

		density_min = std::min(density_min, tile.second);
		density_max = std::max(density_max, tile.second);
	}

	density_average = density_total / data_.size();

	BitmapRenderer renderer(image_size);
	renderer.Clear(Color(0, 0, 0, 0));

	for (auto tile : data_)
	{
		Float2 pos = ((Int2)tile.first).To<float>() * cell_size_;
		pos += Parent().view_size_ * Float2(.5f, .5f);
		pos -= Parent().view_offset_;

		Int2 cell(std::floor(pos.x / cell_size_.x), std::floor(pos.y / cell_size_.y));

		float progress = 1.0f;
		if (density_min != density_max)
		{
			if(tile.second<density_average)
			{
				progress = .5f - ((tile.second - density_average) / (density_average - density_min)) * .5f;
			}
			else
			{
				progress = .5f + ((tile.second - density_average) / (density_max - density_average)) * .5f;
			}
		}

		Color color;
		if (progress <= .5f)
		{
			color = Color(0, progress * 2, 1.0f - progress * 2.0f);
		}
		else
		{
			color = Color((progress - 0.5f) * 2, (1.0f - (progress - 0.5f) * 2), 0);
		}

		color.a = .7f;
		renderer.DrawRectangle(Int2(cell.x * cell_pixel_size.x, cell.y * cell_pixel_size.y), cell_pixel_size, color);
	}

	renderer.Export(filename);
}
