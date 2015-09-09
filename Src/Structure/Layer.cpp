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

	for (auto n : nodes())
		image.DrawNode(n);
	for (auto e : edges())
	{
		auto len = e->node1()->Pos().Distance(e->node2()->Pos());;
		auto lenClass = (len - avg) / dev/2;
		
		auto r = (e->node1()->Pos().val[2] + e->node2()->Pos().val[2])*0.5;
		auto re = (r - nodeExtent.min.val[2]) / (nodeExtent.max.val[2] - nodeExtent.min.val[2]);
		lenClass = re * 2 - 1;
		
		if (lenClass < 0)
		{
			if (lenClass < -1)
				lenClass = -1;
			image.SetPenColor(-lenClass * 150, -lenClass * 50, -lenClass * 50);
		}
		if (lenClass >= 0)
		{
			if (lenClass > 1)
				lenClass = 1;
			image.SetPenColor(lenClass * 50, lenClass * 150, lenClass * 50);
		}			

		
		
		image.DrawEdge(e);
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