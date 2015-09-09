// App1.cpp : Defines the entry point for the console application.
// 

#include "stdafx.h"
#include "Methods\ForceLayout.h"
#include "Methods\SpringElectric.h"
#include "Methods\SpringMethod.h"
#include "Utils\Stopwatch.h"
#include "Methods\SpringTopDown.h"

concurrency::critical_section base_object_cs;

int ms_elapsed( clock_t start, clock_t stop )
{
    return (int)( 1000.0 * ( stop - start ) / CLOCKS_PER_SEC );
}
//
//void lapacktest()
//{
//	int m = 1000;
//
//
//	double* a = new double[m*m];
//	double* w = new double[2];
//	int eigenCount=2;
//	double * eigenVectors = new double[m*2];
//    lapack_int info,lda;
//    int i,j;
//
//   
//	for (int i=0; i < m*m; i++)
//		a[i] = i;
//
//    volatile clock_t start;
//    volatile clock_t stop;
//
//   start = clock();
//   //DSYEV
//   //LAPACKE_dsyev  SSTEGR 
//	info = LAPACKE_dsyev(LAPACK_COL_MAJOR,'V','L',m,a,m,w);
//    int foundEigenValues;
//	int foundEigenVectors = 
//	//info = LAPACKE_dsyevr(LAPACK_COL_MAJOR,'V','I','L',m,a,m,0,0,1,2,0,&foundEigenValues, w,eigenVectors,m,&foundEigenVectors);
//	stop = clock();
//	cout<< "lapack test: " <<ms_elapsed(start,stop) << " ms" << endl;
//	
//    double min1=w[0],min2=w[1];
//	int min1i=0,min2i=1;
//	for(int i=0; i < m; i++)
//	{
//		if (w[i] < min1)
//		{
//			min2 = min1;
//			min2i = min1i;
//			min1 = w[i];
//			min1i = i;
//		}
//	}	
//
//}
//
//void SpectralLayout(Graph& g,GraphAttributes& ga)
//{
//	int nodeCount = g.numberOfNodes();
//	// prepare laplacian L = D - A
//	float *matrix = new float [nodeCount*nodeCount];
//	float *eigenvalues = new float[3];
//	float *eigenVectors = new float[3*nodeCount];
//	ZeroMemory(matrix,nodeCount*nodeCount*sizeof(float));
//	for (auto edge = g.firstEdge(); edge != NULL; edge = edge->succ())
//	{
//		int n1 = edge->source()->index();
//		int n2 = edge->target()->index();
//		float weight = -1.0/sqrt(edge->source()->degree() * edge->target()->degree());
//		weight=-1;
//		matrix[n1 + n2*nodeCount] = weight;
//		matrix[n2 + n1*nodeCount] = weight;
//	}
//	for (auto node = g.firstNode(); node != NULL; node= node->succ())
//	{
//		int n = node->index();		
//		//matrix[n + nodeCount*n] = 1;//node->degree();
//	}
//	int foundEigenValuesCount;
//	int isupz[10];
//	lapack_int info = info = LAPACKE_ssyevr(LAPACK_COL_MAJOR,'V','I','U',nodeCount,matrix,nodeCount,0,0,2,3,0,&foundEigenValuesCount, eigenvalues,eigenVectors,nodeCount,isupz);
//	// find 2 smallest eigenvalues
//	
//	for (auto node = g.firstNode(); node != NULL; node= node->succ())
//	{
//		int n = node->index();		
//		ga.x(node) = 100*eigenVectors[0*nodeCount + n];
//		ga.y(node) = 100*eigenVectors[1*nodeCount + n];
//
//	} 
//
//	delete[] matrix;
//	delete[] eigenvalues;
//	delete[] eigenVectors;
// }

