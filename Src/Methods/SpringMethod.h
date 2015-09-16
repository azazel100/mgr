#include "../utils/Stopwatch.h"

class SpringMethod :public IForceMethod
{

	vector<Layer*> alternativeTopLayers;
	vector<Layer*> alternativeLayers;
	double repositionScalingFactor;
	int upperIndex = 0;
	double m_totalWaitSeconds = 0;
	bool m_isFirstInit = true;
public:
	double exactForceMaxDistance = 0;
	bool useAproxForce = true;
	bool useAproxForceEdgeTransitions = false;
	bool useAproxForceRandomUpperLevels = true;
	bool useAproxForceRandomUpperLevelsParallel = true;
	bool useAproxForceAlternativeUpperLevels = false;
	bool useAproxForceAlternativeUpperLevelsAveraging = false;	
	bool useAproxForcePersistentNodeProperties = true;
	bool useAproxNodeProperties = true;
	bool useAproxForceSkipManyLevels = false;
	double useAproxForceSkipLevelThreashold = 1.4;
	int aproxForceAlternativeUpperLevelsCount= 1;


	void DrawAlternativeLayouts()
	{
		auto tops = alternativeTopLayers;
		tops.push_back(topLayer);

		auto extent = layer->CalcExtent();

		int num = 0;
		for (auto top : tops)
		{
			num++;
			for (auto l = top; l->level > 0; l = l->lower())
			{				
				Image image(extent, Point(500, 500), l->edges().size(), true);
				image.SetFillColor(100, 0, 0, 0);
				for (auto n : l->nodes())
					image.DrawBoundingBox(n, 3, 0);
				for (auto e : layer->edges())
					image.DrawEdge(e);
				wchar_t buff[255];
				wsprintf(buff, L"output\\layer %d alternative %d .png", l->level,num);
				image.Save(buff);
			}
		}

	}

	SpringMethod()
	{
		
		toVisitCombinable.resize(10);
		for (auto& tv : toVisitCombinable)
		{
			tv.reserve(40000);
			FillMemory(tv.data(), sizeof(NodePtr ) * 3000,0xF3);
		}
	}

	wstring Describe()override
	{
		wstringstream s;	
		s << "spring";

		if (useAproxForce)
		{			
			s << " aprox";

			if (useAproxForceSkipManyLevels)
				s << " skipManyLevels";

			if (useAproxForceSkipLevelThreashold != 1.4)
				s << " skipThreashold " << useAproxForceSkipLevelThreashold;

			if (useAproxForceEdgeTransitions)
			{			
				s << ", transitions";
			}			
			if (useAproxForceRandomUpperLevels)
				s << " random uppers";
			else if (useAproxForceAlternativeUpperLevels)
			{				
				
				s << ", additional " << aproxForceAlternativeUpperLevelsCount << " uppers";
				if (useAproxForceAlternativeUpperLevelsAveraging)
					s << " averaged";
				
			}
			if (useAproxNodeProperties)
			{
				s << ", aprox props";
				if (useAproxForcePersistentNodeProperties)
				{
					s << " persistent";
				}
			}
		}
		else
		{
			s << " exact";

			if (exactForceMaxDistance != numeric_limits<int>::max())
				s << ", max dist " << exactForceMaxDistance;
		}
		return s.str();
	}

	double CalculateScaling()
	{		
		return 1;
	}

	//void InitLayerPositions()override
	//{
	//	int levelFromTop = topLayer->level - layer->level;
	//	int i = 0;

	//	/*if (m_isFirstInit)
	//	{
	//		auto diam = CalcDiameterFast();
	//		double angle = 0;
	//		for (int i = 0; i < layer->nodes().size(); i++)
	//		{
	//			auto node = layer->nodes()[i];
	//			node ->Pos().X() = sin(angle)* diam / 2;
	//			node->Pos().Y() = cos(angle)* diam / 2;
	//			node->speed = node->Pos();
	//			angle += 2.0*3.14 / layer->nodes().size();				
	//		}
	//		topLayer->nodes()[0]->CalculatePosFromLoverLevels(topLayer->level - layer->level);
	//		m_isFirstInit = false;
	//		return;
	//	}*/

	//	if (levelFromTop == 0)
	//	{
	//		layer->nodes()[0]->Pos() = 0;
	//	}
	//	else if (levelFromTop == 1)
	//	{
	//		layer->nodes()[0]->Pos() = Point(0, 0);
	//		layer->nodes()[1]->Pos() = Point(1, 0);
	//	}		
	//	else
	//	{			
	//		double factor = repositionScalingFactor;;

	//		auto avglen = layer->upper()[0]->CalcAverageEdgeLength();
	//		for (auto node : layer->nodes())
	//		{
	//			node->force = 0;
	//			node->Pos() = node->upperLevel()[0]->Pos();
	//			node->Pos() *= factor;				
	//			node->speed.X() = rand() % 100 * (avglen / 100000);
	//			node->speed.Y() = rand() % 100 * (avglen / 100000);
	//			node->Pos() += node->speed;

	//			fordimm<Point::dimmensions - 2>::loop([&](unsigned d)
	//			{					
	//				;// node->Pos().val[d + 2] += rand() % 100 * (avglen / 1000);
	//			});

	//			if (isnan(node->Pos().Lentgh()))
	//				return;
	//		}
	//	}
	//}

	void RebuildUpperLayer(int number)
	{
		auto l = layer->upper()[number];
		while (l != nullptr)
		{
			auto nl = l->upper()[0];
			delete l;
			l = nl;
		}

		auto upperLayer = new Layer(layer, number,Layer::EdgeRemoval);		
		while (upperLayer->nodes().size() > 1)
		{
			upperLayer = new Layer(upperLayer, 0, Layer::EdgeRemoval);
		}
		PrepareLayer(upperLayer, false);
		alternativeTopLayers[number] = upperLayer;

	}

	unsigned prevUpperDiam = 1;

	virtual void InitExecution()
	{
		if (!useAproxForceRandomUpperLevels)
			useAproxForceRandomUpperLevelsParallel = false;

		upperIndex = 0;
		for (int i = 0; i < 10; i++)
			currentRoundMarker[i] = 1;

		if (useAproxForce)
		{
			PrepareLayer(topLayer, true);


			if (!useAproxNodeProperties)
			{
				CalcExactNodeProperties(topLayer->nodes()[0], topLayer->level);// -layer->level);
			}
		}

	}

