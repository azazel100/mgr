#pragma once
#pragma once
#include "../structure/layer.h"
#include "../utils/Image.h"
#include "../utils/Stopwatch.h"



class ostream_combined
{
	vector<ostream*> m_streams;
public:
	void Add(ostream& stream)
	{
		m_streams.push_back(&stream);
	}

	template<typename OP>
	ostream_combined& operator <<(const OP& arg)
	{
		for (auto s : m_streams)
		{
			*s << arg;
			s->flush();
		}
		return *this;
	}
};



class ForceLayout;

class IForceMethod
{
public:	
	Layer* layer;	
	Layer* topLayer;
	ForceLayout* foreceLayout;
	
	virtual Point CalcForce(NodePtr node,int threadId)=0;
	
	virtual void InitExecution() = 0;
	virtual double CalculateScaling() = 0;
	virtual void InitLayer()=0;	
	//virtual void InitLayerPositions()=0;
	virtual void InitIteration(int number)=0;

	virtual wstring Describe() = 0;
};




class ForceLayout
{

	std::vector<Layer*> m_layers;
	wstring name;
	
public:
	Rect m_lastExtent;
	wstring outputDir;
	bool writeStats=true;
	ForceLayout(void);
	~ForceLayout(void);

		
	IForceMethod* method;
	bool drawStages = false;
	int  drawFinalSize = 3000;
	int  drawStagesSize = 500;
	bool drawMovie = false;
	int  drawMovieSize = 500;
	bool isDebugRun = false;

	void Load(const wstring filename,const wstring positions=L"");
	//void Load(Graph& graph, GraphAttributes&);
	void LoadQuadMesh(int level)
	{
		name = L"Quad";
		m_layers.push_back(Layer::CreateQuad(level,isDebugRun));
	}		
	
	void TestSpringApprox();
	
	void Execute(){
		Init();
		// preparations
		totalTime.Start();

		FindLargestConnectedSubgraph(m_layers.front());

		cout << "Laying out graph with " << m_layers.front()->nodes().size() << " nodes and " << m_layers.front()->edges().size() << " edges" << endl;

		CreateLayers();

		out << "Execution" << "\n";
		out << " init.." << "\n";
		method->topLayer = m_layers.back();
		method->layer = m_layers.front();
		method->InitExecution();


		// layout first
		layer = m_layers[m_layers.size() - 3];
		InitLayerPositionRandom();
		avgLen = layer->CalcAverageEdgeLength();
		auto initialStep = avgLen/2;
		auto finalStep = avgLen / 100;
		LayoutLayer(initialStep, finalStep, true, [&](double step)
		{
			avgLen = layer->CalcAverageEdgeLength();
			if (step >= avgLen)
				return  avgLen;
			if (step < avgLen / 10)
				return 0.0;
			return step;

		});
		layer = layer->lower();

		//layout lower
		while (layer != nullptr)
		{
			method->layer = layer;
			InitLayerPositionsFromUpper();

			auto avgLen = layer->CalcAverageEdgeLength();

			auto extent = layer->CalcExtent();
			auto initialStep = avgLen*0.5;
			if (initialStep > extent.width() / 10)
			{
				out << "initial step was to big setting from" << initialStep << " to " << extent.width() / 10 << "\n";
				initialStep = extent.width() / 10;
				
			}

			auto finalStep = initialStep / 10;
			if (layer->level == 0)
				finalStep = initialStep / 100;
			LayoutLayer(initialStep, finalStep, false, [&](double step){return step; });

			
			layer = layer->lower();
		}

		totalTime.Stop();
		out << "Total time: " << totalTime.Seconds();

		{
			wchar_t buff[500];
			auto imageExtent = m_layers.front()->CalcExtent();
			wsprintf(buff, L"%s\\%s.png", outputDir.c_str(), outName.c_str());
			m_layers.front()->Draw(imageExtent, { drawFinalSize, drawFinalSize }, buff, true);
		}
		avgLen = m_layers.front()->CalcAverageEdgeLength();
		double avforce, energy;
		CalcForces(energy, avforce);
		out << " final energy: " << energy / avgLen / avgLen << " average force is: " << avforce << "\n";


	}


private:
	std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t > wstringConverter;
	Stopwatch totalTime;
	wstring methodName;
	wstring graphName;
	wstring outName;
	wstring detailsDir;

	ofstream outputFile;
	ostream_combined out;
	
	fstream levelStatFile, graphStatFile;

	Layer* layer;
	int iteration;
	double avgLen;
	int totalFrameNumber;

	string ToShort(wstring wide)
	{
		return wstringConverter.to_bytes(wide);
	}