//
//bool LoadCSVGraph(Graph& g,const wstring& fileName)
//{
//	ifstream file;
//	file.open(fileName);
//
//	int nodeCount;
//	int edgeCount;
//	file >> nodeCount >> nodeCount >> edgeCount;
//	vector<node> nodes()(nodeCount);
//	for (int i=0; i < nodeCount;i++)
//	{
//		nodes()[i] = g.newNode();
//	}
//	char buff[200];
//	while(!file.eof())
//	{
//		int n1,n2;
//		file >> n1 >> n2;
//		if (file.bad()||file.fail())
//			break;
//		g.newEdge(nodes()[n1-1],nodes()[n2-1]);
//		file.getline(buff,200);
//	}
//	return true;
//}

//
//void SpectrasVsFMAA()
//{
//	
//
//	Graph G;
//	GraphAttributes GA(G);
//	cout<< "Loading.."<<endl;
//	LoadCSVGraph(G,L"graphs/commanche.txt");
//	
//	/*node v;
//	forall_nodes(v,G)
//		GA.width(v) = GA.height(v) = 10.0;
// */
//	
//	volatile clock_t start;
//    volatile clock_t stop;
//	cout<< "Laying out FMMA.. ";
//    start = clock();
//	FMMMLayout fmmm; 
//	fmmm.useHighLevelOptions(true);
//	fmmm.unitEdgeLength(15.0); 
//	fmmm.newInitialPlacement(true);
//	fmmm.qualityVersusSpeed(FMMMLayout::qvsBeautifulAndFast); 
//	fmmm.call(GA);	
//	stop = clock();
//	cout<<ms_elapsed(start,stop) << " ms" << endl;
//	GA.writeGML("commanche_fmmma.gml"); 
//
//	cout<< "Laying out spectral.. ";
//	start = clock();
//	SpectralLayout(G,GA);
//	stop = clock();
//	cout<<ms_elapsed(start,stop) << " ms" << endl;
//	cout<< "Saving.."<<endl;
//	GA.writeGML("commanche_spectral.gml"); 
//}

void TestLineDraw()
{
	Point start1 = { 20, 110 };
	Point start2 = { 10, 100 };
	Point end1 = { 0, 210 };
	Point end2 = { 30, 410 };

	Image img({ { 0, 0 }, { 400, 400 } }, { 500, 500 }, 10, true);
	img.Fill(255, 255, 255, 255);

	MemoryImage vImg(500, 500);
	

	
	vImg.DrawLine( 20, 20 ,  390, 400 , 1.0f/25);
	Point offset(10, 0);
	int cnt = 255;
	for (int i = 0; i <= cnt; i++)
	{
		vImg.DrawLine(
			offset + start1*(1.0*(cnt - i) / cnt) + start2*(1.0*i / cnt),
			offset + end1*(1.0*(cnt - i) / cnt) + end2*(1.0*i / cnt),
			1.0/cnt);
	};
	img.DrawMemoryImage(vImg);
	img.Save(L"line draw test.png");
}

void TestMotionBlur()
{;

	Image img({ { 0, 0 }, { 400, 400 } }, { 500, 500 }, 10, true);
	MemoryImage memImg(400, 400);
	for (float y = 10; y < 100; y += 10.1)
	{
		memImg.DrawLine(10, y, 100.5, y, 1);
	}

	for (double y = 10; y < 100; y += 10.1)
	{
		for (double x = 200; x < 300; x+=4.2)
			memImg.DrawLine(x, y, x+4.2, y, 1);
	}
	img.DrawMemoryImage(memImg);
	img.Save(L"motion blur test.png");
}

void TestSpringTopDown()
{
	auto layer = Layer::CreateQuad(1,true);
	auto topLayer = layer;
	while (topLayer->nodes().size() > 1)
		topLayer = new Layer(topLayer, 0);

	auto m = new SpringTopDown();
	m->topLayer = topLayer;	
	
	
		m->layer = layer;
		m->InitLayer();
		m->drawStages = true;
		m->InitIteration(0);
	

}

