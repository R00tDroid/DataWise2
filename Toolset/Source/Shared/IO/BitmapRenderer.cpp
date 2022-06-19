#include "BitmapRenderer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../Dependencies/stb/stb_image_write.h"
#include <algorithm>

BitmapRenderer::BitmapRenderer(Int2 size)
{
	size_ = size;

	for (int i = 0; i < size.x * size.y; i++)
	{
		data_.push_back({0, 0, 0, 0});
	}

	Clear(Colors::Black);
}

void BitmapRenderer::Clear(Color color)
{
	unsigned int index;
	for (int y = 0; y < size_.y; y++)
	{
		for (int x = 0; x < size_.x; x++)
		{
			index = LocalToIndex({ x, y });
			data_[index] = color;
		}
	}
}

void BitmapRenderer::DrawPixel(Int2 pos, Color color)
{
	if (color.a == 0) return;
	unsigned int index = LocalToIndex(pos);
	if (index<0 || index >= data_.size() || pos.x < 0 || pos.x >= size_.x || pos.y < 0 || pos.y >= size_.y) return;

	if (color.a == 255)
	{
		data_[index] = color;
	}
	else
	{
		Color bg = data_[index];
		Color fg = color;
		Color result;
		result.r = (fg.r * fg.a) + (bg.r * (1.0f - fg.a));
		result.g = (fg.g * fg.a) + (bg.g * (1.0f - fg.a));
		result.b = (fg.b * fg.a) + (bg.b * (1.0f - fg.a));
		result.a = fg.a;

		data_[index] = result;
	}
}

void BitmapRenderer::DrawTriangle(Int2 a, Int2 b, Int2 c, Color color)
{
	DrawTriangle(a, b, c, color, color, color);
}

void BitmapRenderer::DrawTriangle(Int2 a, Int2 b, Int2 c, Color ca, Color cb, Color cc)
{
	Int2 min = a;
	Int2 max = a;
	Float2 middle = a.To<float>() + b.To<float>() + c.To<float>();
	middle /= 3.0f;

	min.x = std::min(std::min(min.x, b.x), c.x);
	min.y = std::min(std::min(min.y, b.y), c.y);
	max.x = std::max(std::max(max.x, b.x), c.x);
	max.y = std::max(std::max(max.y, b.y), c.y);

	for (int y = min.y; y < max.y; y++)
	{
		for (int x = min.x; x < max.x; x++)
		{
			Float2 p1 = a.To<float>();
			Float2 p2 = b.To<float>();
			Float2 p3 = c.To<float>();

			Float2 f((float)x, (float)y);

			Float2 f1 = p1 - f;
			Float2 f2 = p2 - f;
			Float2 f3 = p3 - f;

			float alpha = Float2::cross(p1 - p2, p1 - p3);
			float alpha1 = Float2::cross(f2, f3) / alpha;
			float alpha2 = Float2::cross(f3, f1) / alpha;
			float alpha3 = Float2::cross(f1, f2) / alpha;

			if (alpha1 >= 0 && alpha2 >= 0 && alpha3 >= 0) {
				Color color = ca * alpha1 + cb * alpha2 + cc * alpha3;
				DrawPixel({ x, y }, color);
			}
		}
	}
}

void BitmapRenderer::DrawLine(Int2 a, Int2 b, Color c)
{
	bool steep = (std::abs(b.y - a.y) > std::abs(b.x - a.x));
	if (steep)
	{
		int tmp = a.x;
		a.x = a.y;
		a.y = tmp;

		tmp = b.x;
		b.x = b.y;
		b.y = tmp;
	}

	if (a.x > b.x)
	{
		int tmp = a.x;
		a.x = b.x;
		b.x = tmp;

		tmp = a.y;
		a.y = b.y;
		b.y = tmp;
	}

	float x_delta = b.x - a.x;
	float y_delta = std::abs(b.y - a.y);

	float error = x_delta / 2.0f;
	int y_step = (a.y < b.y) ? 1 : -1;

	int y = a.y;

	for (int x = a.x; x < b.x; x++)
	{
		if (steep)
		{
			DrawPixel({ y, x }, c);
		}
		else
		{
			DrawPixel({ x, y }, c);
		}

		error -= y_delta;
		if (error < 0)
		{
			error += x_delta;
			y += y_step;
		}
	}
}

void BitmapRenderer::DrawRectangle(Int2 pos, Int2 size, Color color)
{
	for (int y = pos.y; y < pos.y + size.y; y++)
	{
		for (int x = pos.x; x < pos.x + size.x; x++) 
		{
			DrawPixel({ x, y }, color);
		}
	}
}

void BitmapRenderer::DrawRectangle(Int2 a, Int2 b, Int2 c, Int2 d, Color color)
{
	DrawRectangle(a, b, c, d, color, color, color, color);
}

void BitmapRenderer::DrawRectangle(Int2 a, Int2 b, Int2 c, Int2 d, Color ca, Color cb, Color cc, Color cd)
{
	DrawTriangle(a, b, c, ca, cb, cc);
	DrawTriangle(c, d, a, cc, cd, ca);
}

void BitmapRenderer::DrawImage(Int2 pos, Int2 size, unsigned char* bitmap, Int2 bitmap_size)
{
	for (int y = 0; y < size.y; y++)
	{
		for (int x = 0; x < size.x; x++)
		{
			Float2 uv = { x / (float)size.x , y / (float)size.y };
			Float2 bitmap_point = uv * Float2(bitmap_size.x, bitmap_size.y);
			unsigned int index = (std::floor(bitmap_point.x) + std::floor(bitmap_point.y) * bitmap_size.x) * 4;
			DrawPixel(pos + Int2(x, y), Color(bitmap[index] / 255.0f, bitmap[index + 1] / 255.0f, bitmap[index + 2] / 255.0f, bitmap[index + 3] / 255.0f));
		}
	}
}

void BitmapRenderer::Export(std::string file)
{
	std::vector<unsigned char> bitmap;
	for (int i = 0; i < data_.size(); i++)
	{
		bitmap.push_back(data_[i].r * 255);
		bitmap.push_back(data_[i].g * 255);
		bitmap.push_back(data_[i].b * 255);
		bitmap.push_back(data_[i].a * 255);
	}

	stbi_write_png(file.c_str(), size_.x, size_.y, 4, bitmap.data(), size_.x * 4 * sizeof(unsigned char));
}

unsigned BitmapRenderer::LocalToIndex(Int2 pos)
{
	return pos.x + pos.y * size_.x;
}

Int2 BitmapRenderer::IndexToLocal(unsigned int index)
{
	Int2 pos;
	pos.y = std::floor(index / size_.x);
	pos.x = index - pos.y * size_.x;
	return pos;
}
