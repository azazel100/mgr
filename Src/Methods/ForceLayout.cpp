#include "stdafx.h"
#include "ForceLayout.h"
#include <list>
#include <time.h>
#include "Stopwatch.h"


#include "SpringMethod.h"
#include "Node.h"

ForceLayout::ForceLayout(void)
{
}


ForceLayout::~ForceLayout(void)
{
}



void ForceLayout::TestSpringApprox()
{
	isDebugRun = true;
	CreateLayers();



	auto model = new SpringMethod();
	model->foreceLayout = this;
	model->topLayer = m_layers.back();
	model->layer = m_layers.front();
	model->useAproxForce = true;
	model->useAproxNodeProperties = true;
	model->useAproxForceRandomUpperLevelsParallel = false;
	model->useAproxForceRandomUpperLevels = false;
	model->useAproxForceSkipManyLevels = true;


	auto extent = model->layer->CalcExtent();
	model->InitExecution();
	model->InitLayer();
	model->InitIteration(0);
	for (auto l : m_layers)
	{
		Image image(extent, Point(1000, 1000), model->layer->edges().size(), true);
		image.SetFillColor(100, 0, 0, 0);
		for (auto n : l->nodes())
		{
			image.DrawBoundingBox(n, 3, 0);
		}
		for (auto e : l->edges())
		{
			image.DrawEdge(e);
		}
		wchar_t buff[255];
		wsprintf(buff, L"layer %d.png", l->level);
		image.Save(buff);
	}



	for (int i = 0; i < 1/*model->layer->nodes().size()*/; i++)
	{
		auto n = model->layer->nodes()[i];
		model->useAproxForce = false;
		auto forceExact = 0;// model->CalcForce(n, 0);
		model->useAproxForce = true;
		auto forceAprox = model->CalcForce(n, 0);
		Image image(extent, Point(800, 800), model->layer->edges().size(), true);

		int margin = 2;

		deque<NodePtr > nodeToDraw;
		nodeToDraw.push_back(model->topLayer->nodes()[0]);
		while (!nodeToDraw.empty())
		{
			auto node = nodeToDraw.front();
			nodeToDraw.pop_front();
			for (auto n : node->lowerLevel())
				nodeToDraw.push_back(n);
			auto status = model->GetNodeStatus(node, 0);
			if (node->upperLevel()[0] != nullptr && model->GetNodeStatus(node->upperLevel()[0], 0) == SpringMethod::NodeStatusType::Visited)
				status = SpringMethod::NodeStatusType::Queued;

			if (status == SpringMethod::NodeStatusType::Visited)
			{

				image.DrawString(node->Pos(), L"%d %0.0f %0.0f", node->index(), node->lastAproxDistance[0], node->averageDistance);
				image.SetPenColor(0, 0, 0);
				image.SetFillColor(50, 0, 0, 255);


				image.DrawBoundingBox(node, node->level*margin + margin, 0);

				image.SetPenColor(255, 0, 0);
				image.SetFillColor(255, 255, 0, 0);
				if (node->lastIncomingEdge[0] != NULL)
				{
					image.DrawLine(node->lastIncomingEdge[0]->node1()->Pos(), node->lastIncomingEdge[0]->node2()->Pos());
				}
				else
				{
					int i = 0;
				}



			}
			else if (status == SpringMethod::NodeStatusType::Subdivided)
			{
				image.SetPenColor(0, 0, 0);
				image.SetFillColor(255, 255, 255, 0);
				//image.DrawBoundingBox(node, node->level*margin + margin, 0);
			}
			else
			{
				image.SetPenColor(0, 0, 0);
				image.SetFillColor(255, 255, 255, 255);
				//image.DrawBoundingBox(node, node->level*margin + margin, 0);

			}

		}


		/*for (auto e : model->layer->edges)
		image.DrawEdge(e);
		*/
		/*image.SetPenColor(0, 244, 0);
		if (forceExact!=0)
		image.DrawLine(n->Pos(), n->Pos() + forceExact);
		image.SetPenColor(255, 0, 0);
		if (forceAprox!=0)
		image.DrawLine(n->Pos(), n->Pos() + forceAprox);
		*/
		image.SetFillColor(255, 255, 0, 0);
		image.DrawBoundingBox(n, 5, 0);
		wchar_t buff[244];
		wsprintf(buff, L"force calc %d.png", n->index());
		image.Save(buff);
		i += model->layer->nodes().size() / 20;

	}
}




//void 
//ForceLayout::Load(Graph& g, GraphAttributes& graphAttributes)
//{
//	m_layers.push_back(new Layer(g, graphAttributes));
//
//}

void
ForceLayout::Load(const wstring filename, const wstring positions)
{
	auto i1 = filename.find_last_of('\\');
	auto i2 = filename.find_last_of('.');
	if (i1 == -1)
		i1 = 0;
	else
		i1 += 1;
	if (i2 == -1)
		i2 = filename.size();
	name = filename.substr(i1, i2 - i1);

	auto layer = new Layer(filename, isDebugRun);
	m_layers.push_back(layer);
	if (!positions.empty())
	{
		m_layers[0]->ReadPositions(positions);
	}

	cout << "Laded graph with " << layer->nodes().size() << " nodes and " << layer->edges().size() <<" edges" << endl;

}