void DiameterTest()
{
	auto l = new Layer(L"graphs\\epb2.mtx");
	auto m = new SpringMethod();
	m->layer = l;

	Stopwatch w1, w2;
	w1.Start();
	auto diam1 = m->CalcGraphDiam();
	w1.Stop();

	w2.Start();
	auto diam2 = m->CalcDiameterFast();
	w2.Stop();

	cout << "Diameter 1: " << (unsigned)diam1 << " in " << w1.Seconds() << " s" << endl;
	cout << "Diameter 2: " << (unsigned)diam2 << " in " << w2.Seconds() << " s" << endl;

}


void ConcurencyTest()
{
	
	while (true)
	{
		concurrency::parallel_for(0, 10000, [&](int d)
		{

			NodePtr stack[100];
			int cnt = rand()%100;
			for (int i = 0; i < cnt; i++)
			{
				stack[i] = pool<Node>::New(0);
			}
			for (int i = 0; i < cnt; i++)
			{
				NodeDelete( stack[i]);
			}
			cout << '.';
		});

		cout << "Ok.";


	}
}

void Test(){
	Stopwatch watch;
	TestMotionBlur(); return;
	
	//ConcurencyTest(); return;

	//TestLineDraw(); return;

	//DiameterTest();
	//return;

	/*auto l = new Layer(L"graphs\\epb2.mtx");
	auto l1 = new Layer(l, 0);
	l1->SaveAs(L"graphs\\epb2_level1.mtx_coarsened");
	auto l2 = new Layer(l1, 0);
	l2->SaveAs(L"graphs\\epb2_level2.mtx_coarsened");
	return;*/

	ForceLayout fl;
	//fl.Load(G,GA);
	//fl.Load(L"..\\graphs\\simple.mtx");
	//fl.Load(L"..\\graphs\\n3c4-b4.mtx");
	//fl.Load(L"graphs\\jagmesh1.mtx");		
	//fl.Load(L"graphs\\add32.mtx");
	
	//fl.Load(L"..\\graphs\\data.mtx");
	//fl.Load(L"..\\graphs\\commanche_dual.mtx");	
	//fl.Load(L"..\\graphs\\barth.mtx");	
	//fl.Load(L"graphs\\finance256.mtx");
	//fl.Load(L"..\\graphs\\ncvxbqp1.mtx");
	//fl.Load(L"..\\graphs\\pkustk01.mtx");	
	//fl.Load(L"..\\graphs\\finan512.mtx");
	//fl.Load(L"graphs\\epb2.mtx",L"output\\epb2- spring electric\\Layout 0.layout");
	fl.isDebugRun = true;
	fl.LoadQuadMesh(3);
	

	auto m = new SpringMethod();
	m->useAproxForce = true;
	m->useAproxNodeProperties = true;
	m->useAproxForceEdgeTransitions = false;
	m->useAproxForceRandomUpperLevels = true;
	//m->useAproxForceRandomUpperLevelsParallel = true;
	/*m->useAproxForceAlternativeUpperLevels = true;
	m->useAproxForceAlternativeUpperLevelsAveraging = false;
	m->aproxForceAlternativeUpperLevelsCount = 10;*/
	m->exactForceMaxDistance =  numeric_limits<int>::max();

	auto m1 = new SpringElectric();
	//fl.drawStages = true;
	//fl.drawMovie = true;
	
	fl.method = m;
	fl.outputDir = L"drawings";
	//fl.drawStages = true;
	
	//fl.Execute();// return;
	//m->DrawAlternativeLayouts();
	fl.TestSpringApprox();
	//GA.writeGML("commanche_coarsed.gml");
}

void ConvexHullTest()
{
	vector<Point> points = { { 0, 0 }, { 0, 1 }, { 0, 0 }, { 0, 2 }, { 0, 2 }, { 0, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 } };
	Image img(Rect({ -2, -2 }, { 2, 2 }), { 300, 300 },10,true);
	//img.DrawConvexHull(points,4);
	img.SetFillColor(255, 255, 0, 0);
	for (auto p : points)
		img.DrawPoint(p);
	img.SetFillColor(255,0, 0, 0);
	for (auto p : points)
		img.DrawLine(Point(0, 0), p);
	img.Save(L"convex hull.png");
}