	void FindLargestConnectedSubgraph(Layer* layer)
	{
		out << "Searching for largest connected component:\n";
		vector<pair<Node*, int>> components;
		int largetsSize = 0;
		Node* largestStart = nullptr;


		for (auto n: layer->nodes())
		{
			n->statusMarker[0] = 0;
		}

		for (auto n : layer->nodes())
		{
			
			if (n->statusMarker[0] == 0)
			{
				auto size = FindConnectedSizeAndMark(n, n);
				components.push_back({ n, size });
				if (size> largetsSize)
				{
					largetsSize = size;
					largestStart = n;
				}
				out << "  found connected with size " << size << "\n";
			}
		}
		
		for (auto n:layer->nodes())
		{
			if (n->filterNode != largestStart)
				n->deleted = true;
		}

		for (auto e : layer->edges())
		{
			if (e->node1()->filterNode != largestStart)
				e->deleted = true;
		}

		layer->RemoveDeadElements();


	}

	int FindConnectedSizeAndMark(NodePtr sourceNode, Node* marker)
	{	
		vector<NodePtr > nextList;
		vector<NodePtr > currList;
		nextList.clear();
		currList.clear();		
		int graphDistance = 0;
		currList.push_back(sourceNode);
		enum  NodeStatusType:unsigned {Unset, Visited };

		sourceNode->statusMarker[0] = NodeStatusType::Visited;

		int count = 0;
		while (!currList.empty())
		{
			for (auto node : currList)
			{
				count++;
				node->filterNode = marker;

				auto &edges = node->edges();
				auto es = edges.size();
				for (int i = es - 1; i >= 0; i--)
				{
					auto edge = edges[i];
					auto nextNode = edge->otherNode(node);
					if (nextNode->statusMarker[0] ==  NodeStatusType::Visited)
						continue;
					
					nextNode->statusMarker[0] = NodeStatusType::Visited;
					nextList.push_back(nextNode);
				}
			}
			currList.swap(nextList);
			nextList.clear();
			graphDistance++;
		}

		return count;
	}

	void Init()
	{
		totalFrameNumber = 0;
		methodName = method->Describe();
		outName = name + L"- " + methodName;
		detailsDir = outputDir + L"\\" + outName;		
		
		wstring cmd = L"rmdir /S /Q \"" + detailsDir + L"\"";
		system(ToShort(cmd).c_str());

		cmd = L"mkdir \"" + detailsDir + L"\"";
		system(ToShort(cmd).c_str());

		outputFile.open(detailsDir + L"\\log.log",ios::trunc);
		out.Add(cout);
		out.Add(outputFile);
	
		levelStatFile.open(outputDir + L"\\levelStats.csv", ios::app | ios_base::in | ios_base::out | ios_base::ate);
		if (!levelStatFile.is_open())
			throw exception();
		if ((int)levelStatFile.tellp() == 0)
		{
			levelStatFile << "method;graph;level;node count;edge count;seconds init;seconds per iteration" << endl;
		}

		graphStatFile.open(outputDir + L"\\graphStats.csv", ios::app | ios_base::in | ios_base::out | ios_base::ate);
		if ((int)graphStatFile.tellp() == 0)
		{
			graphStatFile << "method;graph;node count;edge count;coarsed node count;seconds" << endl;
		}
		
		srand(0);
		method->foreceLayout = this;
	

	}

	void CreateLayers()
	{	
		auto layer = m_layers.back();
		
		cout << "Creating layers" << endl;

		m_layers.resize(1);
		while (m_layers.back()->nodes().size() > 1)
		{
			auto newlayer = new Layer(m_layers.back(), 0, Layer::EdgeRemoval);			
			layer = newlayer;
			
			if (layer->nodes().size() == m_layers.back()->nodes().size())
				break;
			m_layers.push_back(layer);			
			
			double sum = accumulate(layer->nodes().begin(), layer->nodes().end()
				,0
				, [](int sum, NodePtr node){return sum + node->weight(); }
			);
			double avg = sum / layer->nodes().size();
			int devSum = accumulate(layer->nodes().begin(), layer->nodes().end()
				, 0
				, [&](int sum, NodePtr node){return  (node->weight() - avg)*(node->weight() - avg); }
			);
			double dev = sqrt(devSum / layer->nodes().size());

			cout << "Create layer with " << layer->nodes().size() << " nodes and " << layer->edges().size()<< "edges. Avg node weight is "<< avg << " and stdDev is " << dev<< endl;
		}		
		cout << "Created " << m_layers.size() << " layers." << endl;		
	}			

