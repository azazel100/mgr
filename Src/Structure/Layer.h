#pragma once
#include "Edge.h"
#include "Node.h"
#include "../utils/BaseObject.h" 
#include "../utils/pool.h"

class Layer : public BaseObject<Layer, (int)0xFAFAFAFA6>
{	
	int magic = 123456;
	vector<Layer*> m_upper;
	Layer* m_lower=nullptr;
	bool m_isDebugRun;
public:
	
	int level=0;
	void Draw(const Rect& nodeExtent, const Point& imageExtent, const wchar_t* fileName, bool smooth);

	vector<Layer*>& upper()
	{ 
		
		return m_upper; 
	}
	Layer* lower(){ return m_lower; }

	std::vector<NodePtr > _nodes;
	std::vector<Edge*> _edges;

	const std::vector<NodePtr >& nodes(){
		return _nodes;}
	const std::vector<Edge*>& edges(){ return _edges; }

	Layer(bool isDebugRun)
	{
		m_isDebugRun = isDebugRun;
		 level = 0;
	}

	~Layer()
	{
		magic = -1;
		for (auto n : nodes())
			NodeDelete( n);
		for (auto e : edges())
			pool<Edge>::Delete(e);
	}

	void ReadPositions(wstring filename)
	{
		ifstream myReadFile;
		myReadFile.open(filename);
		char line[200];
		level = 0;

		int nodeCount;
		
		
		while (myReadFile.peek() == '%')
			myReadFile.getline(line, 200);

		myReadFile >> nodeCount;
		if (nodeCount != nodes().size())
			throw exception("Invalid node count");

		int index=0;
		while (!myReadFile.eof()) {
			float x, y;			
			myReadFile >> x>> y;
			if (myReadFile.fail())
				break;
			char d;
			while (myReadFile.peek() != '\n' && !myReadFile.eof())
				myReadFile.get(d);
			nodes()[index]->Move(Point(x, y));
			index++;
		}		
	}

	void WritePositions(wstring filename)
	{
		ofstream file;
		file.open(filename, ios::trunc);
		file << nodes().size() << endl;
		for (auto n : nodes())
			file << n->Pos().X() << " " << n->Pos().Y() << endl;

	}
	
	void SaveAs(wstring filename)
	{
		
		ofstream file(filename,ios_base::trunc);
		file << nodes().size() << " " << nodes().size() << " " << edges().size() << endl;
		for (int i = 0; i < nodes().size(); i++)
		{
			//nodes()[i]->index() = i;			
		}
		for (auto edge : edges())
		{
			file 
				<< edge->node1()->index() + 1 << " " 
				<< edge->node2()->index() + 1 << " "
				<< edge->weight() + 1 << " "
				<< edge->node1()->weight() + 1 << " "
				<< edge->node2()->weight() + 1 << " "				
				<< endl;
		}
	}