bool Compare(wchar_t* s1, wchar_t* s2)
{
	int len1 = lstrlenW(s1);
	int len2 = lstrlenW(s2);
	if (len1 != len2)
		return false;
	for (int i = 0; i < len1; i++)
	{
		if (towlower(s1[i]) != towlower(s2[i]))
			return false;
	}
	return true;
}


void PrintArgs()
{
	cout << "Usage: <mtx file path> <output path> [<draw params>] <method name> <method arguments>" << endl;
	cout << "   <mtx file path>     source graph in Matrix Market format" << endl;
	cout << "   <output path>       output directory"     << endl;
	cout << "   <draw params>:      " << endl;
	cout << "		/drawStages <size>  draw intermediate levels and iterations" << endl;	
	cout << "		/drawMovie <size>  draw movie frames (multiple per iteration)" << endl;
	cout << "		/drawFinal <size>  set final frame size" << endl;
	cout << endl;
	cout << "methods:" << endl;
	cout << "   springelectric      draw using multilevel Fruchterman & Reingold " << endl;
	cout << "                       no arguments" << endl;
	cout << "   " << endl;
	cout << "   springexact         draw using multilevel Kamada Kawai algorithm." << endl;
	cout << "       [/maxDistance <maxDistance>]         " << endl;
	cout << "                       maximum distance from node" << endl;
	cout << "   " << endl;
	cout << "   springaprox         draw using multilevel aproximated Kamada Kawai modyfication" << endl;
	cout << "		[/AproxNodeProperties]  			     " << endl;
	cout << "                       aproximate average graph distance (save n*n*logN to n*n*logN)" << endl;
	cout << "       [/EdgeTransitions] " << endl;
	cout << "                       calculate transition path throug claster nodes(). " << endl;
	cout << "                       Improves resoults greatly" << endl;
	cout << "                       Costs m*logM in time each iteration and space same space" << endl;
	cout << "       [/AlternativeUpperLevels  <count>]" << endl;
	cout << "                       Alternate coarsenings between iterations to mimise error." << endl;
	cout << "                       Costs m*logM + n*logN in space, processing per level" << endl;
	cout << "       [/AlternativeUpperLevelsAverage]" << endl;
	cout << "                       Average forces from all alternative coarsenings each iteration" << endl;
	cout << "                       Best quality, cost linear to alteratives count" << endl;
	cout << "       [/AlternativeUpperLevelsRandom ]" << endl;
	cout << "                       Generate new random upper level each iteration." << endl;
	cout << "                       Costs m*logM + n*logN in space." << endl;
	cout << "       [/SkipManyLevels ]" << endl;
	cout << "                       Move move then one level if theshold allows." << endl;
	cout << "       [/SkipLevelThreashold ]" << endl;
	cout << "                       Aproximation threshold ." << endl;
	cout << "   " << endl;
}