	virtual void InitLayer()
	{
		upperIndex = 0;

		for (int i = 0; i < 10; i++)
			currentRoundMarker[i] = 1;
		

		/*for (auto n : layer->nodes())
			n->weight() = 1;
		

		for (auto e : layer->edges)
			e->weight() = 1;
*/
		
		if (useAproxForceRandomUpperLevelsParallel)
		{
			waitForRebildUpper();
			upperIndex = 1;

			if (layer->upper().size() > 0 && layer->upper()[0]->upper().size() > 1)
			{
				auto l = layer->upper()[0]->upper()[1];
				while (l != nullptr)
				{
					auto nl = l->upper()[0];
					delete l;
					l = nl;
				}
			}

			if (!alternativeTopLayers.empty()){
				topLayer = alternativeTopLayers[0];				
			}
			else
			{
				alternativeTopLayers.resize(2);
				alternativeTopLayers[0] = topLayer;
				alternativeTopLayers[1] = nullptr;
			}

			//for (auto e : layer->edges())
			//	e->upperLevel().resize(2);
			layer->upper().resize(2);
			
			
		}		
		
		
		if (useAproxForce && !useAproxForcePersistentNodeProperties)
		{
			PrepareLayer(topLayer, true);
			

			if (!useAproxNodeProperties)
			{
				CalcExactNodeProperties(topLayer->nodes()[0], topLayer->level);// -layer->level);
			}
		}

		if (useAproxForce && useAproxForceAlternativeUpperLevels &&  topLayer->level - layer->level >2)
		{

			BuildAlternativeUpperLevels(aproxForceAlternativeUpperLevelsCount);
		}

		if (useAproxForce)
		{						
			if (!useAproxNodeProperties)
			{				
				for_each(
					alternativeTopLayers.begin(),
					alternativeTopLayers.end(),
					[&](Layer* l)
				{
					CalcExactNodeProperties(l->nodes()[0], l->level);// -layer->level);
				});
			}
		}
		
		
		if (!useAproxForcePersistentNodeProperties)
		{
			topLayer->nodes()[0]->graphDiameter = CalcDiameterFast();
			auto thisDiam = topLayer->nodes()[0]->graphDiameter;
			repositionScalingFactor = thisDiam / prevUpperDiam;
			prevUpperDiam = thisDiam;
		}
		else

		{
			repositionScalingFactor = 1;
		}
		
		
	}


	Point CalcForce(NodePtr sourceNode, int threadID)
	{
		if (useAproxForce)
			return CalcForceApprox(sourceNode, threadID);
		else
			return CalcForceExact(sourceNode, threadID);
	}

	enum  NodeStatusType:unsigned { Visited, Queued, Subdivided };

	void StartNextRound(int threadId)
	{
		currentRoundMarker[threadId] += 3;
	}

	NodeStatusType GetNodeStatus(NodePtr node, unsigned int threadId)
	{
		return (NodeStatusType)(node->statusMarker[threadId] - currentRoundMarker[threadId]);

	}

	void SetNodeStatus(NodePtr node, NodeStatusType status, int threadId)
	{
		node->statusMarker[threadId] = currentRoundMarker[threadId] + status;

	}

	unsigned  GetStatusMarker(NodeStatusType status, int threadId)
	{
		return currentRoundMarker[threadId] + status;
	}

	
	concurrency::task<void>* rebuildUpperTask = nullptr;

	void waitForRebildUpper()
	{
		if (rebuildUpperTask != nullptr)
		{
			Stopwatch watch;
			watch.Start();
			rebuildUpperTask->wait();
			watch.Stop();
			m_totalWaitSeconds += watch.Seconds();
			if (watch.Seconds() > 0.001)
				cout << "  waited for rebuild upper " << watch.Seconds() << "s" << " total (" << m_totalWaitSeconds<<"\n";
			delete rebuildUpperTask;
			rebuildUpperTask = nullptr;
		}
	}

	void InitIteration(int number)override
	{
		/*if (layer->level == 0 && number > 45)
		{
			useAproxForce = false;
			exactForceMaxDistance = 4;
		}*/

		if (useAproxForceRandomUpperLevelsParallel)
		{

			waitForRebildUpper();

			auto prevUpperIndex = upperIndex;
			upperIndex++;
			if (upperIndex == 2)
				upperIndex = 0;
			rebuildUpperTask = new concurrency::task<void>([=](){
				RebuildUpperLayer(prevUpperIndex);
			});

			topLayer = alternativeTopLayers[upperIndex];
			topLayer->nodes()[0]->CalculatePosFromLoverLevels(topLayer->level - layer->level);

		}
		else if (useAproxForceRandomUpperLevels)
		{
			
			if (upperIndex == 6)
				upperIndex = 0;
			upperIndex++;

			if (upperIndex == 1)
			{
				BuildAlternativeUpperLevels(6);
			}
			auto top = alternativeTopLayers[upperIndex - 1];
			
			
			
			top->nodes()[0]->CalculatePosFromLoverLevels(top->level - layer->level);	

			//for (auto l = alternativeLayers[upperIndex - 1]; l != NULL; l = l->lower())
			//{
			//	l->validate();
			//}
			
		}
		else if (useAproxForceAlternativeUpperLevels && !useAproxForceAlternativeUpperLevelsAveraging)
		{


			auto topLayer = this->topLayer;
			upperIndex++;
			if (upperIndex > alternativeTopLayers.size())
				upperIndex = 0;

			if (upperIndex > 0)
			{
				topLayer = alternativeTopLayers[upperIndex - 1];
			}
			topLayer->nodes()[0]->CalculatePosFromLoverLevels(topLayer->level - layer->level);

		}
		else if (useAproxForceAlternativeUpperLevels && useAproxForceAlternativeUpperLevelsAveraging)
		{

			topLayer->nodes()[0]->CalculatePosFromLoverLevels(topLayer->level - layer->level);
			for (auto top : alternativeTopLayers)
				top->nodes()[0]->CalculatePosFromLoverLevels(top->level - layer->level);

			upperIndex = 0;
		}
		else
		{
			topLayer->nodes()[0]->CalculatePosFromLoverLevels(topLayer->level - layer->level);
			upperIndex = 0;
		}


		//topLayer->nodes()[0]->CalculatePosFromLoverLevels(topLayer->level - layer->level);
		//for (auto top : alternativeTopLayers)
		//	top->nodes()[0]->CalculatePosFromLoverLevels(top->level - layer->level);
	};

protected:


	void BuildAlternativeUpperLevels(int aproxForceAlternativeUpperLevelsCount)
	{
		cout << " build alternative layers \n";
		alternativeTopLayers.clear();
		for_each(alternativeLayers.begin(), alternativeLayers.end(), [](Layer* al)
			//concurrency::parallel_for_each(alternativeLayers.begin(), alternativeLayers.end(), [](Layer* al)		
		{
			delete al;
		});
		alternativeLayers.clear();

		//for (auto e : layer->edges())
		//	e->upperLevel().resize(aproxForceAlternativeUpperLevelsCount + 1);
		layer->upper().resize(aproxForceAlternativeUpperLevelsCount + 1);
		if (true)
		{

			concurrency::critical_section lock;
			alternativeTopLayers.resize(aproxForceAlternativeUpperLevelsCount);
			//Concurrency::parallel_for(0, aproxForceAlternativeUpperLevelsCount, [&](int i)
			for (int i = 0; i < aproxForceAlternativeUpperLevelsCount; i++)
			{
				
				auto upperLayer = new Layer(layer, i + 1, Layer::EdgeRemoval);
				lock.lock();
				alternativeLayers.push_back(upperLayer);
				lock.unlock();
				while (upperLayer->nodes().size() > 1)
				{
					upperLayer = new Layer(upperLayer, 0, Layer::EdgeRemoval);
					lock.lock();
					alternativeLayers.push_back(upperLayer);
					lock.unlock();
				}
				PrepareLayer(upperLayer, false);
				alternativeTopLayers[i] = upperLayer;			

				
			}//);//, concurrency::static_partitioner());

			for (auto top : alternativeTopLayers)
			{
				for (auto l = top; l != NULL; l = l->lower())
				{
					//l->validate();
				}
			}
		}
		else
		{
			for (int i = 0; i < aproxForceAlternativeUpperLevelsCount; i++)
			{
				auto upperLayer = new Layer(layer, i + 1, Layer::EdgeRemoval);
				alternativeLayers.push_back(upperLayer);
				while (upperLayer->nodes().size() > 1)
				{
					upperLayer = new Layer(upperLayer, 0, Layer::EdgeRemoval);
					alternativeLayers.push_back(upperLayer);
				}
				PrepareLayer(upperLayer, false);
				alternativeTopLayers.push_back(upperLayer);
			}
		}
	}

	// assumes needed lower layers are prepared
	void PrepareLayer(Layer* layer, bool buildBase)
	{
		//cout << "  prepare layer " << layer->level << "\n";
		if (this->layer == layer)
		{
			if (buildBase)
			{
				if (!useAproxForcePersistentNodeProperties)
				{
					concurrency::parallel_for_each(layer->nodes().begin(), layer->nodes().end(), [](NodePtr node){
						node->graphDiameter = 0;
						node->averageDistance = 0;
						node->weight() = 1;
						});
				}

				if (useAproxForceEdgeTransitions)
				{
					//BuildEdgeTransitionsBase(layer);
				}
				
			}
		}
		else
		{
			PrepareLayer(layer->lower(),buildBase);
			//if (useAproxNodeProperties)
				CalcAproxNodeProperties(layer);			
			if (useAproxForceEdgeTransitions)
			{
				//BuildEdgeTransitionsFromLower(layer);
			}

			for (auto e : layer->edges())
			{
				e->weight() = 0;
				for (auto le : e->lowerLevel())
					e->weight() += le->weight();
			}
		}		
	}

	unsigned  currentRoundMarker[20];
	
	void CalcSumDistanceFromCluster(NodePtr sourceNode, NodePtr filterNode,int threadId, double &sum,double& count)
	{		
		assert(threadId > 0);
		assert(threadId < 10);
		vector<NodePtr > nextList;
		vector<NodePtr > currList;
		nextList.clear();
		currList.clear();
		StartNextRound(threadId);		
		currList.push_back(sourceNode);
		SetNodeStatus(sourceNode, NodeStatusType::Queued, threadId);
		sourceNode->lastAproxDistance[threadId] = sourceNode->averageDistanceInclSelf();

		priority_queue<QueuedNode, vector<QueuedNode>, QueuedNodeComparer> toVisit;
		toVisit.push({ sourceNode, sourceNode->lastAproxDistance[threadId] });
		while (!toVisit.empty())
		{
			auto node = toVisit.top().node;
			toVisit.pop();
			if (GetNodeStatus(node, threadId) == NodeStatusType::Visited)
				continue;
			SetNodeStatus(node, NodeStatusType::Visited, threadId);
			if (node != sourceNode && node->filterNode == filterNode)
			{
				auto clusterDistance = node->lastAproxDistance[threadId];
				auto clustersPairsCombinations = 0.5 *sourceNode->weight()*node->weight();

				sum += clusterDistance * clustersPairsCombinations;
				count += clustersPairsCombinations;
			}

			auto &edges = node->edges();
			auto es = edges.size();
			for (int i = es - 1; i >= 0; i--)
			{
				auto edge = edges[i];
				auto nextNode = node->neightbours()[i];// edge->otherNode(current);
				// allow to step one node outsize of filter
				if (nextNode->filterNode != filterNode && node->filterNode != filterNode)
					continue;
				auto nextStatus = GetNodeStatus(nextNode, threadId);
				if (nextStatus== NodeStatusType::Visited)
					continue;
				auto dist = node->lastAproxDistance[threadId] + 1 + nextNode->averageDistanceInclSelf();
				if (nextStatus == NodeStatusType::Queued)
				{
					if (dist > nextNode->lastAproxDistance[threadId])
						continue;
				}				
				nextNode->lastAproxDistance[threadId] = dist;
				toVisit.push({ nextNode, dist });
				SetNodeStatus(nextNode, NodeStatusType::Queued, threadId);				
			}
			
		}
		return;

		while (!currList.empty())
		{
			for (auto node : currList)
			{
				if (node != sourceNode)
				{
					auto clusterDistance = node->lastAproxDistance[threadId];
					auto clustersPairsCombinations = 0.5 *sourceNode->weight()*node->weight();

					sum += clusterDistance * clustersPairsCombinations;						
					count += clustersPairsCombinations;
				}

				auto &edges = node->edges();
				auto es = edges.size();
				for (int i = es - 1; i >= 0; i--)
				{
					auto edge = edges[i];
					auto nextNode = node->neightbours()[i];// edge->otherNode(current);
					if (GetNodeStatus(nextNode, threadId) == NodeStatusType::Visited)
						continue;
					if (filterNode != NULL && nextNode->filterNode != filterNode)
						continue;
					nextNode->lastAproxDistance[threadId] = node->lastAproxDistance[threadId] + 1 + nextNode->averageDistanceInclSelf();
					SetNodeStatus(nextNode, NodeStatusType::Visited, threadId);
					nextList.push_back(nextNode);
				}
			}
			currList.swap(nextList);
			nextList.clear();			
		}
		
	}