	Layer(wstring filename, bool isDebugRun)
	{
		m_isDebugRun = isDebugRun;
		ifstream myReadFile;
		auto lasti = filename.find(L".mtx_coarsened") ;
		bool readWeights = lasti != string::npos;
		upper().push_back(nullptr);

		myReadFile.open(filename);
		if (!myReadFile.is_open())
		{
			throw std::exception("Unable to open file");
		}
		char line[200];
		level = 0;
		
		int nodeCount1,nodeCount2,nodeCount;
		int edgeCount;
		_nodes.clear();
		_edges.clear();

		while (myReadFile.peek() == '%')
			myReadFile.getline(line, 200);

		myReadFile >> nodeCount1 >> nodeCount2 >> edgeCount;
		nodeCount = max(nodeCount1, nodeCount2);
		
		_edges.reserve(edgeCount);
		_nodes.reserve(nodeCount);
		int count = 0;
		while (!myReadFile.eof()) {
			int index1, index2;			
			int weight1=1, weight2=1, edgeWeight=1;
			myReadFile >> index1 >> index2;
			if (myReadFile.fail())
				break;
			if (readWeights)
				myReadFile >> edgeWeight >> weight1 >> weight2;
			char d;			
			while (myReadFile.peek() != '\n' && !myReadFile.eof())			
				myReadFile.get(d);			
			if (index1 == index2)
				continue;
			

			count++;
			if (count % 10000 == 0)
				cout << " Read " << count << " edges." << endl;
			index1--;
			index2--;
			while (nodes().size() < index1+1)
			{
				_nodes.push_back(pool<Node>::New((int)nodes().size()));
			}
			while (nodes().size() < index2+1)
			{
				_nodes.push_back(pool<Node>::New((int)nodes().size()));
			}

			auto node1 = nodes()[index1];
			auto node2 = nodes()[index2];
			node1->weight() = weight1;
			node2->weight() = weight2;
			bool exists = false;
			for (auto e : node1->edges())
			{
				if (e->otherNode(node1) == node2)
				{
					exists = true;
					break;
				}
			}
			if (exists)
				continue;
			auto edge = pool<Edge>::New(node1, node2, (int)edges().size());
			edge->weight() = edgeWeight;
			_edges.push_back(edge);
		}
		for (auto n : nodes())
			n->CalcNeighbours();
	}
//
//	Layer(Graph& g, GraphAttributes& ga)
//	{
//		level = 0;
//
//		/*int nodeCount = g.numberOfNodes();
//		NodePtr nodemem = (NodePtr )new int[nodeCount* sizeof(Node) / sizeof(int)];
//
//		int edgeCount = g.numberOfEdges();
//		NodePtr nodemem = (NodePtr )new int[nodeCount* sizeof(Node) / sizeof(int)];
//*/
//		vector<Node>* nodeMem = new vector<Node>();
//		nodeMem->reserve(g.numberOfNodes());
//
//		vector<Edge>* edgeMem = new vector<Edge>();
//		edgeMem->reserve(g.numberOfEdges());
//
//		for (auto node = g.firstNode(); node != NULL; node = node->succ())
//		{
//			assert(nodes().size() == node->index());
//			nodeMem->emplace_back( node, ga, nodes().size() );
//			nodes().push_back(&nodeMem->back());
//
//		}
//
//		for (auto edge = g.firstEdge(); edge != NULL; edge = edge->succ())
//		{
//			if (edge->source() == edge->target())
//				continue;
//			auto node1 = nodes()[edge->source()->index()];
//			auto node2 = nodes()[edge->target()->index()];
//
//			bool exists = false;
//			for (auto edge1 : node1->edges())
//			{
//				if (edge1->otherNode(node1) == node2)
//				{
//					exists = true;
//					break;
//				}
//			}
//			if (exists)
//				continue;
//			edgeMem->emplace_back(node1, node2, edges().size());
//			edges().push_back(&edgeMem->back());
//		}
//	}	
//
	
	void validate()
	{
		
		for (auto edge : edges())
		{
		}

		for (auto node : nodes())
		{
			node->validate();
				
		}
	}

	static Layer* CreateQuad(int level,bool isDebugRun)
	{
		auto layer = new Layer(isDebugRun);
		
		int nodeCount = (int)pow(2, level);

		for (int x = 0; x < nodeCount; x++)
		{
			for (int y = 0; y < nodeCount; y++)
			{
				auto node = pool<Node>::New((int)layer->nodes().size());
				node->Pos().X() = x;
				node->Pos().Y() = y;
				layer->_nodes.push_back(node);
			}
		}

		for (int x = 0; x < nodeCount; x++)
		{
			for (int y = 0; y < nodeCount - 1; y++)
			{
				layer->_edges.push_back(pool<Edge>::New(layer->nodes()[x*nodeCount + y], layer->nodes()[x*nodeCount + y + 1], (int)layer->edges().size()));
			}
		}

		for (int y = 0; y < nodeCount; y++)
		{
			for (int x = 0; x < nodeCount - 1; x++)
			{
				layer->_edges.push_back(pool<Edge>::New(layer->nodes()[x*nodeCount + y], layer->nodes()[(x + 1)*nodeCount + y], (int)layer->edges().size()));
			}
		}

		/*for (int y = 0; y < nodeCount - 5; y+=4)
		{
			for (int x = 0; x < nodeCount - 5; x+=4)
			{
				layer->_edges.push_back(pool<Edge>::New(layer->nodes()[x*nodeCount + y], layer->nodes()[(x + 5)*nodeCount + y + 1], (int)layer->edges().size()));
			}
		}*/

		for (auto n : layer->nodes())
			n->CalcNeighbours();
		return layer;
	}

	void CheckDups()
	{
		/*for (auto node:nodes())
		{
			for (int i = 0; i < node->neightbours().size(); i++)
			{
				for (int j = i + 1; j < node->neightbours().size(); j++)
					assert(node->neightbours()[i] != node->neightbours()[j]);
			}
			
		}*/
#ifdef SUEPRDEBUG
		for (int i = 0; i < edges().size(); i++)
		{
			auto edge1 = edges()[i];
			for (int j = i + 1; j < edges().size(); j++)
			{
				auto edge2 = edges()[j];
				assert(!(*edge1 == *edge2));
			}

		}
#endif
	}

