#pragma once
#include "ForceLayout.h"
#include "Image.h"


class SpringTopDown :public IForceMethod
{	
	Image* image;
	double repositionScalingFactor;
public:
	bool drawStages = false;

	virtual void InitExecution()
	{
	}

	void InitLayer()
	{
		auto upperDiam = topLayer->nodes()[0]->graphDiameter;
		InitLayer(topLayer);
		auto thisDiam = topLayer->nodes()[0]->graphDiameter;
		repositionScalingFactor = 1.0*thisDiam / upperDiam;
	}
	
	void InitLayerPositions()override
	{
		int levelFromTop = topLayer->level - layer->level;
		int i = 0;
		if (levelFromTop == 0)
		{
			layer->nodes()[0]->Pos() = 0;
		}
		else if (levelFromTop == 1)
		{
			layer->nodes()[0]->Pos() = Point(0, 0);
			layer->nodes()[1]->Pos() = Point(1, 0);
		}
		else
		{
			double factor = repositionScalingFactor;;

			for (auto node : layer->nodes())
			{				
				node->Pos() = node->upperLevel()[0]->Pos();
				node->Pos() *= factor;
				node->Pos().X() += rand() % 100 * 0.001;
				node->Pos().Y() += rand() % 100 * 0.001;				
				if (isnan(node->Pos().Lentgh()))
					return;
			}
		}
	}
	
	double CalculateScaling()
	{
		return 1;
	}

	void InitIteration(int number) override
	{
		topLayer->nodes()[0]->CalculatePosFromLoverLevels(topLayer->level - layer->level);
		
		for (auto l = topLayer; l!=nullptr&& l->level >= layer->level; l = l->lower())
		{
			for (auto n : l->nodes())
			{
				n->force = 0;
			}
		}

		for (auto l = topLayer; l != layer; l = l->lower())
		{
			CalcLayerForces(l);
		}
	};

	Point CalcForce(NodePtr node, int threadId)
	{
		return node->force;
	}

	wstring Describe() override;
private:
	void InitLayer(Layer* l)
	{
		if (l == layer)
		{
			for (auto node : l->nodes())
			{
				node->weight() = 1;
				node->graphDiameter = 0;
				node->averageDistance = 0;
			}
		}
		else
		{
			InitLayer(l->lower());
			l->CalcAproxNodeProperties();
		}
	}

	void CalcLayerForces(Layer* layer);

	/*
	Calculate and store force acting throgh the connecting edge.
	Recurse to lower levels if aproximation is to big.
	*/
	void CalcForcesBetweenNodes(
		NodePtr n1,
		NodePtr n2,
		NodePtr n1connectedParent,
		NodePtr n2connectedParent);

	
};