	void CalcAproxNodeProperties(Layer* layer)
	{
		for (auto n : layer->lower()->nodes())
			n->filterNode = nullptr;
		//concurrency::parallel_for_each(layer->nodes().begin(), layer->nodes().end(), [&](Node*node)
		for(auto node : layer->nodes())
		{
			int threadId = concurrency::Context::VirtualProcessorId();
			if (threadId == -1)
				threadId = 9;
			assert(threadId > 0);
			assert(threadId < 10);


		


			//assert(node->lowerLevel().size() <= 2);
			auto lower1 = node->lowerLevel()[0];
			if (node->lowerLevel().size() == 1)
			{
				node->graphDiameter = lower1->graphDiameter;
				node->averageDistance = lower1->averageDistance;
				node->weight() = lower1->weight();
			}
			else
			{


			/*	auto lower2 = node->lowerLevel()[1];
				node->weight() = lower1->weight() + lower2->weight();
				node->graphDiameter = lower1->graphDiameter + 1 + lower2->graphDiameter;
				auto lower1combinations = 0.5*(lower1->weight()*(lower1->weight() - 1));
				auto lower2combinations = 0.5*(lower2->weight()*(lower2->weight() - 1));
				auto lower12combinatinos = lower1->weight()*lower2->weight();
				auto lower12distance = lower1->averageDistance*(lower1->weight() - 1) / lower1->weight()
					+ 1
					+ lower2->averageDistance*(lower2->weight() - 1) / (lower2->weight());
				auto averageDistance =
					(lower1combinations * lower1->averageDistance
					+ lower2combinations * lower2->averageDistance
					+ lower12combinatinos *lower12distance					
					) / (lower1combinations + lower2combinations + lower12combinatinos);
				node->averageDistance = averageDistance;*/



				// any number of lower
				double intraClustersSum = 0;
				double interClustersSum = 0;
				double interPairs = 0;
				double intraPairs = 0;
				int weight = 0;
				for (auto subNode : node->lowerLevel())
				{
					subNode->filterNode = node;
				}
				for (auto subNode : node->lowerLevel())
				{
					weight += subNode->weight();
					CalcSumDistanceFromCluster(subNode, node, threadId, interClustersSum, intraPairs);
					
					auto pairs = 0.5*subNode->weight()*(subNode->weight() - 1);
					intraClustersSum += subNode->averageDistance * pairs;
					interPairs += pairs;
				}
				auto averageDistance = (intraClustersSum + interClustersSum) / (interPairs + intraPairs);
				
				
				node->averageDistance = averageDistance;
				node->weight() = weight;
				/*for (auto subNode : node->lowerLevel())
				{
					subNode->filterNode = nullptr;
				}*/
				//assert(abs(node->averageDistance - averageDistance)<0.00001);
				//assert(node->weight() == weight);
			}
		};
	}

	void CalcExactNodeProperties(NodePtr node, int level)
	{
		if (level == 0)
		{
			node->averageDistance = 0;
			node->graphDiameter = 0;
		}
		else
		{
			int count = 0;
			double sum = 0;
			MarkNodesWithFilter(node, node, level);
			double diamter = 0;
			CalcAverageDistanceFromSubnodes(node, node, level, sum, count,diamter);
			//node->graphDiameter = diamter;
			node->averageDistance = sum / count;
			for (auto lower : node->lowerLevel())
			{
				CalcExactNodeProperties(lower, level - 1);
			}
		}
	}

	void MarkNodesWithFilter(NodePtr node, NodePtr filterNode, int level)
	{
		if (level == 0)

			node->filterNode = filterNode;
		else
			for (auto lower : node->lowerLevel())
				MarkNodesWithFilter(lower, filterNode, level - 1);
	}

	void CalcAverageDistanceFromSubnodes(NodePtr node, NodePtr topNode, int level, double& sum, int& count,double &maxDistance)
	{
		if (level == 0)
		{
			count++;
			sum += CalcAverageDistanceFromNode(node, topNode,maxDistance);
		}
		else
		{
			for (auto lower : node->lowerLevel())
				CalcAverageDistanceFromSubnodes(lower, topNode, level - 1, sum, count, maxDistance);
		}

	}

	double CalcAverageDistanceFromNode(NodePtr sourceNode, NodePtr filterNode,double& maxDistance)
	{
		int threadId = concurrency::Context::VirtualProcessorId();
		if (threadId == -1)
			threadId = 9;
		assert(threadId > 0);
		assert(threadId < 10);
		double sum = 0;
		vector<NodePtr > nextList;
		vector<NodePtr > currList;
		nextList.clear();
		currList.clear();
		StartNextRound(threadId);
		int graphDistance = 0;
		currList.push_back(sourceNode);
		SetNodeStatus(sourceNode, NodeStatusType::Visited, threadId);
		
		int count = 0;
		while (!currList.empty())
		{
			for (auto node : currList)
			{
				if (node != sourceNode)
				{
					if (graphDistance > maxDistance)
						maxDistance = graphDistance;
					sum += graphDistance;
					count++;
				}

				auto &edges = node->edges();
				auto es = edges.size();
				for (int i = es - 1; i >= 0; i--)
				{
					auto edge = edges[i];
					auto nextNode = node->neightbours()[i];// edge->otherNode(current);
					if (GetNodeStatus(nextNode, threadId) == NodeStatusType::Visited)
						continue;
					if (filterNode != NULL && nextNode->filterNode != filterNode)
						continue;
					SetNodeStatus(nextNode, NodeStatusType::Visited, threadId);
					nextList.push_back(nextNode);
				}
			}
			currList.swap(nextList);
			nextList.clear();
			graphDistance++;
		}
		
		if (count > 0)
			return sum / count;
		else
			return 0;
	}

	

	Point CalcForceExact(NodePtr sourceNode, int threadId)
	{
		
		vector<NodePtr > nextList;
		vector<NodePtr > currList;

		Point force = 0;
		Point forceErr = 0;
		
		nextList.clear();
		currList.clear();

		int graphDistance = 0;
		currList.push_back(sourceNode);
//		sourceNode->lastExactDistance = 0;
		StartNextRound( threadId);
		SetNodeStatus(sourceNode, NodeStatusType::Visited, threadId);
		
		while (!currList.empty())
		{
			
			auto floatDist = (double)graphDistance;
			for (auto node : currList)
			{
				if (graphDistance != 0)
				{

					Point dir = node->Pos() - sourceNode->Pos();
					auto dirLen = dir.Lentgh();
					if (dirLen == 0)
						continue;
					auto nodeForce = dir*((1.0 - floatDist / dirLen) / floatDist / floatDist);

					auto nodeForceWithError = nodeForce - forceErr;
					auto newForce = force + nodeForceWithError;
					forceErr = (newForce - force) - nodeForceWithError;
					force = newForce;
					//force += nodeForce;

				}
				
				auto &edges = node->edges();
				auto es = edges.size();
				for (int i = es - 1; i >= 0; i--)
				{
					auto edge = edges[i];
					auto nextNode = node->neightbours()[i];// edge->otherNode(current);
					
					if (GetNodeStatus(nextNode, threadId) == NodeStatusType::Visited)
						continue;

					SetNodeStatus(nextNode, NodeStatusType::Visited, threadId);
					
					nextList.push_back(nextNode);
				}
			}
			currList.swap(nextList);
			nextList.clear();
			graphDistance++;
			if (graphDistance > this->exactForceMaxDistance)
				break;
		}
		
		return force;
	}

	

