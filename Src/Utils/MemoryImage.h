#pragma once

#include "Point.h"
class MemoryImage
{
	double* m_bits;
	int m_width;
	int m_height;

	void DrawPoint(int x, int y, float value);
public:
	int Width(){ return m_width; }
	int Height(){ return m_height; }

	MemoryImage(int width,int height);
	double& Pixel(int x, int y)
	{
		return *(m_bits + y*m_width + x);
	}
	~MemoryImage();
	
	void DrawLine(double x1, double y1, double x2, double y2, double color);

	void DrawLine(const Point& p1, const Point& p2, double color)
	{
		DrawLine((double)p1.X(), (double)p1.Y(), (double)p2.X(), (double)p2.Y(), color);
	}
};