	int progress = 0;
	double CalcAdaptiveStep(double step, double prevEnergy,double newEnergy,int &progress)
	{
		if (newEnergy > prevEnergy*0.999)
		{
			progress = 0;
			step = step*0.9;
		}
		else
		{
			progress++;
			if (progress > 5)
			{
				step = step / 0.9;
				progress = 0;
			}
		}
		return step;
	}

	void InitLayerPositionRandom()
	{
		for (auto n : layer->nodes())
		{
			n->Pos().X() = rand() % 100;
			n->Pos().Y() = rand() % 100;
			n->speed = n->Pos();
		}
	}

	void InitLayerPositionsFromUpper()
	{
		double scaleFactor =  method->CalculateScaling();
		avgLen = layer->upper()[0]->CalcAverageEdgeLength()*scaleFactor;
		double separationDistance = avgLen / 10000;
		for (auto node  : layer->upper()[0]->nodes())
		{
			if (node->lowerLevel().size() == 1)
			{
				node->lowerLevel()[0]->Pos() = node->Pos()*scaleFactor;
				node->lowerLevel()[0]->speed = 0;
			}
			else
			{
				auto angle = rand();
				for (auto n: node->lowerLevel())
				{
					Point offset;
					angle += 2 * 3.14 / node->lowerLevel().size();
					offset.X() = sin(angle)*separationDistance;
					offset.Y() = cos(angle)*separationDistance;
					n->speed = offset;					
					n->Pos() = node->Pos()*scaleFactor + n->speed;					
				}

			} 
			/*if (node->lowerLevel().size() == 2)
			{
				auto n1 = node->lowerLevel()[0];
				auto n2 = node->lowerLevel()[1];				
				Point offset;
				auto angle = rand();
				offset.X() = sin(angle)*separationDistance;
				offset.Y() = cos(angle)*separationDistance;
				n1->speed = offset;
				n2->speed = offset*(-1);
				n1->Pos() = node->Pos()*scaleFactor + n1->speed;
				n2->Pos() = node->Pos()*scaleFactor + n2->speed;
			}*/
		}

		auto initialLayerExtent = layer->CalcExtent();

		//Animate perturbations
		iteration = 0;
		DrawMovieFrame(initialLayerExtent, initialLayerExtent);
		DrawIteration( initialLayerExtent );
	}


	double minimizeSA(double x, double s, function<double(double)> func)
	{
		double y, xp, yp;


		x = x;
		y = func(x);
		double dir = 1;

		for (int it = 0; it < 40; it++)
		{

			dir = y < yp ? dir : -dir;
			xp = x;
			yp = y;

			s = s*0.98;
			x = x + s*dir;
			y = func(x);			
		}
		return x;
	}

	void LayoutLayer(double initialStep, double finalStep,bool adaptiveStep,function<double(double)> afterIter)
	{
		out << "Layout layer " << layer->level << "\n";
		out << " nodes(): " << layer->nodes().size() << " edges: " << layer->edges().size() << "\n";
		out << " init.." << "\n";

		Stopwatch iterateWatch, initWatch;
		for (auto n : layer->nodes())
		{
			n->force = 0;
			n->speed = 0;
			n->avgForce = -100;
		}		
		initWatch.Start();
		method->layer = layer;
		method->InitLayer();
		initWatch.Stop();
		

		/*out << " find optimal scale..." << "\n";
		method->InitIteration(iteration);
		double scalePow = minimizeSA(0.9, 0.5, [&](double scalePow)
		{
			double scale = pow(2, scalePow);
			concurrency::parallel_for_each(layer->nodes().begin(), layer->nodes().end(), [&](Node*n)
			{
				n->Pos() = n->Pos()*scale;
			});
			double energy, a;
			method->InitIteration(iteration);
			CalcForces(energy, a);
			concurrency::parallel_for_each(layer->nodes().begin(), layer->nodes().end(), [&](Node*n)
			{
				n->Pos() = n->Pos()/scale;
			});
			out << "  scale: " << scale << " energy: " << energy << "\n";
			return a;
		});

		double scale = pow(2, scalePow);
		concurrency::parallel_for_each(layer->nodes().begin(), layer->nodes().end(), [&](Node*n)
		{
			n->Pos() = n->Pos()*scale;
		});*/

		out << " iterate..." << "\n";		
		iterateWatch.Start();
		double step = initialStep;		
		double prevEnergy = numeric_limits<double>::max();
		iteration = 1;
		auto initialExtent = layer->CalcExtent();
		auto extentBefore = initialExtent;		
		while (step > finalStep)
		{
			totalTime.Tick();			
			out << " it: " << iteration << " time: " << totalTime.Seconds() << " layout..";
			// calc and move			
			double newEnergy, newAvForce;
			method->InitIteration(iteration);
			CalcForces(newEnergy, newAvForce);
			MoveNodes(step);
			
			out << " energy: " << newEnergy;

			// log and draw
			auto extentAfter = layer->CalcExtent();
			//out << "  it: " << iteration;
			/*out << " avglen" << std::fixed << avgLen;
			out << " energy: " << std::fixed << newEnergy;
			out << " step: " << step;
			out << " avforce " << newAvForce;*/
			
			
			DrawIteration(initialExtent);
			DrawMovieFrame(extentBefore, extentAfter);						

			out << "\n";

			// change step
			if (adaptiveStep)
				step = CalcAdaptiveStep(step, prevEnergy, newEnergy, progress);
			else
				step = step*0.9;

			step = afterIter(step);
			if (step == 0)
				break;
			extentBefore = extentAfter;
			prevEnergy = newEnergy;
			iteration++;
		}
		//log
		out << "  done " << iteration << " iterations in "<<  iterateWatch.Seconds() << "s \n";	
		levelStatFile << ToShort(methodName) << ";" << ToShort(name) << ";"
			<< layer->level << ";"
			<< layer->nodes().size() << ";" << layer->edges().size() << ";"
			<< initWatch.Seconds() << ";"
			<< iterateWatch.Seconds() / iteration << endl;

		totalTime.Tick();		
		out << "Total time: " << totalTime.Seconds();
		DrawLayer();

	}