	//static void BuildEdgeTransitionsBase(Layer* layer)
	//{

	//	/*concurrency::parallel_*/for_each(layer->edges().begin(), layer->edges().end(), [](Edge* edge)
	//	{
	//		edge->clearEdgeTransitions();
	//		for (auto node1Edge : edge->node1()->edges())
	//		{
	//			if (node1Edge != edge)
	//				edge->setTransitionLength(node1Edge, 0);
	//		}

	//		for (auto node2Edge : edge->node2()->edges())
	//		{
	//			if (node2Edge != edge)
	//				edge->setTransitionLength(node2Edge,0);
	//		}
	//		edge->sortEdgeTransitions();
	//	});
	//}

	//-------------------------------------------------------------------------------------------------------------------------------------
	//static void BuildEdgeTransitionsFromLower(Layer* layer)
	//{
	//	
	//	/*concurrency::parallel_*/for_each(layer->edges().begin(), layer->edges().end(), [](Edge* edge)
	//	{
	//		edge->clearEdgeTransitions();
	//		for (auto node1Edge : edge->node1()->edges())
	//		{
	//			if (node1Edge != edge)
	//				edge->setTransitionLength(node1Edge, GetTransitionLength(edge, node1Edge));
	//		}

	//		for (auto node2Edge : edge->node2()->edges())
	//		{
	//			if (node2Edge != edge)
	//				edge->setTransitionLength(node2Edge, GetTransitionLength(edge, node2Edge));
	//		}
	//		edge->sortEdgeTransitions();
	//	});
	//}


	//-------------------------------------------------------------------------------------------------------------------------------------
//	static double GetTransitionLength(Edge* sourceEdge, Edge* targetEdge)
//	{
//		double sumOutgoing = 0;
//
//		NodePtr commonNode = NULL;
//		if (sourceEdge->node1() == targetEdge->node1() || sourceEdge->node1() == targetEdge->node2())
//			commonNode = sourceEdge->node1();
//
//		else
//		{
//			commonNode = sourceEdge->node2();
////			assert(sourceEdge->node2() == targetEdge->node1() || sourceEdge->node2() == targetEdge->node2());
//		}
//
//		Edge* connectiongLowerEdge = commonNode->lowerEdge();
//		double connectiongLowerEdgeTransition = 0;
//
//		double minOutgoing = 1000000;
//		for (auto lowerSourceEdge : sourceEdge->lowerLevel())
//		{
//			Edge* mintargetedge;
//			auto targetMin = GetTransitionLength(lowerSourceEdge, connectiongLowerEdge, targetEdge, mintargetedge);
//			if (targetMin < minOutgoing)
//				minOutgoing = targetMin;
//			sumOutgoing += targetMin;
//		}
//		return  sumOutgoing / sourceEdge->lowerLevel().size();
//	}

	//-------------------------------------------------------------------------------------------------------------------------------------
	//static double GetTransitionLength(Edge* lowerSourceEdge, Edge* connectiongLowerEdge, Edge* targetEdge, Edge*& chosenLowerTargetEdge)
	//{
	//	double targetMin = 100000;
	//	for (auto lowerTargetEdge : targetEdge->lowerLevel())
	//	{
	//		double transitionLength = lowerSourceEdge->edgeTransitionLength(lowerTargetEdge);
	//		if (transitionLength == -1)
	//		{

	//			// no direct transition, go throug connecting lower edge
	//			transitionLength = 0;
	//			transitionLength += lowerSourceEdge->edgeTransitionLength(connectiongLowerEdge);
	//			transitionLength += 1;
	//			transitionLength += connectiongLowerEdge->edgeTransitionLength(lowerTargetEdge);

	//		}
	//		if (transitionLength < targetMin){
	//			targetMin = transitionLength;
	//			chosenLowerTargetEdge = lowerTargetEdge;
	//		}

	//	}
	//	return targetMin;
	//}

	//double NodeMinLastExactDistance(NodePtr node, int level)
	//{
	//	if (level == 0)
	//		return node->lastExactDistance;
	//	else
	//	{
	//		double minDist = 300000;
	//		for (auto n : node->lowerLevel())
	//			minDist = min(minDist, NodeMinLastExactDistance(n, level - 1));
	//		return minDist;
	//	}

	//}

	//double NodeAverageDistanceSum(NodePtr node, int level)
	//{
	//	if (level == 0)
	//		return node->lastExactDistance;
	//	else
	//	{
	//		double sum = 0;
	//		for (auto n : node->lowerLevel())
	//			sum += NodeAverageDistanceSum(n, level - 1);
	//		return sum;
	//	}

	//}

	//double CalcTransiton(Edge* sourceEdge, Edge* targetEdge, vector<NodePtr >::iterator sourceParentBegin, vector<NodePtr >::iterator sourceParentEnd)
	//{
	//	if (sourceParentEnd - sourceParentBegin == 1)
	//		return sourceEdge->edgeTransitionLength(targetEdge);
	//	else
	//	{
	//		sourceParentEnd--;
	//		auto topSourceParent = *(sourceParentEnd - 1);
	//		if (find(topSourceParent->edges().begin(), topSourceParent->edges().end(), targetEdge) == topSourceParent->edges().end())
	//		{
	//			// target edge not connected 

	//		}
	//	}
	//}



	struct ToVisit{
		int m1 = 123456;
		NodePtr node;
		int m2 = 543221;		
		ToVisit() = default;
		ToVisit(NodePtr n){ node = n; }
		bool operator==(const ToVisit& other){
			return other.node == node;
		}
	};

	vector<vector<NodePtr >> toVisitCombinable;

	vector<NodePtr> toVisitBuffers[10];

#ifndef dd

