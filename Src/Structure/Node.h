#pragma once
#include "../Utils/Point.h"
#include "../Utils/Rect.h"
#include "../Utils/BaseObject.h"
class Node;
class Edge;

template<class Type>
class PointerWrapper
{
	//int magic = 0xFAFAFAFA9;
	Type* ptr;
public:
	PointerWrapper()
	{
		ptr = (Type*)0xDDDDDDDD;
	}
	PointerWrapper(Type* p)
	{
		ptr = p;
	}

	~PointerWrapper()
	{
	}

	bool operator==(const PointerWrapper& other)const
	{
		return ptr == other.ptr;
	}

	bool operator!=(const PointerWrapper& other)const 
	{
		return ptr != other.ptr;
	}

	
	bool operator<(const PointerWrapper& other)const
	{
		return ptr < other.ptr;
	}


	Type* operator->()
	{
		return ptr;
	}

	Type*const  operator->()const 
	{
		return ptr;
	}
	
	void Delete()
	{
		delete ptr;
		ptr = (Type*)0xEEEEEE;
	}
};


//typedef PointerWrapper<Node> NodePtr;
typedef Node* NodePtr;
template<class T>
inline void NodeDelete(PointerWrapper<T> p)
{
	p.Delete();
}

template<class T>
inline void NodeDelete(T* p)
{
	pool<Node>::Delete( p);
}


class Node //:public BaseObject<Node, (int)0xFAFAFAFA1>
{
	
	std::vector<NodePtr > _neighbours;

	int _index;
	Point _pos;
	Rect _bound;
	
	std::vector<NodePtr > _lowerLevel;
	NodePtr _upperLevel[10];
	Edge* _lowerLevelEdge=NULL;
	//NodeElement* _oryginalNode = NULL;

	std::vector<Edge*> _edges;
	
	
	int _weight; // number of nodes() in lower levels

public: 
	Node* nextFree;
	int magic = 123456;
	int index(){ return _index; }
	int level = 0;
	double averageDistance=0;//average graph distance in lower subgraph
	double graphDiameter=0;
    double lastAproxDistance[10];
	//double lastExactDistance;
	Edge* lastIncomingEdge[10];
	unsigned  statusMarker[10];
	NodePtr filterNode;

	/*
	  In normal case, node may have only one parent in upper level.
	  But we sometimes carete multiple alternative upper hierachies.
	  w
	*/
	const NodePtr* upperLevel() {

		/*if (_upperLevel.size() == 0)
			return nullptr;
		if (_upperLevel.size() <= num)
			return _upperLevel[0];*/
		return _upperLevel;
	}
	
	
	double boundDiamSqr;
	Point force;
	Point avgForce;
	Point speed;
	bool deleted = false;
	const Rect& Bound() { return _bound; }	
	int& weight() { return _weight; }
	const vector<NodePtr >& lowerLevel()const{ return _lowerLevel; }
	Edge* lowerEdge(){ return _lowerLevelEdge; }

	void Move(const Point& newPos)
	{
		_pos = newPos;
	}

	void MoveBy(const Point& vector)
	{
		_pos += vector;
	}

	void CalcNeighbours();

	const vector<NodePtr >& neightbours(){ return _neighbours; }

	void CalculatePosFromLoverLevels(int levelCount)
	{
		if (levelCount == 0)
		{
			_bound.min = _bound.max = _pos;			
			boundDiamSqr = 0;			
			return;
			
		}
		else
		{
			Point posSum (0);						

			_bound.min = std::numeric_limits<float>::max();
			_bound.max = std::numeric_limits<float>::lowest();
			_weight = 0;

			bool isFirst = true;
			for (auto n : _lowerLevel)
			{				
				n->CalculatePosFromLoverLevels(levelCount - 1);
				_weight += n->weight();
				posSum += n->_pos*n->_weight;
								
				_bound.Expand(n->Bound().min);
				_bound.Expand(n->Bound().max);											

			}		
			boundDiamSqr = max(_bound.width(), _bound.height());
			boundDiamSqr *= boundDiamSqr;
			_pos = posSum* (1.0/_weight);
		}

	}

	const vector<Edge*>& edges()const { return _edges; }
	

	//NodeElement* oryginalNode(){ return _oryginalNode; }
	Point& Pos(){
		return _pos;
	}

	void SetPosToLower();
	


	
	void Reset()
	{
		deleted = false;
		magic = 123456;
		_lowerLevel.clear();
		_edges.clear();
		_neighbours.clear();
		_lowerLevelEdge = nullptr;
		level = 0;
		for (int i = 0; i < 10; i++){
			statusMarker[i] = 0;
			lastIncomingEdge[i] = 0;
		}
	}
	//copy node from lower level
	void Init(NodePtr lowerLevel1, int index,int upperIndex)
	{
		Reset();
		//assert(_upperLevel.size() == 0);
		assert(_lowerLevel.size() == 0);
		assert(_edges.size() == 0);
		assert(_neighbours.size() == 0);
		//assert(lowerLevel1->_upperLevel.size() > upperIndex);

		level = lowerLevel1->level + 1;
		_upperLevel[0]=(nullptr);
		_lowerLevel.reserve(2);
		_edges.reserve(4);
		_index = index;
		
		lowerLevel1->_upperLevel[upperIndex] = this;
		_lowerLevel.push_back(lowerLevel1);				
		_weight = lowerLevel1->_weight;
		//_edges.reserve(lowerLevel1->edges().size());

	}


	void Init(int index)
	{
		Reset();
		//assert(_upperLevel.size() == 0);
		assert(_lowerLevel.size() == 0);
		assert(_edges.size() == 0);
		assert(_neighbours.size() == 0);
		_upperLevel[0] = nullptr;
		_index = index;		
		_weight = 1;
		
	}
public:
	~Node()
	{

	}
public:
	
	void AddEdge(Edge* edge)
	{
		assert(count(_edges.begin(), _edges.end(), edge) == 0);
		_edges.push_back(edge);		
		_neighbours.clear();
		assert(count(_edges.begin(), _edges.end(), edge) == 1);
	}

	void RemoveEdge(Edge* edge)
	{
		assert(count(_edges.begin(), _edges.end(), edge) == 1);
		
		_edges.erase(remove(_edges.begin(), _edges.end(), edge),_edges.end());
		_neighbours.clear();
	}

	void MergeLowerLevel(NodePtr other, Edge* edge,int upperNumber);

	

	void validate();
	
};