	void CalcForces(double& energy, double& avForce)
	{
		energy = 0;
		avForce = 0;
		vector<int> threadIds;
		concurrency::critical_section cs;


		double sumsForce[10];
		double sumsEnergy[10];
		for (int i = 0; i < 10; i++)
		{
			sumsForce[i] = 0;
			sumsEnergy[i] = 0;
		}

		concurrency::parallel_for_each(method->layer->nodes().begin(), method->layer->nodes().end(), [&](NodePtr node)	{
			int threadNum = concurrency::Context::VirtualProcessorId();
			if (threadNum == -1)
				threadNum = 9;
			assert(threadNum < 20);
			node->force = method->CalcForce(node, threadNum);
			sumsEnergy[threadNum] += node->force.LengthSqr();;
			sumsForce[threadNum] += node->force.Lentgh();
		});

		double sumForce = 0;
		double sumEnergy = 0;
		for (int i = 0; i < 10; i++)
		{
			sumEnergy += sumsEnergy[i];
			sumForce += sumsForce[i];
		}

		avForce = sumForce / method->layer->nodes().size();
		energy = sumEnergy;
	}

	void MoveNodes(double maxStep)
	{
		auto batchSize = 1000;
		auto batchCount = (int)ceil(method->layer->nodes().size()*1.0 / batchSize);

	

		concurrency::parallel_for(0, batchCount, [&](int batchNum){
			int threadNum = concurrency::Context::VirtualProcessorId();
			if (threadNum == -1)
				threadNum = 9;


			auto batchFirst = batchNum* batchSize;
			auto batchEnd = min(batchFirst + batchSize, method->layer->nodes().size());

			for (auto i = batchFirst; i < batchEnd; i++)
			{
				Node* node = method->layer->nodes()[i];
				auto len = node->force.Lentgh();
				Point newSpeed;
				if (len != 0)
				{
					newSpeed = node->force*(maxStep / len);
				}
				node->speed = node->speed*0.3 + newSpeed*0.7;
				node->MoveBy(node->speed);
				assert(!isnan(node->Pos().Lentgh()));
				node->Pos()[2] -= maxStep / 10;
				node->Pos()[2] = max(0.0, node->Pos()[2]);
				
			}
		});

		
	}

	void DrawLayer()
	{
		auto extent = layer->CalcExtent();
		totalTime.Stop();
		wchar_t buff[200];
		wsprintf(buff, L"%s\\%s\\Layer %d.png", outputDir.c_str(), outName.c_str(), layer->level, iteration);
		layer->Draw(extent, { drawFinalSize, drawFinalSize }, buff, false);
		totalTime.Start();
	}



	void DrawIteration(Rect extent)
	{
		if (drawStages)
		{
			totalTime.Stop();
			wchar_t buff[200];
			wsprintf(buff, L"%s\\%s\\Layer %d Iteration %d.png", outputDir.c_str(), outName.c_str(), layer->level,iteration);
			layer->Draw(extent, { drawStagesSize, drawStagesSize }, buff, false);
			totalTime.Start();
		}
	}

	void DrawMovieFrame(Rect extentBefore, Rect extentAfter);


	


};