	Point CalcForceApprox(NodePtr sourceNode, int threadId)
	{
		switch (threadId)
		{
			case 0:return CalcForceApprox<0>(sourceNode);
			case 1:return CalcForceApprox<1>(sourceNode);
			case 2:return CalcForceApprox<2>(sourceNode);
			case 3:return CalcForceApprox<3>(sourceNode);
			case 4:return CalcForceApprox<4>(sourceNode);
			case 5:return CalcForceApprox<5>(sourceNode);
			case 6:return CalcForceApprox<6>(sourceNode);
			case 7:return CalcForceApprox<7>(sourceNode);
			case 8:return CalcForceApprox<8>(sourceNode);
			case 9:return CalcForceApprox<9>(sourceNode);
		}
	}

	template<unsigned threadId>
	Point CalcForceApprox(NodePtr sourceNode)
	{
		
		switch (upperIndex){
			case 0:return CalcForceApprox<threadId,0>(sourceNode);
			case 1:return CalcForceApprox<threadId,1>(sourceNode);
			case 2:return CalcForceApprox<threadId,2>(sourceNode);
			case 3:return CalcForceApprox<threadId,3>(sourceNode);
			case 4:return CalcForceApprox<threadId,4>(sourceNode);
			case 5:return CalcForceApprox<threadId,5>(sourceNode);
			case 6:return CalcForceApprox<threadId,6>(sourceNode);
			case 7:return CalcForceApprox<threadId,7>(sourceNode);
			case 8:return CalcForceApprox<threadId,8>(sourceNode);
			case 9:return CalcForceApprox<threadId,9>(sourceNode);
		}
	}
	
	struct QueuedNode
	{
		Node* node;
		double prioritity;

		bool operator<(const QueuedNode& other)
		{
			return prioritity > other.prioritity;
		}
	};

	struct QueuedNodeComparer{
		bool operator()(const QueuedNode& n1, const QueuedNode& n2)
		{
			return n2.prioritity < n1.prioritity;
		}

	};

	void DrawStage(Node* n,Node* startNode,Point& force)
	{
		auto model = this;
		auto extent = model->layer->CalcExtent();
		Image image(extent, Point(800, 800), model->layer->edges().size(), false);
		int margin = 7;


		image.SetPenWidth(1);
		image.SetPenColor(100, 100, 100);
		for (auto e : layer->edges())
		{
			image.DrawLine(e->node1()->Pos(), e->node2()->Pos());
		}


		image.SetFillColor(255, 255, 255, 255);
		for (auto n : layer->nodes())
		{
			image.DrawBoundingBox(n, margin, 0);
		}

		image.SetPenColor(0, 0, 0);		
		image.SetPenWidth(4);

		for (auto l = topLayer; l != nullptr; l = l->lower())				
			for (auto node : l->nodes())
			{
				auto status = model->GetNodeStatus(node, 0);				
				if (status == SpringMethod::NodeStatusType::Visited || status == Queued)
				{
					if (node->lastIncomingEdge[0] != NULL)
					{						
						image.DrawLine(node->lastIncomingEdge[0]->node1()->Pos(), node->lastIncomingEdge[0]->node2()->Pos());
					}
				}
			}
		image.SetPenColor(0, 0, 0);
		image.SetPenWidth(2);
		for (auto l = topLayer; l != nullptr; l = l->lower())
			for (auto node : l->nodes())
			{
				auto status = model->GetNodeStatus(node, 0);
				image.SetPenWidth(2);
				if (status == SpringMethod::NodeStatusType::Visited)
				{
					
					image.SetFillColor(255, 255, 255, 255);
					if (node == n)
						image.SetFillColor(255, 255, 200, 200);
					image.DrawBoundingBox(node, margin * 4, 0);
					image.DrawString(node->Pos() + Point(-0.38, +0.25), L"%0.1f ", node->lastAproxDistance[0]);
				}
				else if (status == Queued)
				{
					image.SetFillColor(255, 255, 255, 200);
					image.DrawBoundingBox(node, margin*4, 0);
					image.DrawString(node->Pos() + Point(-0.38,+0.25), L"%0.1f ", node->lastAproxDistance[0]);
				}				
			}
		
		


		//deque<NodePtr > nodeToDraw;
		//nodeToDraw.push_back(model->topLayer->nodes()[0]);
		//while (!nodeToDraw.empty())
		//{
		//	auto node = nodeToDraw.front();
		//	nodeToDraw.pop_front();
		//	for (auto n : node->lowerLevel())
		//		nodeToDraw.push_back(n);
		//	auto status = model->GetNodeStatus(node, 0);
		//	if (node->upperLevel()[0] != nullptr && model->GetNodeStatus(node->upperLevel()[0], 0) == SpringMethod::NodeStatusType::Visited)
		//		status = (NodeStatusType)20;

		//	if (status == SpringMethod::NodeStatusType::Visited||status==Queued)
		//	{

		//		image.DrawString(node->Pos(), L"(%d) %0.0f ",node->index(), node->lastAproxDistance[0]+ (node->averageDistance*node->weight()-1)/node->weight());
		//		image.SetPenColor(0, 0, 0);
		//		image.SetFillColor(50, 0, 0, 255);
		//		if (status == Queued)
		//		{
		//			image.SetFillColor(50, 255, 255, 0);
		//		}
		//		if (node == n)
		//		{
		//			image.SetFillColor(50, 255, 0, 0);
		//		}

		//		image.DrawBoundingBox(node, node->level*margin + margin, 0);					
		//	}
		//	else if (status == SpringMethod::NodeStatusType::Subdivided)
		//	{
		//		image.SetPenColor(0, 0, 0);
		//		image.SetFillColor(255, 255, 255, 0);
		//		//image.DrawBoundingBox(node, node->level*margin + margin, 0);
		//	}
		//	else
		//	{
		//		image.SetPenColor(0, 0, 0);
		//		image.SetFillColor(255, 255, 255, 255);
		//		//image.DrawBoundingBox(node, node->level*margin + margin, 0);

		//	}

		//}
		

		//image.DrawLine(startNode->Pos(), startNode->Pos() - force);
		//image.DrawString({ 0, 0 }, L"(%0.2f %0.2)", force.X(), force.Y());
		wchar_t buff[244];
		wsprintf(buff, L"force calc details %d_%d.png",n->level, n->index());
		image.Save(buff);
	}
	
