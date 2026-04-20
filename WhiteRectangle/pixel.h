#pragma once

struct pixel
{
	unsigned char r, g, b;
};

bool operator == (const pixel& p1, const pixel& p2)
{
	return p1.r == p2.r && p1.g == p2.g && p1.b == p2.b;
}

bool operator != (const pixel& p1, const pixel& p2)
{
	return p1.r != p2.r || p1.g != p2.g || p1.b != p2.b;
}

namespace pixel_const
{
	const pixel red{ 255, 0, 0 }, green{ 0, 255, 0 }, blue{ 0, 0, 255 },
				black{ 0, 0, 0 }, white{ 255, 255, 255 };
}