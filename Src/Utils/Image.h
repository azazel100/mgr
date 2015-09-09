#pragma once

#include "Point.h"
#include "Rect.h"
#include "MemoryImage.h"
#include "..\Structure\Node.h"
class Edge;

class Image
{
	Rect m_nodeExtent;
	Rect m_imageExtent;
	int m_edgeCount;
	float m_lineWidth;
	float m_pointSize;
	Gdiplus::Font* m_font;
	Gdiplus::Pen *m_pen;
	Gdiplus::SolidBrush *m_fillBrush;
	
	Gdiplus::Bitmap* m_bitmap;
	Gdiplus::Graphics* m_graphics;
	

	struct Col{
		BYTE r, g, b,a;
	};
public:
	Image(const Rect& graphExtent, const Point& imageExtent, int edgeCount, bool smooth);	
	~Image();

	int Width(){ return m_bitmap->GetWidth(); }
	int Height(){ return m_bitmap->GetHeight(); }

	void DrawMemoryImage( MemoryImage& img)
	{
		Gdiplus::BitmapData bd;
		m_bitmap->LockBits(&Gdiplus::Rect(0, 0, m_bitmap->GetWidth(), m_bitmap->GetHeight()), Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bd);
	
		for (unsigned y = 0; y < img.Height(); y++)
		{
			for (unsigned x = 0; x < img.Width(); x++)
			{
				auto value = max(0.0, min(1.0, img.Pixel(x, y)));
				auto grayValue = 1 - value;
				auto grayValueGammaCorrected = grayValue;// pow(grayValue, 1.0 / 2.2);
				auto src = max(0, min(255, (int)(grayValueGammaCorrected * 255)));
				auto dst = (Col*)bd.Scan0 + y*bd.Stride / 4 + x;
				dst->r = (BYTE)src;
				dst->g = (BYTE)src;
				dst->b = (BYTE)src;
				dst->a = 255;
			}
		}
		m_bitmap->UnlockBits(&bd);
	}

	void DrawImage(const Image& img, double opacity=1)
	{
		img.m_graphics->Flush();
		if (opacity == 1)
		{
			m_graphics->DrawImage(img.m_bitmap, 0, 0);
		}
		else
		{
			Gdiplus::ColorMatrix matrix{ {
					{ 1, 0, 0, 0, 0 },
					{ 0, 1, 0, 0, 0 },
					{ 0, 0, 1, 0, 0 },
					{ 0, 0, 0, (float)opacity, 0 },
					{ 0, 0, 0, 0, 1 } } };

			Gdiplus::ImageAttributes imageAttributes;
			imageAttributes.SetColorMatrix(&matrix);
			int w = m_bitmap->GetWidth();
			int h = m_bitmap->GetHeight();
			m_graphics->DrawImage(img.m_bitmap, Gdiplus::Rect{ 0, 0, w, h }, 0, 0, w, h, Gdiplus::UnitPixel, &imageAttributes);
		}
	}

	void Fill(float alpha, float r, float g, float b)
	{
		m_graphics->Clear(Gdiplus::Color((BYTE)alpha, (BYTE)r, (BYTE)g, (BYTE)b));
	}

	Rect& GraphExcent(){ return m_nodeExtent; }

	void SetPenColor(float alpha, float r, float g, float b)
	{
		m_pen->SetColor(Gdiplus::Color((BYTE)alpha, (BYTE)r, (BYTE)g, (BYTE)b));
	}

	void SetCompQ(bool blendGamma)
	{
		m_graphics->SetCompositingQuality(blendGamma ? Gdiplus::CompositingQualityHighQuality : Gdiplus::CompositingQualityAssumeLinear);
	}
	

	void SetPenWidth(float width)
	{
		m_pen->SetWidth(width);
	}

	void SetPenColor(float r, float g, float b)
	{
		m_pen->SetColor(Gdiplus::Color((BYTE)(min(1.0f, m_lineWidth) * 255), (BYTE)r, (BYTE)g, (BYTE)b));
		
	}

	void SetFillColor(float alpha,float r, float g, float b)
	{
		m_fillBrush->SetColor(Gdiplus::Color((BYTE)alpha, (BYTE)r, (BYTE)g, (BYTE)b));
	}

	void DrawMontionBlurLine(const Point& start1, const Point& end1,
		const Point& start2, const Point& end2);

	void DrawNode(NodePtr node);
	void DrawEdge(Edge* edge);
	void DrawBoundingBox(NodePtr node,int margin,int baseLevel);
	void DrawLine(const Point& p1, const Point &p2);
	void DrawString(const Point& p1,const wchar_t* fmt,...);
	void DrawPoint(const Point& p1);
	void DrawConvexHull(const vector<Point>& points,double margin);


	void Save( const wchar_t* fileName);
};