	template<unsigned threadId,unsigned  upperIndex>
	Point CalcForceApprox(NodePtr sourceNode)
#else
	Point CalcForceApprox(NodePtr sourceNode, int threadId)
#endif
	{
		Point force = 0;
		Point forceErr = 0;
				
		toVisitBuffers[threadId].clear();
		//priority_queue<NodePtr, vector<NodePtr>, ToVisitComparer> toVisit(ToVisitComparer{ threadId });// , toVisitBuffers[threadId]);
		priority_queue<QueuedNode, vector<QueuedNode>, QueuedNodeComparer> toVisit;
		QueuedNode q1, q2;
		auto d = q1 < q2;

		StartNextRound(threadId);

		auto markerSubdivided = GetStatusMarker(NodeStatusType::Subdivided,threadId);
		auto markerVisited = GetStatusMarker(NodeStatusType::Visited, threadId);
		auto markerQueued = GetStatusMarker(NodeStatusType::Queued, threadId);
		auto currentMarker = currentRoundMarker[threadId];


		auto parent = sourceNode->upperLevel()[upperIndex];
		while (parent != NULL)
		{
  			SetNodeStatus(parent, NodeStatusType::Subdivided, threadId);

			parent = parent->upperLevel()[0];
		}
		
		sourceNode->lastAproxDistance[threadId] = 0;
		sourceNode->lastIncomingEdge[threadId] = 0;
			
		SetNodeStatus(sourceNode, NodeStatusType::Queued, threadId);
		toVisit.push({ sourceNode, sourceNode->lastAproxDistance[threadId] });
		int currNum = 0;
		while (!toVisit.empty() && currNum < toVisit.size())
		{
			auto topItem = toVisit.top();
			auto current = topItem.node;
			toVisit.pop();
			if (GetNodeStatus(current, threadId) != NodeStatusType::Queued)
				continue;
			
			assert(current->magic == 123456);

			
			SetNodeStatus(current, NodeStatusType::Visited, threadId);
			auto currentDistance = current->lastAproxDistance[threadId];
			//currentDistance += current->averageDistance*(current->weight() - 1) / current->weight();
			if (current->lastAproxDistance[threadId] != 0)
			{
				Point dir = current->Pos() - sourceNode->Pos();
				auto dirLen = dir.Lentgh();
				if (dirLen == 0)
					continue;


				auto nodeForce = dir* ((1.0 - currentDistance / dirLen)*current->weight() / currentDistance / currentDistance);
				assert(!isnan(nodeForce.X()) && !isnan(nodeForce.Y()));
				force += nodeForce;
				assert(!isnan(force.X()) && !isnan(force.Y()));

			}
			auto currentUpperIndex = upperIndex;
			if (current->level > sourceNode->level)
				currentUpperIndex = 0;			
			auto &edges = current->edges();
			auto es = edges.size();			
			for (int i = es - 1; i >= 0; i--)
			{
				auto edge = edges[i];
				auto nextNode = current->neightbours()[i];// edge->otherNode(current);				
				assert(current->magic == 123456);
				assert(edge->otherNode(current) == nextNode);
				assert(nextNode->magic == 123456);
				assert(nextNode->level == current->level);

				auto nextNodeStatusMarker = nextNode->statusMarker[threadId];

				NodePtr queuedParent = NULL;
				bool checkParent = true;
				bool parentIsVisited = false;


				if (nextNodeStatusMarker == markerSubdivided)
					continue;// skip this whole edge
				else if (nextNodeStatusMarker == markerVisited)
					continue;
				else if (nextNodeStatusMarker == markerQueued)
				{
					queuedParent = nextNode;
					checkParent = false;
				}
				else if (nextNodeStatusMarker == markerSubdivided)
				{
					checkParent = false;
				}

				// check what's above
				if (checkParent)
					for (auto parent = nextNode->upperLevel()[currentUpperIndex]; parent != NULL; parent = parent->upperLevel()[0])
					{
						assert(parent->magic == 123456);
						auto parentStatusMarker = parent->statusMarker[threadId] - currentMarker;
						if (parentStatusMarker == NodeStatusType::Visited)
						{
							parentIsVisited = true;
							nextNode->statusMarker[threadId] = markerVisited;
							for (auto npparent = nextNode->upperLevel()[currentUpperIndex]; npparent != parent; npparent = npparent->upperLevel()[0])
							{
								npparent->statusMarker[threadId] = markerVisited;
							}
							break;
						}
						else if (parentStatusMarker == NodeStatusType::Queued)
						{
							queuedParent = parent;
							break;
						}
						else if (parentStatusMarker == NodeStatusType::Subdivided)
							break;
					}

				if (parentIsVisited)
					continue;

				// go to upper level
				do
				{

					auto parent = nextNode->upperLevel()[nextNode->level == sourceNode->level ? upperIndex : 0];
					if (parent == NULL)
						break;
					assert(parent->magic == 123456);
					assert(parent->level == nextNode->level + 1);

					if (GetNodeStatus(parent, threadId) == NodeStatusType::Subdivided)
						break;

					auto goemDistanceSqr = parent->Pos().DistanceSqr(sourceNode->Pos());
					auto  boundDiam = parent->boundDiamSqr;
					if (parent->boundDiamSqr > useAproxForceSkipLevelThreashold* goemDistanceSqr)
						break;
					if (parent->averageDistance > currentDistance+1)
							break;

					//edge = edge->upperLevel()[nextNode->level == sourceNode->level ? upperIndex : 0];

					nextNode = parent;

				} while (useAproxForceSkipManyLevels);// nextNode->upperLevel()[0]->lowerLevel().size() == 1);
				assert(!nextNode->deleted);


				auto nextDistance = current->lastAproxDistance[threadId] + 1;
				nextDistance+= nextNode->averageDistance*(nextNode->weight() - 1) / nextNode->weight();
				//nextDistance += current->averageDistance*(current->weight() - 1) / current->weight();
				

				if (queuedParent != nullptr)
				{
					if (queuedParent->lastAproxDistance[threadId] > nextDistance)
					{
						//toVisit.erase(find(toVisit.begin(), toVisit.end(), queuedParent));

					}
					else
					{
						continue;
					}
				}

				// Store and queue
				nextNode->lastAproxDistance[threadId] = nextDistance;
				nextNode->lastIncomingEdge[threadId] = edge;

				/*	auto exactAverageDistance = NodeAverageDistanceSum(nextNode, nextNode->level - sourceNode->level) / nextNode->weight();
					auto exactMinDistance = NodeMinLastExactDistance(nextNode, nextNode->level - sourceNode->level);
					nextNode->lastExactDistance = exactMinDistance;
					*/

				auto parent = nextNode->upperLevel()[nextNode->level == sourceNode->level ? upperIndex : 0];
				while (parent != NULL)
				{
					if (GetNodeStatus(parent, threadId) == NodeStatusType::Subdivided)
					{
						break;
					}
					SetNodeStatus(parent, NodeStatusType::Subdivided, threadId);
					parent = parent->upperLevel()[parent->level == sourceNode->level ? upperIndex : 0];
				}

				SetNodeStatus(nextNode, NodeStatusType::Queued, threadId);

				/*auto insertPoint = lower_bound(toVisit.begin()+currNum, toVisit.end(), nextNode, [&](NodePtr n1, NodePtr n2)
				{
				return n1->lastAproxDistance[threadId] < n2->lastAproxDistance[threadId];
				});
				*/
				assert(nextNode->level >= sourceNode->level);
				assert(nextNode->level <= 30);
				assert(nextNode->magic == 123456);
				assert(!nextNode->deleted);
				//	toVisit.insert(insertPoint, nextNode);
				toVisit.push({ nextNode, nextDistance });

			}
			if (foreceLayout->isDebugRun){
				DrawStage(current, sourceNode, force);
			}
		}
		assert(!isnan(force.X()) && !isnan(force.Y()));
		return force;
	}
	public:
	double CalcGraphDiam()
	{

		concurrency::combinable<double> maxDiam;
		maxDiam.clear();
		//concurrency::parallel_for_each(layer->nodes().begin(), layer->nodes().end(), [&](NodePtr node)
		for_each(layer->nodes().begin(), layer->nodes().end(), [&](NodePtr node)
		{
			double nodeMaxDiam;
			CalcAverageDistanceFromNode(node, nullptr, nodeMaxDiam);
			maxDiam.local() = max(maxDiam.local(),nodeMaxDiam);

		});
		return maxDiam.combine([](double a, double b){return max(a, b); });

	}


