#include "Stdafx.h"
#include "Node.h"
#include "Edge.h" 

//NodePtr BaseObject<Node, 0xFAFAFAFA1>::lastEmpty = 0;

//
//void Node::SetPosToLower(){
//		if (_lowerLevel.size() == 1)
//			_lowerLevel.front()->Move(Pos());
//		else if (_lowerLevel.size() == 2)
//		{
//			double C = 1;
//			double K = 3;
//			auto idealDistance =  pow(C* K*K*K * _lowerLevel[0]->weight() * _lowerLevel[1]->weight() / _lowerLevelEdge->weight(), 1.0 / 3);
//			Point offset;
//			auto angle = rand();
//			offset.X() = sin(angle)*idealDistance/2;
//			offset.Y() = cos(angle)*idealDistance / 2;
//			_lowerLevel[0]->Move(Pos() + offset);
//			_lowerLevel[1]->Move(Pos() - offset);
//		}
//	}

void Node::CalcNeighbours()
{
	_neighbours.clear();
	_neighbours.reserve(_edges.size());
	for (auto e : _edges)
		_neighbours.push_back(e->otherNode(this));
}


void Node::MergeLowerLevel(NodePtr other, Edge* edge, int upperNumber){
	//assert(edge->lowerLevel().size() == 1);
	//assert(_lowerLevel.size() == 1);
	//assert(other->_lowerLevel.size() == 1);
	
	_lowerLevel.insert(_lowerLevel.end(), other->_lowerLevel.begin(), other->_lowerLevel.end());
	for (auto l : other->_lowerLevel)
		l->_upperLevel[upperNumber] = this;
	_weight += other->_weight;
	
}



void Node::validate(){
	auto node = this;
	//for (auto p : upperLevel())
	//{
	//	if (p != nullptr)
	//		assert(p->magic == 123456);
	//}

	for (int i = 0; i < node->edges().size(); i++)
	{		
//		assert(node->neightbours()[i] == node->edges()[i]->otherNode(node));
		assert(node->neightbours()[i]->magic == 123456);
	}


	//for (auto upp : node->upperLevel())
	//{
	//	if (upp == NULL)
	//		continue;
	//	assert(count(upp->lowerLevel().begin(), upp->lowerLevel().end(), node) == 1);
	//	assert(upp->level == node->level + 1);
	//		
	//}
}