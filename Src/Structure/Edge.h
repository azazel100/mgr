#pragma once
#include "node.h"


class Edge //:public BaseObject<Edge, (int)0xFAFAFAFA4>
{	
	int _index;
	NodePtr _n1;
	NodePtr	_n2;
	int _weight;
	//vector<Edge*> _upperLevel;	
	vector<Edge*> _lowerLevel;
	/*vector<pair<Edge*, double>> _edgeTransitionLength;*/
public:
	Edge* nextFree;
	double CoarseningRating;
	//double edgeTransitionLength(Edge* other)
	//{
	//	

	//	unsigned imin = 0, imax = (unsigned)_edgeTransitionLength.size() - 1;
	//	while (imin < imax)
	//	{
	//		unsigned imid = (imin+ imax)/2;

	//		// code must guarantee the interval is reduced at each iteration
	//		assert(imid < imax);
	//		// note: 0 <= imin < imax implies imid will always be less than imax

	//		// reduce the search
	//		if (_edgeTransitionLength[imid].first < other)
	//			imin = imid + 1;
	//		else
	//			imax = imid;
	//	}
	//	// At exit of while:
	//	//   if A[] is empty, then imax < imin
	//	//   otherwise imax == imin

	//	// deferred test for equality
	//	if ((imax == imin) && (_edgeTransitionLength[imin].first == other))
	//		return _edgeTransitionLength[imin].second;
	//	else
	//		return -1;


	//}
	//void clearEdgeTransitions()
	//{
	//	_edgeTransitionLength.clear();
	//}
	//void setTransitionLength(Edge* other, double len)
	//{
	//	_edgeTransitionLength.emplace_back(other, len);
	//}
	//void sortEdgeTransitions()
	//{
	//	sort(_edgeTransitionLength.begin(), _edgeTransitionLength.end(), 
	//		[](pair<Edge*, double>&e1, pair<Edge*, double> &e2)
	//	{
	//		return e1.first < e2.first;
	//	});
	//}

	//map<Edge*, double> edgeTransitionLength;
	int& weight(){ return _weight; }
	bool processed;
	bool deleted = false;

	/*
	In normal case, node may have only one parent in upper level.
	But we sometimes carete multiple alternative upper hierachies.
	w
	*/
	/*vector<Edge*>& upperLevel(){ return _upperLevel; }*/
	const vector<Edge*>& lowerLevel() const{ return _lowerLevel; }

	NodePtr node1(){ return _n1; }
	NodePtr node2(){ return _n2; }

	NodePtr otherNode(NodePtr node)
	{
		if (_n1 == node)
			return _n2;

		if (_n2 == node)
			return _n1;
		throw exception("not found");
	}

	bool operator<(const Edge &rhs) const { return _weight > rhs._weight; }

	bool operator==(const Edge &rhs) const
	{
		return _n1 == rhs._n1 && _n2 == rhs._n2
			|| _n1 == rhs._n2 && _n2 == rhs._n1;
	}


	
	void ReplaceNode(NodePtr oldNode, NodePtr newNode)
	{
		assert(newNode != NULL);
		if (_n1 == oldNode)
		
			_n1 = newNode;
		else if (_n2 == oldNode)
			_n2 = newNode;
		else
			throw exception();
		oldNode->RemoveEdge(this);
		newNode->AddEdge(this);
	}

	void Remove(int upperNum)
	{
		_n1->RemoveEdge(this);
		_n2->RemoveEdge(this);
		//for (auto lower : _lowerLevel)
		//	lower->_upperLevel[upperNum] = NULL;
		deleted = true;
	}

	Edge()
	{

	}

	void Reset()
	{
		//_upperLevel.clear();
		_lowerLevel.clear();
		deleted = false;
	}

	void Init(NodePtr node1, NodePtr node2, int index)
	{
		Reset();
		//assert(_upperLevel.size() == 0);
		assert(_lowerLevel.size() == 0);
		

		_weight = 1;
		//_upperLevel.push_back(nullptr);
		_index = index;
		_n1 =node1;
		_n2 =node2;
		_n1->AddEdge(this);
		_n2->AddEdge(this);
	}

	// copy edge from lower level	
	void Init(Edge* lowerLevel, int index, int upperIndex,bool dontInitNodes)
	{
		Reset();
		//assert(_upperLevel.size() == 0);
		assert(_lowerLevel.size() == 0);
//		assert(_lowerLevel->_upperLevel.size() > upperIndex);
		processed = false;
		//_upperLevel.push_back(nullptr);
		_index = index;
		_weight = lowerLevel->_weight;
		_lowerLevel.push_back(lowerLevel);
		_n1 = lowerLevel->node1()->upperLevel()[upperIndex];
		_n2 = lowerLevel->node2()->upperLevel()[upperIndex];
		_n1->AddEdge(this);
		_n2->AddEdge(this);
		//lowerLevel->_upperLevel[upperIndex] = this;
	}

	// copy edge from lower level	
	void Init(Edge* lowerLevel, int index,int upperIndex)
	{
		Reset();
//		assert(_upperLevel.size() == 0);
		assert(_lowerLevel.size() == 0);
//		assert(_lowerLevel->_upperLevel.size() > upperIndex);
		processed = false;
		//_upperLevel.push_back(nullptr);
		_index = index;
		_weight = lowerLevel->_weight;
		_lowerLevel.push_back(lowerLevel);
		_n1 = lowerLevel->node1()->upperLevel()[upperIndex];
		_n2 = lowerLevel->node2()->upperLevel()[upperIndex];
		_n1->AddEdge(this);
		_n2->AddEdge(this);				
		//lowerLevel->_upperLevel[upperIndex] = this;
	}
public:
	
	~Edge()
	{

	}
public:

	void Merge(Edge* other, int upperIndex)
	{
		assert(_lowerLevel.size() > 0);
		_lowerLevel.insert(_lowerLevel.end(), other->_lowerLevel.begin(), other->_lowerLevel.end());
		//for (auto l : other->_lowerLevel)
		//	l->_upperLevel[upperIndex] = this;
		_weight += other->_weight;
		other->_lowerLevel.clear();
		other->Remove(upperIndex);
	}

	private:
	/*	void AddNode(NodePtr node)
		{
			assert(_n1 == NULL && _n2 != node || _n2 == NULL && _n1 != node);

			if (_n1 == NULL)
				_n1 = node;
			else if (_n2 == NULL)
				_n2 = node;
			else
				throw exception();

			node->AddEdge(this);
		}*/


};
