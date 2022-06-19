#pragma once
#include "../Vec2.h"
#include "../Vec4.h"
#include <string>
#include <vector>


class BitmapRenderer
{
public:
	BitmapRenderer(Int2);

	void Clear(Color);
	void DrawPixel(Int2, Color);
	void DrawTriangle(Int2 a, Int2 b, Int2 c, Color);
	void DrawTriangle(Int2 a, Int2 b, Int2 c, Color ca, Color cb, Color cc);
	void DrawLine(Int2 a, Int2 b, Color c);
	void DrawRectangle(Int2 pos, Int2 size, Color);
	void DrawRectangle(Int2 a, Int2 b, Int2 c, Int2 d, Color);
	void DrawRectangle(Int2 a, Int2 b, Int2 c, Int2 d, Color ca, Color cb, Color cc, Color cd);
	void DrawImage(Int2 pos, Int2 size, unsigned char* bitmap, Int2 bitmap_size);
	void Export(std::string);

private:
	unsigned int LocalToIndex(Int2);
	Int2 IndexToLocal(unsigned int);

	//std::vector<Color> BitmapFromTexture(UTexture2D* texture);

	std::vector<Color> data_;
	Int2 size_;
};
