#include "stdafx.h"
#include "Image.h"

#include "../structure/Node.h"
#include "../structure/Edge.h"





Point DrawPos(const Point& point, const Rect& sourceExtent, const Rect& targetExtent)
{
	Point result;
	result.X() = (point.X() - sourceExtent.min.X()) / sourceExtent.width() * targetExtent.width() + targetExtent.min.X();
	result.Y() = -(point.Y() - sourceExtent.min.Y()) / sourceExtent.height() * targetExtent.height() + targetExtent.max.Y();
	return result;
}

Rect DrawPos(const Rect& rect, const Rect& sourceExtent, const Rect& targetExtent)
{
	Rect result;
	result.min = DrawPos(rect.min, sourceExtent, targetExtent);
	result.max = DrawPos(rect.max, sourceExtent, targetExtent);
	std::swap(result.min.Y(), result.max.Y());
	return result;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

Image::Image(const Rect& graphExtent, const Point& imageSize, int edgeCount,  bool smooth)

{
	m_imageExtent = Rect(imageSize.X() / 10, imageSize - imageSize.X() / 10);
	m_nodeExtent = graphExtent;
	m_edgeCount = edgeCount;
	m_lineWidth = imageSize.X() / sqrt(edgeCount) / 50;

	m_bitmap = new Gdiplus::Bitmap((int)imageSize.X(), (int)imageSize.Y());
	m_bitmap->SetResolution(600, 600);
	m_graphics = new Gdiplus::Graphics (m_bitmap);
	
	m_pen = new Gdiplus::Pen(Gdiplus::Color(0, 0, 0), m_lineWidth);
	SetPenColor(0, 0, 0);
	m_fillBrush = new Gdiplus::SolidBrush(Gdiplus::Color(0, 0, 0));
	m_font = new Gdiplus::Font(L"Arial", 8);
	if (smooth)
	{
		if (smooth)
			m_graphics->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);		
	}

	Gdiplus::SolidBrush whiteBrush(Gdiplus::Color( 255,255,255));
	m_graphics->FillRectangle(&whiteBrush, Gdiplus::Rect(0, 0, imageSize.X(), imageSize.Y()));
	
}

Point PointOnLine(const Point& point, const Point& line1, const Point line2)
{
	auto line = line2 - line1  ;
	auto lineLen = line.Lentgh();
	auto pointToLine = point - line1;
	auto u = (pointToLine.X()*line.X() + pointToLine.Y()*line.Y()) / lineLen/lineLen;
	auto pz = line1 + line*u;
	return pz;
}

Point GetLineIntersection(const Point& p1, const Point& n1, const Point& p2, const Point& n2)
{
	auto det = n2.X()*n1.Y() -n1.X()*n2.Y();
	auto det1 = p1.X()*n1.Y() - p1.Y()*n1.X();
	auto det2 = p2.X()*n2.Y() - p2.Y()*n2.X();
	auto x = (det1*n2.X() - det2*n1.X()) / det;
	auto y = (det1*n2.Y() - det2*n1.Y()) / det;
	return { x, y };
}

