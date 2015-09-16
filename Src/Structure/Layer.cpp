#include "stdafx.h"
#include "layer.h"
#include "Image.h"


std::minstd_rand0 Layer::rand_engine(1);


Gdiplus::Color HsvToRgbColor(double alpha,double hue, double saturation, double value)
{
	assert(hue >= 0);
	assert(hue <= 1);
	assert(saturation >= 0);
	assert(saturation <= 1);
	assert(value >= 0);
	assert(value <= 1);

	double chroma = value * saturation;
	double h = 6 * hue;
	double x = chroma * (1 - abs(h - floor(h/2) - 1));
	double r1 = 0;
	double g1 = 0;
	double b1 = 0;
	switch ((int)floor(h))
	{
		case 0: r1 = chroma; g1 = x; break;
		case 1: r1 = x; g1 = chroma; break;
		case 2: g1 = chroma; b1 = x; break;
		case 3: g1 = x; b1 = chroma; break;
		case 4: r1 = chroma; b1 = x; break;
		case 5: r1 = x; b1 = chroma; break;
		default: break;
	}
	double m = value - chroma;
	double r = r1 + m;
	double g = g1 + m;
	double b = b1 + m;
	return Gdiplus::Color( 255*alpha,(int)(255 * r), (int)(255 * g), (int)(255 * b));
} 

void Layer::Draw(const Rect& nodeExtent, const Point& imageSize, const wchar_t* fileName,bool smooth)
{
	Image image(nodeExtent, imageSize, edges().size(), smooth);
	

	double sum = 0;
	double sumSqr = 0;
	int count = edges().size();
	for (auto e : edges())
	{
		auto len = e->node1()->Pos().Distance(e->node2()->Pos());
		sum += len;
		sumSqr += len*len;
	}
	auto dev = sqrt(sumSqr - sum*sum/count)/count;
	auto avg = sum / count;


	auto lineWidth = imageSize.X() / sum * 10;
	image.SetPenWidth(lineWidth);

	bool useVimage = true;
	if (useVimage)
	{
		bool oversample = false;

		MemoryImage* vimg;
		vimg = new MemoryImage(imageSize.X(), imageSize.Y(), oversample? 4 : 1);

		auto imageExtent = Rect(imageSize.X() / 10, imageSize - imageSize.X() / 10);

		auto translatePoint = [&](const Point& point)
		{
			Point result;
			result.X() = (point.X() - nodeExtent.min.X()) / nodeExtent.width() * imageExtent.width() + imageExtent.min.X();
			result.Y() = (point.Y() - nodeExtent.min.Y()) / nodeExtent.height() * imageExtent.height() + imageExtent.min.Y();
			return result;
		};

		for (auto e : edges())
		{
			auto p1 = translatePoint(e->node1()->Pos());
			auto p2 = translatePoint(e->node2()->Pos());
			vimg->DrawLine(p1, p2, lineWidth, oversample? MemoryImage::Maximum : MemoryImage::Additive);
		}

		if (true)
		{

			double sum = 0;
			double max = 1;
			for (int x = 0; x < imageSize.X(); x++)
				for (int y = 0; y < imageSize.Y(); y++)
				{
					auto v = vimg->Pixel(x, y);
					assert(v >= 0);
					sum += v;
					max = std::max(max, v);
				}

			const int bandCount = 100;
			int histogram[bandCount];
			for (int i = 0; i < bandCount; i++)
				histogram[i] = 0;



			int sampleCount = 0;
			for (int x = 0; x < imageSize.X(); x++)
				for (int y = 0; y < imageSize.Y(); y++)
				{

					auto v = vimg->Pixel(x, y);
					auto s = (int)((bandCount - 1) * v / max);

					assert(s >= 0);
					assert(s < bandCount);

					if (v > 0)
					{
						sampleCount++;
						histogram[s]++;
					}
				}

			double bands[bandCount];
			int bandSum = 0;
			for (int i = 0; i < bandCount; i++)
			{
				bands[i] = (double)bandSum / sampleCount;
				bandSum += histogram[i];
			}


			for (int x = 0; x < imageSize.X(); x++)
				for (int y = 0; y < imageSize.Y(); y++)
				{
					auto& v = vimg->Pixel(x, y);
					auto bandNum = ((bandCount - 1) * v / max);					
					auto bandIndex = (int)floor(bandNum);
					auto bandFrac = bandNum - bandIndex;
					auto color1 = bands[bandIndex];
					auto color2 = bandIndex < bandCount - 1 ? bands[bandIndex+1] : 1;
					auto color = color1*(1 - bandFrac) + color2*bandFrac;					
					
					v = color;
				}
		}

		image.DrawMemoryImage(*vimg);
		delete vimg;
	}
	else
	{

		for (auto n : nodes())
			image.DrawNode(n);
		for (auto e : edges())
		{
			image.DrawEdge(e);
		}
	}
	image.Save(fileName);
}


