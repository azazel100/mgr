#include "stdafx.h"
#include "SpringTopDown.h"







wstring SpringTopDown::Describe() 
{
	return L"spring topdown";
}

/*
Propagate forces from this layer to lower one.
Also calculate forces on edges that collaped to nodes() in this layer.
*/
void SpringTopDown::CalcLayerForces(Layer* layer)
{
	if (drawStages)
	{
		image = new Image(this->layer->CalcExtent(), Point(800, 800), this->layer->edges().size(), true);
	}

	//TODO: make parallel
	for (auto parent : layer->nodes())
	{
		if (parent->lowerLevel().size() == 1)
		{
			auto n = parent->lowerLevel()[0];
			n->force += parent->force;
		}
		else
		{
			assert(parent->lowerLevel().size() == 2);
			auto n1 = parent->lowerLevel()[0];
			auto n2 = parent->lowerLevel()[1];
			n1->force += parent->force;
			n2->force += parent->force;
			image->SetFillColor(20, 0, 0, 0);
			image->DrawBoundingBox(n1, 10, 0);
			image->DrawBoundingBox(n2, 10, 0);
			//CalcForcesBetweenNodes(n1, n2, parent->lowerEdge());
		}
	}

	if (drawStages)
	{
		image->SetPenColor(0, 0, 255);
		for (auto e : this->layer->edges())
			image->DrawEdge(e);
		wstringstream s;
		s << L"spring topdown layer" << layer->level <<L".png";
		image->Save(s.str().c_str());
	}
}


/*
Calculate and store force acting throgh the connecting edge.
Recurse to lower levels if aproximation is to big.

connectingEdge also represents the closest level on which parents 
of n1 and n2 were connected.

n1 may be one level lower that n2
//*/
//void SpringTopDown::CalcForcesBetweenNodes(
//	NodePtr n1,
//	NodePtr n2,
//	NodePtr n1connectedParent,
//	NodePtr n2connectedParent,
//	Edge* connectingEdge
//	)
//{
//	assert(n1->level == n2->level || n1->level == n2->level - 1);
//	float distSqr = n1->Pos().DistanceSqr(n2->Pos());
//	float graphDist = n1connectedParent->averageDistance*(n1connectedParent->weight() - 1) / n1connectedParent->weight()
//		+ 1
//		+ n2connectedParent->averageDistance*(n2connectedParent->weight() - 1) / n2connectedParent->weight();
//
//	float distRatio = (n1->boundDiamSqr + n2->boundDiamSqr) / distSqr;
//	float graphDistRatio = (n1->graphDiameter + n2->graphDiameter) / graphDist;
//
//	if (distRatio < 1 && graphDistRatio < 1)
//	{
//		auto dir = (n2->Pos() - n1->Pos());
//		auto dirLen = dir.Lentgh();
//		auto forceValue = (1.0 - graphDist / dirLen) / graphDist / graphDist;
//		n1->force += dir * forceValue * n2->weight();
//		n2->force -= dir * forceValue * n1->weight();
//		if (image != NULL)
//		{
//			image->SetFillColor(100, 255, 0, 0);
//			image->DrawBoundingBox(n1, 2, layer->level);
//			image->SetFillColor(100, 0, 255, 0);
//			image->DrawBoundingBox(n2, 2, layer->level);
//			image->DrawLine(n1->Pos(), n2->Pos());
//		}
//	}
//	else
//	{
//		while (n1->lowerLevel().size() == 1 && n2->lowerLevel().size() == 1)
//		{			
//			if (connectingEdge != nullptr)
//			{
//				assert(connectingEdge->lowerLevel().size() == 1);
//				connectingEdge = connectingEdge->lowerLevel[0];
//			}			
//			n1 = n1->lowerLevel()[0];
//			n2 = n2->lowerLevel()[0];
//		}
//		// say we split n1
//		assert(n1->lowerLevel().size() == 2);
//		for (auto n1lower : n1->lowerLevel())
//		{
//			for (auto connectingLower : connectingEdge->lowerLevel)
//			{
//				if (connectingLower->node1() == n1lower || connectingLower->node2() == n1lower)
//				{
//					n1connectedParent = n1lower;
//					connectingEdge = connectingLower;
//					break;
//				}
//
//			}
//		}
//
//
//		//if (n1->level == n2->level-1)
//		//{
//		//	assert(n2->lowerLevel().size() == 2);
//		//	for (int i = 0; i < 2; i++)
//		//	{
//		//		auto node2lower = n2->lowerLevel()[i];
//		//		auto connectingToLower = connectingEdge;
//		//		// we can move connecting edge to lower level
//		//		// if n1 and n2lower are connected on connectingEdge lower level
//		//		if (connectingEdge->node1()->level == n2->level)
//		//			for (auto lower : connectingEdge->lowerLevel())
//		//			{
//		//			if (lower->node1() == node2lower && lower->node2() == n1
//		//				|| lower->node2() == node2lower && lower->node1() == n1)
//		//			{
//		//				connectingToLower = lower;
//		//				break;
//		//			}
//		//			}
//		//		CalcForcesBetweenNodes(n1, node2lower, connectingToLower);
//		//	}
//		//}
//		//else
//		{			
//			
//			//while (nodeToKeep->lowerLevel().size() == 1 && nodeToSplit->lowerLevel().size() == 1)
//			//{
//			//	/*if (n1->conn)
//			//	assert(n1connectingEdge->lowerLevel().size() == 1);
//			//	assert(n2connectingEdge->lowerLevel().size() == 1);
//			//	n1connectingEdge = n1connectingEdge->lowerLevel()[0];
//			//	n1connectingEdge = n1connectingEdge->lowerLevel()[0];
//			//	connectingEdge = connectingEdge->lowerLevel()[0];
//			//	nodeToKeep = nodeToKeep->lowerLevel()[0];
//			//	nodeToSplit = nodeToSplit->lowerLevel()[0];*/
//			//}
//			if (n2->lowerLevel().size() == 1)
//			{
//				;//ok
//			}
//			else if (n1->lowerLevel().size() == 1)
//			{
//				swap(n1, n2);
//				swap(n1connectedParent, n2connectedParent);
//				swap(n1connectingEdge, n2connectingEdge);
//			}
//			else if (graphDistRatio > distRatio)
//			{
//				if (n1->boundDiamSqr > n2->boundDiamSqr)
//				{
//					swap(n1, n2);
//					swap(n1connectedParent, n2connectedParent);
//					swap(n1connectingEdge, n2connectingEdge);
//				}
//			}
//			else
//			{
//				if (n1->graphDiameter > n2->graphDiameter)
//				{
//					swap(n1, n2);
//					swap(n1connectedParent, n2connectedParent);
//					swap(n1connectingEdge, n2connectingEdge);
//				}
//			}
//
//			for (auto sub1 : n1->lowerLevel())
//			{
//				if (n1connectingEdge != nullptr)
//				{
//					if (n1connectingEdge->lowerLevel())
//				}
//			}
//
//			CalcForcesBetweenNodes(nodeToSplit->lowerLevel()[0], nodeToKeep, connectingEdge);
//			CalcForcesBetweenNodes(nodeToSplit->lowerLevel()[1], nodeToKeep, connectingEdge);
//		}
//	}
//}


