#include "ForceLayout.h"


class SpringElectric : public IForceMethod
{
	double K;
	
public:

	wstring Describe() override
	{
		return L"spring electric";
	}

	virtual void InitExecution()
	{
		K = 4;
	}

	void InitLayer()
	{
		


		//K /= layer->nodes().size() *1.0 / layer->upper()->nodes().size();
		/*
		We want to estimate how much average graph distance changed. For this we need edges that were collapsed (not merged).
		Vertex count is a good measure of that
		Missing vertex mean edges on paths were collapsed. */


		for (auto n : layer->nodes())
		{
			n->weight() = 1;
			n->force = 0;
		}

		/*for (auto n : layer->upper()[0]->nodes())
		{
			UnmergeNode(n);
		}*/

		


		//if (topLayer->level - layer->level !=1)
		//	for (auto n: layer->nodes())
		//		n->Pos() *= layer->upper()->nodes().size() / layer->nodes().size() ;
	}

	//void InitLayerPositions()override
	//{
	//	double scaleFactor;
	//	bool scalePos = false;

	//	if (topLayer->level - layer->level == 1)
	//	{
	//		K = 1;
	//		scaleFactor = 1;
	//	}
	//	else
	//		scaleFactor = sqrt(7.0 / 4.0);

	//	if (scalePos)
	//	{
	//		for (auto node : layer->nodes())
	//		{
	//			node->force = 0;
	//			node->Pos() = node->upperLevel()[0]->Pos();
	//			node->Pos() *= scaleFactor;
	//			
	//			node->Pos().X() += rand() % 100 * 0.001;
	//			node->Pos().Y() += rand() % 100 * 0.001;
	//		}
	//	}
	//	else
	//		K = K / scaleFactor;
	//}

	void InitIteration(int number)override
	{
		topLayer->nodes()[0]->CalculatePosFromLoverLevels(topLayer->level - layer->level);
	};

	double CalculateScaling()
	{
		//return sqrt(7.0 / 4.0);
		//K /= layer->nodes().size() *1.0 / layer->upper()[0]->nodes().size()*0.7;
		K = K / sqrt(7.0 / 4.0);
		return 1.0;;
	}

	void UnmergeNode(NodePtr node)
	{
		if (node->lowerLevel().size() == 1)
			node->lowerLevel()[0]->Move(node->Pos());
		else if (node->lowerLevel().size() == 2)
		{
			double C = 1;

			auto n1 = node->lowerLevel()[0];
			auto n2 = node->lowerLevel()[1];
			auto idealDistance = 0.1;
			Point offset;
			auto angle = rand();
			offset.X() = sin(angle)*idealDistance / 2;
			offset.Y() = cos(angle)*idealDistance / 2;
			n1->Move(node->Pos() + offset);
			n2->Move(node->Pos() - offset);
		}
	}

	Point CalcForce(NodePtr node, int threadId){
		Point force(0);
		Point forceErr = 0;
		double C = 1;


		// edge attraction
		for (auto e : node->edges())
		{
			auto other = e->otherNode(node);
			auto dir = other->Pos() - node->Pos();
			auto len = dir.Lentgh();

			if (len == 0)
				continue;
			//dir *= 1 / len;
			auto forceValue = len;
			dir *= forceValue;

			auto dirWithError = dir - forceErr;
			auto newForce = force + dirWithError;
			forceErr = (newForce - force) - dirWithError;
			force = newForce;
		}


		force *= 1 / K;
		// node repulsion	
		auto repForce1 = ApproxRepulsionsNonRecurse(node);

		/*	Point repForce1 = 0;
		for (auto other : layer->nodes())
		{
		if (other == node)
		continue;

		auto dir = other->Pos() - node->Pos(); auto lensqr = dir.LengthSqr();
		if (lensqr == 0)
		continue;

		auto forceValue =C*K*K / lensqr;;
		auto repForce = dir * forceValue;
		repForce1 += repForce;
		}*/


		return force - repForce1;// -forceErr;
	};

	
	Point ApproxRepulsionsNonRecurse(NodePtr source)
	{
		vector<NodePtr > targetsToVisit;
		targetsToVisit.push_back(topLayer->nodes()[0]);
		Point force = 0;
		Point forceErr = 0;
		while (!targetsToVisit.empty())
		{
			NodePtr target = targetsToVisit.back();
			targetsToVisit.pop_back();

			if (target == source)
				continue;
			Point dir = target->Pos();
			dir -= source->Pos();

			auto lensqr = dir.LengthSqr();
			auto  boundDiam = target->boundDiamSqr;

			if (boundDiam < 1.4f*lensqr)
			{
				//approximate																
				//double p = 1.0f;
				//auto forceValue = target->weight()*pow(K,1+p) / pow(lensqr,(p+1)/2);;
				auto forceValue = target->weight() / (lensqr);

				dir *= forceValue;

				auto dirWithError = dir - forceErr;
				auto newForce = force + dirWithError;
				forceErr = (newForce - force) - dirWithError;
				force = newForce;
				force += dir;

			}
			else
			{
				auto & lower = target->lowerLevel();
				int size = (int)lower.size();
				for (int i = 0; i < size; i++)
				{
					auto n = lower[i];
					/*while (n->weight()>1 && n->lowerLevel().size() == 1)
					n = n->lowerLevel()[0];*/
					targetsToVisit.push_back(n);
					//force += ApproxRepulsions(source, n);
				}
			}
		}
		return force*K*K;
	}


	Point ApproxRepulsions(NodePtr source, NodePtr target)
	{
		if (target == source)
			return Point(0);
		double C = 1;
		auto boundCenter = target->Bound().max;
		boundCenter += target->Bound().min;
		boundCenter *= 0.5f;
		auto boundDiam = (target->Bound().min.DistanceSqr(target->Bound().max));
		auto distSqr = (source->Pos().DistanceSqr(boundCenter));



		if (boundDiam< 1.4f*distSqr)
		{

			//approximate
			auto dir = target->Pos() - source->Pos();
			auto lensqr = dir.LengthSqr();
			if (lensqr == 0)
				return Point(0);
			double p = 2;
			//auto forceValue = target->weight()*C*pow(K,1+p) / pow(lensqr,(p+1)/2);;
			auto forceValue = target->weight()*C*K*K / lensqr;
			auto repForce = dir * forceValue;
			return repForce;
		}

		else
		{
			Point force(0);
			auto& lower = target->lowerLevel();
			int size = (int)lower.size();
			for (int i = 0; i < size; i++)
			{
				auto n = lower[i];
				while (n->weight()>1 && n->lowerLevel().size() == 1)
					n = n->lowerLevel()[0];
				force += ApproxRepulsions(source, n);
			}
			return force;
		}
	}

};