	void CheckLatestEdgeDup()
	{
#ifdef SUEPRDEBUG
		auto edge1 = edges()[edges().size() - 1];
		for (int j = 0; j < edges().size() - 1; j++)
		{
			auto edge2 = edges()[j];
			assert(!(*edge1 == *edge2));
		}
#endif
	}

	static bool EdgeIsCollapsed(Edge* edge)
	{
		return edge->deleted;
	}

	static bool NodeIsCollapsed(NodePtr node)
	{
		return node->deleted;
	}

	static double RateNodesLocality(NodePtr node1, NodePtr node2)
	{
		double sum = 0;
		for (auto n : node1->neightbours())
		{
			if (n == node2)
				continue;
			bool isConnected = false;
			for (auto nn : n->neightbours())
				if (nn == node2)
				{
				isConnected = true;
				break;
				}
			if (isConnected)
				sum += 1;
			else
				sum += 2 + node1->averageDistance;
		}
		return sum;
	}

	static double RateEdgeLocality(Edge* edge)
	{
		return RateNodesLocality(edge->node1(), edge->node2())
			+ RateNodesLocality(edge->node2(), edge->node1());
	}

	static inline double RateEdge(Edge* edge)
	{
		return min(edge->node1()->weight(), edge->node2()->weight()) + max(edge->node1()->weight(), edge->node2()->weight())- edge->weight();
		return -edge->weight();
		//return -1 / RateEdgeLocality(edge);// -edge->weight();
		return -min(edge->node1()->weight(), edge->node2()->weight());
		return edge->node1()->weight() + edge->node2()->weight();
	}

	static bool CompareEdges(Edge* e1, Edge* e2)
	{
		return RateEdgeLocality(e1) < RateEdgeLocality(e2);;
	}

	static std::minstd_rand0 rand_engine;
	
	void CollapseEdges(int upperNumber)
	{
		
		if (!m_isDebugRun ||upperNumber != 0)
		{
			//cout <<"Random: "<< rand_engine() << ", " << rand()<<"\n";
			/*
			When building alternative upper coarsenings, we try to make them different from 
			existing ones.
			*/

			/*auto firstAlreadyIncluded= std::stable_partition(edges().begin(), edges().end(), [&](Edge* edge)
			{
				bool wasCollapsedInOtherUppers = false;
				auto lower = edge->lowerLevel()[0];
				for (auto upper : lower->upperLevel())
				{
					if (upper == NULL)
					{
						wasCollapsedInOtherUppers = true;;
						break;
					}
				}
				return !wasCollapsedInOtherUppers;
			});
			cout << "Collapse edge, new not included previously: " << firstAlreadyIncluded - edges().begin() << endl;*/
			auto firstAlreadyIncluded = _edges.begin();
			shuffle(firstAlreadyIncluded, _edges.end(), rand_engine);
		}
		CalcAproxNodeProperties();
		/*for (auto n : nodes())
			n->CalcNeighbours();
		*/
		if (true)
		{			
			if (m_isDebugRun)
				stable_sort(_edges.begin(), _edges.end(), [](Edge* e1, Edge* e2)
				{
				return e1->weight() > e2->weight();
				});
			else
				sort(_edges.begin(), _edges.end(), [](Edge* e1, Edge* e2)
				{
					return RateEdge(e1) < RateEdge(e2);;
				});
			
			for (auto edge : edges())
			{
				if (EdgeIsCollapsed(edge))
					continue;

				// if any of the nodes() was created from collapsig an edge, continue
				if (edge->node1()->lowerLevel().size() > 1
					|| edge->node2()->lowerLevel().size() > 1)
					continue;

				

				CollapseEdge(edge, upperNumber);




			}

		}
		else
		{

			int toCollapse = (int)edges().size() / 2;
			// collapse eges

			for (auto edge : edges())
				edge->CoarseningRating = RateEdge(edge);

			sort(_edges.begin(), _edges.end(), [](Edge* e1, Edge* e2)
			{
				return e1->CoarseningRating < e2->CoarseningRating;
			});


			while (toCollapse>0)
			{
				auto edge = edges().front();
				CollapseEdge(edge, upperNumber);
				toCollapse--;
			}						
		}

	

	}

	void CollapseEdge(Edge* edge,int upperNumber)
	{
		auto node1 = edge->node1();
		auto node2 = edge->node2();
		auto node = node1;
					assert(edge->lowerLevel().size() == 1);
		edge->Remove(upperNumber);

		auto node1edges = node1->edges();
		auto node2edges = node2->edges();

		for (auto e : node2edges)
		{
			e->ReplaceNode(node2, node1);
		}
		node->MergeLowerLevel(node2, edge, upperNumber);
		node2->level = -1;
		node2->magic = 0;
		node2->deleted = true;

		// find edges() to merge
		for (auto edge1 : node1edges)
		{
			auto otherNode1 = edge1->otherNode(node);
			for (auto edge2 : node2edges)
			{
				auto otherNode2 = edge2->otherNode(node);
				if (otherNode1 == otherNode2)
				{
					edge1->Merge(edge2, upperNumber);
				}
			}
		}
	}

