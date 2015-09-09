#pragma once
#include "Point.h"
  
class Rect
{	
	int typeId = (int)0xFAFAFAFA3;
public:
	Point min = numeric_limits<float>::max();
	Point max = numeric_limits<float>::lowest();
	Rect() = default;
	Rect(Point min, Point max)
	{
		this->min = min;
		this->max = max;
	}


	Point size()
	{
		return max - min;
	}

	Point center()
	{
		return (max + min)*0.5;
	}

	operator Gdiplus::Rect() const
	{
		return Gdiplus::Rect((int)min.X(), (int)min.Y(), (int)width(), (int)height());
	}

	double width() const
	{
		return max.X() - min.X();
	}

	double height() const
	{
		return max.Y() - min.Y();
	}

	void Expand(const Point& point)
	{
		min = Point::minCoord(min, point);
		max = Point::maxCoord(max, point);
	}
	
};