void ForceLayout::DrawMovieFrame( Rect beforeExtent, Rect afterExtent)
{
	if (!drawMovie)
		return;

	totalTime.Stop();
	int framePerIter = 3;

	double maxMoveSqr = 0;
	for (auto node : layer->nodes())
	{
		maxMoveSqr = max(maxMoveSqr, node->speed.LengthSqr());
	}
	auto maxPixelMove = sqrt(maxMoveSqr)*drawMovieSize / afterExtent.width();
	framePerIter = max(1, max((int)(maxPixelMove / 600), (int)(sqrt(maxMoveSqr) / avgLen*15 )));
	double lineColor = drawMovieSize*beforeExtent.width() / avgLen / layer->upper()[0]->edges().size() * 5;
	lineColor = max(2.0, min(255.0, lineColor));
	out << " "<<framePerIter << " movie frames..";
	auto prevFrameExtent = beforeExtent;
	for (int frameIndex = 0; frameIndex < framePerIter; frameIndex++)
	{
		Rect frameExtent = beforeExtent;
		frameExtent.min += (afterExtent.min - beforeExtent.min)*(1.0 + frameIndex) / framePerIter;
		frameExtent.max += (afterExtent.max - beforeExtent.max)*(1.0 + frameIndex) / framePerIter;

		Image image({ { 0, 0 }, { drawMovieSize, drawMovieSize } }, { drawMovieSize, drawMovieSize }, layer->edges().size(), true);

		if (false)
		{
			auto subFrameCount = maxPixelMove / framePerIter / 10;

			for (int i = 0; i < subFrameCount; i++)
			{
				auto subFrameAlpha = 1.0 / (i + 1);
				Rect subFrameExtent = beforeExtent;
				auto iterPos = ((i + 1.0) / subFrameCount + frameIndex) / framePerIter;
				subFrameExtent.min += (afterExtent.min - beforeExtent.min)*iterPos;
				subFrameExtent.max += (afterExtent.max - beforeExtent.max)*iterPos;
				Image subFrameImage(subFrameExtent, { drawMovieSize, drawMovieSize }, layer->edges().size(), true);
				subFrameImage.SetPenColor(lineColor*subFrameAlpha, 0, 0, 0);
				subFrameImage.Fill(255 * subFrameAlpha, 255, 255, 255);
				subFrameImage.SetPenWidth(1);
				for (auto e : layer->edges())
				{
					auto start = e->node1()->Pos() + e->node1()->speed*(iterPos - 1);
					auto end = e->node2()->Pos() + e->node2()->speed*(iterPos - 1);
					subFrameImage.DrawLine(start, end);
				}
				image.DrawImage(subFrameImage);// , 1.0 / (i + 1));
			}
		}
		else
		{
			bool useVImage = true;
			auto translatePoint = [](const Point& point, const Rect& sourceExtent, const Rect& targetExtent)
			{
				Point result;
				result.X() = (point.X() - sourceExtent.min.X()) / sourceExtent.width() * targetExtent.width() + targetExtent.min.X();
				result.Y() = (point.Y() - sourceExtent.min.Y()) / sourceExtent.height() * targetExtent.height() + targetExtent.min.Y();
				return result;
			};
			Rect imgExtent = { { 10, 10 }, { drawMovieSize - 10, drawMovieSize - 10 } };
			MemoryImage* vimg;
			if (useVImage)
				vimg = new MemoryImage(drawMovieSize, drawMovieSize);
			image.Fill(255, 255, 255, 255);
			image.SetPenWidth(1);
			for (auto e : layer->edges())
			{
				auto prevStart = e->node1()->Pos() + e->node1()->speed*(-1.0 + (1.0*frameIndex) / framePerIter);
				auto prevEnd = e->node2()->Pos() + e->node2()->speed*(-1.0 + (1.0*frameIndex) / framePerIter);
				prevStart = translatePoint(prevStart, prevFrameExtent, imgExtent);
				prevEnd = translatePoint(prevEnd, prevFrameExtent, imgExtent);
				auto currStart = e->node1()->Pos() + e->node1()->speed*(-1.0 + (1.0*frameIndex + 1) / framePerIter);
				auto currEnd = e->node2()->Pos() + e->node2()->speed*(-1.0 + (1.0*frameIndex + 1) / framePerIter);
				currStart = translatePoint(currStart, frameExtent, imgExtent);
				currEnd = translatePoint(currEnd, frameExtent, imgExtent);



				auto subFrameCount = (int)max((prevStart - currStart).Lentgh(), (prevEnd - currEnd).Lentgh()) + 1;
				auto motionBlurFactor = 0.5;
				auto color = lineColor / subFrameCount;

				image.SetPenColor(color, 0, 0, 0);
				//image.SetPenWidth(lineWidth);
				for (int i = 0; i < subFrameCount; i++)
				{
					if (useVImage)
					{
						vimg->DrawLine(
						{ prevStart*((1.0*subFrameCount - i) / subFrameCount) + currStart*(1.0*i / subFrameCount) },
						{ prevEnd*((1.0*subFrameCount - i) / subFrameCount) + currEnd*(1.0*i / subFrameCount) }
						, color / 255);
					}
					else
					{
						image.DrawLine(
						{ prevStart*((1.0*subFrameCount - i) / subFrameCount) + currStart*(1.0*i / subFrameCount) },
						{ prevEnd*((1.0*subFrameCount - i) / subFrameCount) + currEnd*(1.0*i / subFrameCount) });
					}
				}


			}
			if (useVImage)
			{
				image.DrawMemoryImage(*vimg);
				delete vimg;
			}
		}

		wchar_t buff[255];
		image.SetPenColor(255, 0, 0, 0);
		wsprintf(buff, L"Level: %d Iteration: %d", layer->level, iteration);
		image.DrawString({ -10, -10 }, buff);
		wsprintf(buff, L"%s\\%s\\Frame %d.png", outputDir.c_str(), outName.c_str(), totalFrameNumber);
		image.Save(buff);

		//layer->Draw(extent, { 800, 800 }, buff, true);
		totalFrameNumber++;
		prevFrameExtent = frameExtent;
		
		
	}
	totalTime.Start();
	out << "ok";
}


