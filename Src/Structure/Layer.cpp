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


	auto lineWidth = imageSize.X() / sum * 5;
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

		for (int x = 0; x < imageSize.X(); x++)
			for (int y = 0; y < imageSize.Y(); y++)
			{
			//auto v = log( vimg->Pixel(x, y)+1)/log(20);
			auto v = 1.0 -1.0/(1.0+vimg->Pixel(x, y) ) ;
			vimg->Pixel(x, y) = v;
			}

		/*double sum = 0;
		double max = 0;
		for (int x = 0; x < imageSize.X(); x++)
			for (int y = 0; y < imageSize.Y(); y++)
			{
				auto v = vimg->Pixel(x, y);
				sum += v;
				max = std::max(sum, v);
			}

		int histogram[100];
		for (int x = 0; x < imageSize.X(); x++)
			for (int y = 0; y < imageSize.Y(); y++)
			{
				auto v = vimg->Pixel(x, y);
				auto s = (int)(100 * v / max);
				histogram[s]++;
			}

		double bands[10];
		int bandSum = 0;
		int bandNum = 0;
		int pixelCount = imageSize.X() * imageSize.Y();
		for (int i = 0; i < 100; i++)
		{
			bandSum += histogram[i];
			if (bandSum >= pixelCount / 10)
			{
				bandSum -= pixelCount / 10;
			}
			bands[bandNum]
		}*/

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

void Layer::CalcAproxNodeProperties()
{
	concurrency::parallel_for_each(nodes().begin(), nodes().end(), [](NodePtr node)
	{
		assert(node->lowerLevel().size() <= 2);
		auto lower1 = node->lowerLevel()[0];
		if (node->lowerLevel().size() == 1)
		{
			node->graphDiameter = lower1->graphDiameter;
			node->averageDistance = lower1->averageDistance;
			node->weight() = lower1->weight();
		}
		else
		{
			auto lower2 = node->lowerLevel()[1];
			node->weight() = lower1->weight() + lower2->weight();
			node->graphDiameter = lower1->graphDiameter + 1 + lower2->graphDiameter;
			auto lower1combinations = 0.5*(lower1->weight()*(lower1->weight() - 1));
			auto lower2combinations = 0.5*(lower2->weight()*(lower2->weight() - 1));
			auto lower12combinatinos = lower1->weight()*lower2->weight();
			
			auto averageDistance =
				(lower1combinations * lower1->averageDistance
				+ lower2combinations * lower2->averageDistance
				+ lower12combinatinos
				*(
					lower1->averageDistance*(lower1->weight() - 1) / lower1->weight()
					+ 1
					+ lower2->averageDistance*(lower2->weight() - 1) / (lower2->weight())
				)
				) / (lower1combinations + lower2combinations + lower12combinatinos);
			node->averageDistance = averageDistance;
		}
	});
}