bool ParseArgs(int argc, _TCHAR* argv[])
{
	if (argc < 4)
		return false;

	wstring filePath = argv[1];
	
	//IForceMethod* method = NULL;
	ForceLayout fl;
	fl.outputDir = argv[2];
	int argNum = 3;
	bool getMethodDesc = false;
	while (argNum < argc)
	{
		if (Compare(argv[argNum], L"/drawFinal"))
		{
			argNum++;		
			fl.drawFinalSize = _wtoi(argv[argNum]);
		}
		else if (Compare(argv[argNum], L"/drawStages"))
		{
			argNum++;
			fl.drawStages = true;
			fl.drawStagesSize = _wtoi(argv[argNum]);			
		}
		else if (Compare(argv[argNum], L"/drawMovie"))
		{
			argNum++;
			fl.drawMovie = true;
			fl.drawMovieSize = _wtoi(argv[argNum]);
		
		}
		else if (Compare(argv[argNum], L"/getName"))
		{
			getMethodDesc = true;			
		}
		else
		{
			break;
		}
		argNum++;
	}

	
	if (Compare(argv[argNum], L"springelectric"))
	{
		argNum++;
		fl.method = new SpringElectric();
	}
	else if (Compare(argv[argNum], L"springaprox"))
	{
		auto se = new SpringMethod();
		se->useAproxForce = true;
		se->useAproxNodeProperties = false;
		se->useAproxForceEdgeTransitions = false;
		se->useAproxForceAlternativeUpperLevels = false;
		se->useAproxForceAlternativeUpperLevelsAveraging = false;		
		se->useAproxForceRandomUpperLevels = false;
		se->useAproxForcePersistentNodeProperties = false;
		fl.method = se;
		argNum++;
		while (argNum < argc)
		{			
			if (Compare(argv[argNum], L"/AproxNodeProperties"))
			{
				se->useAproxNodeProperties = true;
			}
			else if (Compare(argv[argNum], L"/AproxNodePropertiesPersistent"))
			{
				se->useAproxForcePersistentNodeProperties = true;
			}
			
			else if (Compare(argv[argNum], L"/EdgeTransitions"))
			{
				se->useAproxForceEdgeTransitions = true;
			}
			else if (Compare(argv[argNum], L"/AlternativeUpperLevelsRandom"))
			{
				se->useAproxForceRandomUpperLevels = true;
			}
			else if (Compare(argv[argNum], L"/AlternativeUpperLevels"))
			{
				se->useAproxForceAlternativeUpperLevels=true;
				argNum++;
				se->aproxForceAlternativeUpperLevelsCount = _wtoi(argv[argNum]);
			}
			else if (Compare(argv[argNum], L"/AlternativeUpperLevelsAverage"))
			{
				se->useAproxForceAlternativeUpperLevelsAveraging = true;
			}
			else if (Compare(argv[argNum], L"/SkipManyLevels"))
			{
				se->useAproxForceSkipManyLevels = true;
			}
			else if (Compare(argv[argNum], L"/SkipLevelThreashold"))
			{
				argNum++;
				se->useAproxForceSkipLevelThreashold = wcstod(argv[argNum],0);
			}
			else
			{
				cout << "Unexpected argument: " << std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(argv[argNum]) << endl;
				return false;
			}
			argNum++;
		}
	}
	else if (Compare(argv[argNum], L"springexact"))
	{
		auto se = new SpringMethod();
		se->useAproxForce = false;
		se->useAproxNodeProperties = false;
		se->useAproxForceEdgeTransitions = false;
		se->useAproxForceAlternativeUpperLevels = false;
		se->useAproxForceAlternativeUpperLevelsAveraging = false;
		se->useAproxForceRandomUpperLevels = false;
		se->useAproxForcePersistentNodeProperties = false;
		se->exactForceMaxDistance = numeric_limits<int>::max();
		fl.method = se;
		argNum++;
		while (argNum < argc)
		{
			if (Compare(argv[argNum], L"/maxDistance"))
			{
				argNum++;
				se->exactForceMaxDistance = _wtoi(argv[argNum]);
			}
			else
			{
				cout << "Unexpected argument: " << std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(argv[argNum]) << endl;
				return false;
			}
			argNum++;
		}
	}
	else
	{
		cout << "Unexpected argument: " << std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(argv[argNum]) << endl;
		return false;
	}
	if (getMethodDesc)
	{
		wcout << fl.method->Describe();
		
		return true;
	}

	
	fl.Load(filePath);
	
	fl.Execute();
	
}

int _tmain(int argc, _TCHAR* argv[])
{	
	
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	if (Compare(argv[1], L"test"))
	{
		Test();

	}
	else
	{
		if (!ParseArgs(argc, argv))
			PrintArgs();
	}
	
 
	int i;
	

	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}