void Image::DrawMontionBlurLine(const Point& startPoint1, const Point& endPoint1,
	const Point& startPoint2, const Point& endPoint2)
{
	auto start1 = DrawPos(startPoint1, m_nodeExtent, m_imageExtent);
	auto start2 = DrawPos(startPoint2, m_nodeExtent, m_imageExtent);
	auto end1 = DrawPos(endPoint1, m_nodeExtent, m_imageExtent);
	auto end2 = DrawPos(endPoint2, m_nodeExtent, m_imageExtent);

	auto dir1 = start1.DirectionTo(end1);
	auto dir2 = start2.DirectionTo(end2);
	auto lineDir = dir1 + dir2;
	lineDir *= 1/lineDir.Lentgh();
	auto lineDirNormal = Point(lineDir.Y(), -lineDir.X());

	Gdiplus::Pen dbgpen(Gdiplus::Color(50,255, 0, 0), 1);

	Point startGrad;
	double startWidth;
	if (start1 == start2)
	{
		startGrad = start1;
		startWidth = 0;
	}
	else
	{				
		auto startVec = start2 - start1;
		auto startLen = startVec.DotProduct(lineDir);
		Point startFar;
		Point startFarOposite;
		if (startLen > 0)
		{
			startFar = start1;
			startFarOposite = GetLineIntersection(start1, lineDirNormal, start2, dir2);			
			startWidth = 1;
		}
		else
		{
			startFar = start2;
			startFarOposite = GetLineIntersection(start2, lineDirNormal, start1, dir1);
			startWidth = -1;
		}
		m_graphics->DrawLine(&dbgpen, startFar, startFarOposite);
		startGrad = (startFar + startFarOposite)*.5;
		startWidth*=startFar.Distance(startFarOposite);
	}

	
	Point endGrad;
	double endWidth;
	if (end1 == end2)
	{
		endGrad = end1;
		endWidth = 0;
	}
	else
	{
		auto endVec = end2 - end1;
		auto endLen = endVec.DotProduct(lineDir);
		Point endFar;
		Point endFarOposite;
		if (endLen < 0)
		{
			endFar = end1;
			endFarOposite = GetLineIntersection(end1, lineDirNormal, end2, dir2);
			endWidth = -1;
		}
		else
		{
			endFar = end2;
			endFarOposite = GetLineIntersection(end2, lineDirNormal, end1, dir1);
			endWidth = 1;
		}
		m_graphics->DrawLine(&dbgpen, endFar, endFarOposite);
		endGrad = (endFar + endFarOposite)*.5;
		endWidth *= endFar.Distance(endFarOposite);
	}

	
	m_graphics->DrawLine(&dbgpen, startGrad, endGrad);

	
	Gdiplus::LinearGradientBrush gradient(startGrad, endGrad, { 255, 0, 0 }, { 0, 255, 0 });

	vector<Gdiplus::Color> gradColors;
	vector<float> gradPositions;
	auto gradPos = 0;
	BYTE lastColor = -1;
	auto gradLen = startGrad.Distance(endGrad);

	
	for (int i = 0; i<= gradLen; i++)
	{
		auto width = startWidth*(gradLen - i) / gradLen + endWidth*(i) / gradLen;
		
		float newColor = (int)(255 / abs(width));
		if (newColor != lastColor)
		{
			gradColors.push_back(Gdiplus::Color(newColor, 0, 0, 0));
			gradPositions.push_back(1.0*i/gradLen);
		}
		lastColor = newColor;
	}
	
	if (gradPositions.back() != 1)
	{
		gradColors.push_back({ lastColor, 0, 0, 0 });
		gradPositions.push_back(1);
	}
	/*gradColors.clear();
	gradPositions.clear();
	gradColors.push_back(Gdiplus::Color(10, 0, 0, 0));
	gradColors.push_back(Gdiplus::Color(100, 0, 0, 0));
	gradPositions.push_back(0);
	gradPositions.push_back(1);*/
	gradient.SetInterpolationColors(gradColors.data(), gradPositions.data(), gradColors.size());
	Gdiplus::Point polyPoints[]{start1, end1, end2, start2};
	m_graphics->FillPolygon(&gradient, polyPoints, 4);


/*
	auto startLen = posStart1.Distance(posStart2);
	auto endLen = posEnd1.Distance(posEnd2);
	auto startOpacity = 255.0 / (1.0+ startLen);
	auto startColor = Gdiplus::Color(startOpacity, 0, 0, 0);
	auto endColor = Gdiplus::Color(255.0 / (1.0+ endLen), 0, 0, 0);
	if (startLen == 0 && endLen == 0)
	{
		Gdiplus::Pen pen(Gdiplus::Color(0, 0, 0));
		m_graphics->DrawLine(&pen, posStart1, posEnd1);
	}
	else 
	{
		Gdiplus::Point points[]{ posStart1, posStart2, posEnd1, posEnd2 };

		if (startLen > 0)
		{
			auto grad1 = PointOnLine(posEnd2, posStart1, posStart2);
			grad1.Y() += 1;
			auto grad2 = posEnd2;
			Gdiplus::LinearGradientBrush b1(grad1, grad2, endColor, startColor);
			m_graphics->FillPolygon(&b1, points, 3);
		}

		if (endLen > 0)
		{
			auto grad1 = PointOnLine(posStart2, posEnd1, posEnd2);			
			auto grad2 = posStart2;
			grad2.Y() += 1;
			Gdiplus::LinearGradientBrush b2(grad2, grad1, startColor, endColor);
			m_graphics->FillPolygon(&b2, points + 1, 3);
		}
	}*/
}

Image::~Image()
{
	delete m_graphics;
	delete m_bitmap;
	delete m_pen;
	delete m_fillBrush;
	delete m_font;
}

void Image::Save(const wchar_t* fileName)
{
	m_graphics->Flush();


	Gdiplus::ColorLUTParams lutParams;
	for (int i = 0; i < 256; i++)
	{
		lutParams.lutA[i] = 255;
		auto g = (BYTE)(pow(i/255.0, 1.0 / 2.2) * 255);
		lutParams.lutR[i] = g;
		lutParams.lutG[i] = g;
		lutParams.lutB[i] = g;
	}
	Gdiplus::ColorLUT lut;
	lut.SetParameters(&lutParams);
	RECT r{ 0, 0, m_bitmap->GetWidth()/2, m_bitmap->GetHeight() };

	

	//m_bitmap->ApplyEffect(&lut, &r);
	
	// Save the altered image.
	CLSID pngClsid;
	GetEncoderClsid(L"image/png", &pngClsid);
	m_bitmap->Save(fileName, &pngClsid);
}

void Image::DrawNode(NodePtr node)
{
	Point pos = DrawPos(node->Pos(), m_nodeExtent, m_imageExtent);
	
	m_graphics->FillEllipse(m_fillBrush, Rect(pos - m_lineWidth * 4, pos + m_lineWidth * 4));
}

