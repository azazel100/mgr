#include "stdafx.h"
#include "MemoryImage.h"


MemoryImage::MemoryImage(int width, int height, int oversample)
{
	m_oversample = oversample;
	m_width = width*oversample;
	m_height = height*oversample;
	m_bits = new double[m_width*m_height];
	ZeroMemory(m_bits, sizeof(double)*m_width*m_height);
}


MemoryImage::~MemoryImage()
{
	delete m_bits;
}




#define ipart_(X) ((int)(X))
#define round_(X) ((int)(((double)(X))+0.5))

#define rfpart_(X) (1.0-fpart_(X))

//double fpart_(double x)
//{
//	if (x < 0)
//		return 1 - (x - floor(x));
//	return x - floor(x);
//}


#define fpart_(X) (((double)(X))-(double)ipart_(X))
#define swap_(a, b) do{ __typeof__(a) tmp;  tmp = a; a = b; b = tmp; }while(0)

struct Col
{
	BYTE r, g, b, a;
};

void MemoryImage::DrawPoint(int x, int y, double value, Mode mode)
{
	if (x < 0 || y < 0 || x >= m_width || y >= m_height)
		return;

	auto p = m_bits + y*m_width + x;
	if (mode == Additive)
		*p += value;
	else
	{
		if (value > *p)
			*p = value;
		
	}
}

double& MemoryImage::Pixel(int x, int y)
{
	x *= m_oversample;
	y *= m_oversample;
	return *(m_bits + y*m_width + x);
	/*if (m_oversample==1)
		return *(m_bits + y*m_width + x);

	double sum = 0;
	for (int i = 0; i < m_oversample; i++)
		for (int j = 0; j < m_oversample; j++)
		{
			sum += *(m_bits + (y+i)*m_width + (x+j));
		}
	return sum;*/
	
}

void MemoryImage::DrawLine(double x1, double y1, double x2, double y2, double color, Mode mode)
{
	x1 *= m_oversample;
	x2 *= m_oversample;
	y1 *= m_oversample;
	y2 *= m_oversample;

	double dx = (double)x2 - (double)x1;
	double dy = (double)y2 - (double)y1;

	if (abs(dx) > abs(dy)) {
		if (x2 < x1) {
			swap(x1, x2);
			swap(y1, y2);
		}
		double gradient = dy / dx;
		double xend = round_(x1);
		double yend = y1 + gradient*(xend - x1);
		double xgap = rfpart_(x1 + 0.5);
		int xpxl1 = xend;
		int ypxl1 = ipart_(yend);
		DrawPoint(xpxl1, ypxl1, color* rfpart_(yend)*xgap,mode);
		DrawPoint(xpxl1, ypxl1 + 1, color*  fpart_(yend)*xgap,mode);
		double intery = yend + gradient;

		xend = round_(x2);
		yend = y2 + gradient*(xend - x2);
		xgap = fpart_(x2 + 0.5);
		int xpxl2 = xend;
		int ypxl2 = ipart_(yend);
		DrawPoint(xpxl2, ypxl2, color* rfpart_(yend) * xgap,mode);
		DrawPoint(xpxl2, ypxl2 + 1, color*  fpart_(yend) * xgap,mode);

		int x;
		for (x = xpxl1 + 1; x <= (xpxl2 - 1); x++) {
			DrawPoint(x, ipart_(intery), color*  rfpart_(intery),mode);
			DrawPoint(x, ipart_(intery) + 1, color* fpart_(intery),mode);
			intery += gradient;
		}
	}
	else {
		if (y2 < y1) {
			swap(x1, x2);
			swap(y1, y2);
		}
		double gradient = dx / dy;
		double yend = round_(y1);
		double xend = x1 + gradient*(yend - y1);
		double ygap = rfpart_(y1 + 0.5);
		int ypxl1 = yend;
		int xpxl1 = ipart_(xend);
		DrawPoint(xpxl1, ypxl1, color*  rfpart_(xend)*ygap, mode);
		DrawPoint(xpxl1, ypxl1 + 1, color*  fpart_(xend)*ygap, mode);
		double interx = xend + gradient;

		yend = round_(y2);
		xend = x2 + gradient*(yend - y2);
		ygap = fpart_(y2 + 0.5);
		int ypxl2 = yend;
		int xpxl2 = ipart_(xend);
		DrawPoint(xpxl2, ypxl2, color* rfpart_(xend) * ygap, mode);
		DrawPoint(xpxl2, ypxl2 + 1, color*  fpart_(xend) * ygap,mode);

		int y;
		for (y = ypxl1 + 1; y <= (ypxl2 - 1); y++) {
			DrawPoint(ipart_(interx), y, color*  rfpart_(interx),mode);
			DrawPoint(ipart_(interx) + 1, y, color* fpart_(interx),mode);
			interx += gradient;
		}
	}
}


#undef swap_
#undef plot_
#undef ipart_
#undef fpart_
#undef round_
#undef rfpart_
