#pragma once

#include "Point.h"
class MemoryImage
{
	
	double* m_bits;
	int m_width;
	int m_height;
	int m_oversample = 1;

	
public:
	int Width(){ return m_width / m_oversample; }
	int Height(){ return m_height/m_oversample; }

	enum Mode{Maximum,Additive};

	MemoryImage(int width,int height,int oversample=1);
	double& Pixel(int x, int y);
	~MemoryImage();
	
	void DrawLine(double x1, double y1, double x2, double y2, double color, Mode mode=Additive);

	void DrawLine(const Point& p1, const Point& p2, double color,Mode mode = Additive)
	{
		DrawLine((double)p1.X(), (double)p1.Y(), (double)p2.X(), (double)p2.Y(), color,mode);
	}
private:
	void DrawPoint(int x, int y, double value, Mode mode);
};