void Image::DrawPoint(const Point& p1)
{
	Point pos = DrawPos(p1, m_nodeExtent, m_imageExtent);
	m_graphics->FillEllipse(m_fillBrush, Rect(pos - m_lineWidth * 4, pos + m_lineWidth * 4));

}
void Image::DrawConvexHull(const vector<Point>& points, double margin)
{
	vector<Point> hull;
	Point leftMost = points.front();;
	
	for (auto p : points)
	{
		if (p.X() < leftMost.X())
		{
			leftMost = p;
		}
	}
	hull.push_back(leftMost);
	auto current = leftMost;
	while (true)
	{
		auto next = points.front();
		
		for (auto p : points)
		{
			if (next == current)
			{
				next = p;
				continue;
			}			
			auto side = (next.X() - current.X())*(p.Y() - current.Y()) - (next.Y() - current.Y())*(p.X() - current.X());
			if (side < 0)
				next = p;
			if (side == 0)
			{
				if (p.DistanceSqr(current) > next.DistanceSqr(current))
					next = p;
			}
		}
		if (next == leftMost)
			break;
		else
			hull.push_back(next);
		current = next;
	}

	// translate and expand
	for (auto &p : hull)
		p = DrawPos(p, m_nodeExtent, m_imageExtent);
		
	

	vector<Gdiplus::Point> gdiHull;
	gdiHull.reserve(hull.size());
	if (hull.size() == 1)
	{
		auto center = hull.front();
		
		gdiHull.push_back(center + Point(-margin, -margin));
		gdiHull.push_back(center + Point(-margin, margin));
		gdiHull.push_back(center + Point(margin, margin));
		gdiHull.push_back(center + Point(margin, -margin));
	}	
	else
	{
		for (int i = 0; i < hull.size(); i++)
		{
			auto prev = (i == 0 ? hull.back() : hull[i - 1]);
			auto next = (i == hull.size() - 1 ? hull[0] : hull[i + 1]);
			auto cur = hull[i];


			auto dir1 = (next - cur) / next.Distance(cur);
			auto normal1 = Point(-dir1.Y(), dir1.X());
			auto dir2 = (cur - prev) / cur.Distance(prev);
			auto normal2 = Point(-dir2.Y(), dir2.X());

			if (dir1.X()*dir2.X() + dir1.Y()*dir2.Y() < 0)
			{
				gdiHull.push_back(hull[i] + (normal2 + dir2)*margin);
				gdiHull.push_back(hull[i] + (normal1 - dir1)*margin);
			}
			else
			{
				gdiHull.push_back(hull[i] + (normal1 + normal2)  * margin);
			}
		}
	}

	

	
	m_graphics->FillPolygon(m_fillBrush, gdiHull.data(), gdiHull.size());
	m_graphics->DrawPolygon(m_pen, gdiHull.data(), gdiHull.size());
}

void Image::DrawLine(const Point& p1, const Point &p2)
{
	Point pr1 = DrawPos(p1, m_nodeExtent, m_imageExtent);
	Point pr2 = DrawPos(p2, m_nodeExtent, m_imageExtent);
	auto dir = (pr2 - pr1) / pr1.Distance(pr2) * 4;
	auto normal = Point( -dir.Y(), dir.X() );
	Gdiplus::Point arrowPoints[3];
	arrowPoints[0] = pr2;
	arrowPoints[1] = pr2 - dir - normal;
	arrowPoints[2] = pr2 - dir + normal;
	m_graphics->DrawLine(m_pen, pr1, pr2);
	//m_graphics->FillPolygon(m_fillBrush, arrowPoints, 3);
}


void Image::DrawEdge(Edge* edge)
{
	
	Point p1 = DrawPos(edge->node1()->Pos(), m_nodeExtent, m_imageExtent);
	Point p2 = DrawPos(edge->node2()->Pos(), m_nodeExtent, m_imageExtent);
	m_graphics->DrawLine(m_pen, p1, p2);
}


void Image::DrawString(const Point& p1, const wchar_t* fmt, ...)
{

	
	Gdiplus::SolidBrush blackBrush(Gdiplus::Color(0, 0, 0));
	wchar_t buff[255];
	va_list valist;
	va_start(valist, fmt);
	vswprintf(buff, fmt, valist);
	va_end(valist);
	
	Point nodeCenter = DrawPos(p1, m_nodeExtent, m_imageExtent);
	m_graphics->DrawString(buff, lstrlenW(buff), m_font, Gdiplus::PointF(nodeCenter.X(), nodeCenter.Y()), &blackBrush);
}


void Image::DrawBoundingBox(NodePtr node, int margin,  int baseLevel)
{
	vector<Point> points;
	vector<NodePtr > toVisit;
	toVisit.push_back(node);
	while (!toVisit.empty())
	{
		auto n = toVisit.back();
		toVisit.pop_back();
		if (n->level == baseLevel)
			points.push_back(n->Pos());
		else
		{
			for (auto sn : n->lowerLevel())
				toVisit.push_back(sn);
		}			
	}

	DrawConvexHull(points,margin);
	
}