	unsigned CalcDiameterFast()
	{
		vector<NodePtr > fringe;
		//first double sweep from random point to find a central point
		
		auto radiusUpperBound = CalcMaxDistance(layer->nodes()[0], numeric_limits<unsigned>::max(), fringe, 0);
		auto n1 = fringe[0];
		auto diamLowerBound = CalcMaxDistance(fringe[0], numeric_limits<unsigned>::max(), fringe, 0);
		auto n2 = fringe[0];
		cout << "  min boud " << diamLowerBound << "\n";
		if (radiusUpperBound * 2 <= diamLowerBound)
			return diamLowerBound;
		//find center, moving along shortest patch
		auto center1 = FindPathCenter(n2);

		// second double sweep from found central point to find a better one
		radiusUpperBound = CalcMaxDistance(center1, numeric_limits<unsigned>::max(), fringe, 0);
		auto n3 = fringe[0];
		diamLowerBound = CalcMaxDistance(fringe[0], numeric_limits<unsigned>::max(), fringe, 0);
		auto n4 = fringe[0];
		cout << "  min boud " << diamLowerBound << "\n";
		if (radiusUpperBound * 2 <= diamLowerBound)
			return diamLowerBound;
		//find center, moving along shortest patch
		auto center2 = FindPathCenter(n4);


		// third sweep
		radiusUpperBound = CalcMaxDistance(center2, numeric_limits<unsigned>::max(), fringe, 0);
		auto n5 = fringe[0];
		diamLowerBound = CalcMaxDistance(fringe[0], numeric_limits<unsigned>::max(), fringe, 0);
		auto n6 = fringe[0];
		cout << "  min boud " << diamLowerBound << "\n";
		if (radiusUpperBound * 2 <= diamLowerBound)
			return diamLowerBound;
		//find center, moving along shortest patch
		center2 = FindPathCenter(n6);


		// magic :)
		
		radiusUpperBound = CalcMaxDistance(center2, numeric_limits<unsigned>::max(), fringe, 0);
		
		while (radiusUpperBound * 2 > diamLowerBound)
		{
			cout << "   diam boud " << diamLowerBound << " " << radiusUpperBound * 2 << "\n";
			unsigned dists[10];
			for (int i = 0; i < 10; i++)
				dists[i] =0;
			
			concurrency::parallel_for_each(fringe.begin(), fringe.end(), [&](NodePtr n)
			//for (auto n : fringe)
			{
				int threadId = concurrency::Context::VirtualProcessorId();
				if (threadId == -1)
					threadId = 9;
				vector<NodePtr > dummyFringe;
				auto dist = CalcMaxDistance(n, numeric_limits<unsigned>::max(), dummyFringe, threadId);
				dists[threadId] = dist;				
			});
			for (int i = 0; i < 10; i++)
			{
				auto dist = dists[i];								
				diamLowerBound = max(diamLowerBound, dist);
				if (radiusUpperBound * 2 <= diamLowerBound)
					return diamLowerBound;
				
			}
			radiusUpperBound -= 1;
			if (radiusUpperBound * 2 <= diamLowerBound)
				return diamLowerBound;
			CalcMaxDistance(center2, radiusUpperBound, fringe, 0);
		}
		
		return diamLowerBound;
		

	}

	NodePtr FindPathCenter(NodePtr lastNode)
	{
		unsigned len = (unsigned)lastNode->lastAproxDistance[0];
		auto node = lastNode;
		for (unsigned i = 0; i < len / 2; i++)
		{
			auto nextNode = node;
			for (auto e : node->edges())
			{
				auto nextNodeCandidate = e->otherNode(node);
				if (nextNodeCandidate->lastAproxDistance[0] < nextNode->lastAproxDistance[0])
					nextNode = nextNodeCandidate;
			}
			node = nextNode;
		}
		return node;
	}

	unsigned CalcMaxDistance(NodePtr sourceNode, unsigned  limitDistance,vector<NodePtr > & fringe,unsigned threadId)
	{
		/*int threadId = concurrency::Context::VirtualProcessorId();
		if (threadId == -1)
			threadId = 9;
*/
		
		vector<NodePtr > nextList;
		vector<NodePtr > currList;
		nextList.clear();
		currList.clear();
		StartNextRound(threadId);
		
		unsigned distance=0;
		currList.push_back(sourceNode);
		SetNodeStatus(sourceNode, NodeStatusType::Visited, threadId);	
		unsigned  visitedMarker = GetStatusMarker(NodeStatusType::Visited, threadId);		
		while (!currList.empty())
		{
			
			for (auto node : currList)
			{				
				node->lastAproxDistance[threadId] = distance;
				
				auto &edges = node->edges();
				auto es = edges.size();
				const NodePtr* nextNode = node->neightbours().data();
				for (int i = es - 1; i >= 0; i--)
				{		
					if ((*nextNode)->statusMarker[threadId]==visitedMarker)
					
					{
						++nextNode;
						continue;
					}
					(*nextNode)->statusMarker[threadId] = visitedMarker;
					//SetNodeStatus(*nextNode, NodeStatusType::Visited, threadId);
					nextList.push_back(*nextNode);
					++nextNode;
				}
			}
			fringe.swap(currList);
			currList.swap(nextList);
			nextList.clear();			
			distance++;
			if (limitDistance == distance)
				break;
		}
		return distance-1;
		
	}
};