	void RemoveDeadElements()
	{	
		vector<Edge*>::iterator edgesToErase;
		if (m_isDebugRun)
			edgesToErase = stable_partition(_edges.begin(), _edges.end(), [](Edge*e){return !e->deleted; });
		else
			edgesToErase = partition(_edges.begin(), _edges.end(), [](Edge*e){return !e->deleted; });
		auto d = edgesToErase - edges().begin();		
		for (auto i = edgesToErase; i != edges().end(); i++)
		{
			Edge*e = *i;
			pool<Edge>::Delete(e);
		}
		_edges.erase(edgesToErase, edges().end());
		_edges.shrink_to_fit();

		auto nodesToErase = partition(_nodes.begin(), _nodes.end(), [](NodePtr n){return !n->deleted; });
		for (auto i = nodesToErase; i != nodes().end(); i++)
		{
			NodePtr n = *i;
			n->level = -1;
			NodeDelete( n);
			
		}

		_nodes.erase(nodesToErase, nodes().end());
		_nodes.shrink_to_fit();
	}


	Layer(Layer* lowerLayer, int upperNumber)
	{
		m_isDebugRun = lowerLayer->m_isDebugRun;
		level = lowerLayer->level + 1;
		m_upper.push_back(nullptr);
		m_lower = lowerLayer;
		
		if (m_lower->m_upper.size() <= upperNumber)
		{
			//assert(false);
			m_lower->m_upper.resize(upperNumber + 1);
		}
		m_lower->m_upper[upperNumber] = this;
		_nodes.reserve(lowerLayer->nodes().size());
		

		
		for (auto n : lowerLayer->nodes())
		{			
			_nodes.push_back(pool<Node>::New(n, (int)nodes().size(), upperNumber));
		}
		if (false)
		{
			_edges.resize(lowerLayer->edges().size());
			concurrency::parallel_for(0, (int)edges().size(), [&](int i)
			{
				_edges[i] = pool<Edge>::New(lowerLayer->edges()[i], (int)edges().size(), upperNumber, true);
			}, concurrency::static_partitioner());

			concurrency::parallel_for((size_t)0, nodes().size(), [&](int i)
			{
				auto node = nodes()[i];
				auto lowerNode = lowerLayer->nodes()[i];
				for (int j = 0; j < lowerNode->edges().size(); j++)
				{
					node->AddEdge(lowerNode->edges()[j]->upperLevel()[upperNumber]);
				}
			}, concurrency::static_partitioner());
		}
		else
		{
			_edges.reserve(lowerLayer->edges().size());
			for (auto e : lowerLayer->edges())
				_edges.push_back(pool<Edge>::New(e, (int)edges().size(), upperNumber));
		}

		CheckDups();
		CollapseEdges(upperNumber);
		RemoveDeadElements();
		concurrency::parallel_for_each(nodes().begin(), nodes().end(), [](Node*n)
		{
			n->CalcNeighbours();
		});		
			
		
		CheckDups();
	}

	Rect CalcExtent()
	{
		Rect initialExtent;
		for (auto n : nodes())
		{
			initialExtent.Expand(n->Pos());
		}
		if (initialExtent.width() < initialExtent.height())
		{
			auto center = (initialExtent.max + initialExtent.min) / 2;
			initialExtent.min.X() = center.X() - initialExtent.height() / 2;
			initialExtent.max.X() = center.X() + initialExtent.height() / 2;
		}
		else
		{
			auto center = (initialExtent.max + initialExtent.min) / 2;
			initialExtent.min.Y() = center.Y() - initialExtent.width() / 2;
			initialExtent.max.Y() = center.Y() + initialExtent.width() / 2;
		}
		return initialExtent;
	}

	double CalcAverageEdgeLength()
	{
		double avgLen = accumulate(edges().begin(), edges().end()
			, 0.0
			, [&](double avg, Edge* edge){return avg + sqrt(edge->node1()->Pos().DistanceSqr(edge->node2()->Pos())); }
		) / edges().size();
		return avgLen;
	}

	void CalcAproxNodeProperties();
	
/*
	void WriteToOryginal(GraphAttributes& ga){
		for (auto n : nodes())
		{
			n->WriteOryginal(ga);
		}
	}*/

	/*void Save(const wstring& filename)
	{

	}*/

};