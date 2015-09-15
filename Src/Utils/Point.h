#pragma once


template <int i>
class fordimm{
public:
	template <class Func>
	void static loop(const Func &f)
	{
		f(i - 1);
		fordimm< i - 1>::loop(f);
	}
};

template <> 
class fordimm <0> {
public:
	template <class Func>	
	void static loop(const Func &f){}
};

template<unsigned dimm>
class PointDimm
{
	int t = (int)0xFAFAFAFA2;
public:
	double val[dimm];

	const static int dimmensions = dimm;

	double X()const { return val[0]; }
	double Y()const { return val[1]; }


	double& X(){ return val[0]; }
	double& Y(){ return val[1]; }


	double& operator[](int d){ return val[d]; }

	PointDimm(){};
	PointDimm(const PointDimm&other)
	{
		fordimm<dimm>::loop([&](unsigned d)
		{
			val[d] = other.val[d];
		});
	};
	PointDimm(const double& other)
	{		
		fordimm<dimm>::loop([&](unsigned d)
		{
			val[d] = other;
		});		
	}

	bool operator==(const PointDimm& other)const
	{
		bool eq = true;
		fordimm<dimm>::loop([&](unsigned d)
		{
			eq = eq && (val[d] == other.val[d]);
		});
		return eq;
	}


	bool operator!=(const PointDimm& other)const
	{
		return !( *this == other);
	}

	
	PointDimm(double x, double y){
		val[0] = x;
		val[1] = y;

		fordimm<dimm-2>::loop([&](unsigned d)
		{
			val[d+2] = 0;
		});
	}

	PointDimm(int x, int y){
		val[0] = x;
		val[1] = y;

		fordimm<dimm - 2>::loop([&](unsigned d)
		{
			val[d + 2] = 0;
		});
	}

	


	operator Gdiplus::Point() const
	{
		return Gdiplus::Point((int)val[0], (int)val[1]);
	}

	PointDimm DirectionTo(const PointDimm& other)
	{
		auto dir = other - *this;
		dir *= 1 / dir.Lentgh();
		return dir;
	}

	double DotProduct(const PointDimm& other)
	{
		double result = 0;
		fordimm<dimm>::loop([&](unsigned d)
		{
			result += val[d] * other.val[d];
		});
		return result;
	}

	PointDimm operator- (const PointDimm& other)const
	{
		Point result;		
		fordimm<dimm>::loop([&](unsigned d)
		{
			result.val[d]=  val[d] - other.val[d];			
		});
		result.check();
		return result;
	}

	void check() const
	{
		fordimm<dimm>::loop([&](unsigned d)
		{
			assert(!isnan(val[d]));
		});
	}

	PointDimm operator+ (const PointDimm& other)const
	{
		Point result;
		fordimm<dimm>::loop([&](unsigned d)
		{
			result.val[d] = val[d] + other.val[d];			
		});
		result.check();
		return result;
	}



	PointDimm operator* (const double& other) const
	{
		Point result;
		fordimm<dimm>::loop([&](unsigned d)
		{
			result.val[d] = val[d] * other;
		
		});
		result.check();
		return result;
	}

	PointDimm operator/ (const double& other) const
	{
		assert(other != 0);
		Point result;		
		fordimm<dimm>::loop([&](unsigned d)
		{
			result.val[d]=  val[d]/other;
			
		});
		result.check();
		return result;
	}

	void operator+= (const PointDimm& other)
	{
		
		fordimm<dimm>::loop([&](unsigned d)
		{
			val[d] += other.val[d];
			
		});
		check();		
	}

	void operator-= (const PointDimm& other)
	{
		fordimm<dimm>::loop([&](unsigned d)
		{
			val[d] -= other.val[d];
		});
		check();
	}

	void operator*= (const double& other)
	{
		fordimm<dimm>::loop([&](unsigned d)
		{
			val[d] *= other;
		});
		check();
	}

	double Distance(const PointDimm& other)const
	{
		return sqrt(DistanceSqr(other));
	}

	double DistanceSqr(const PointDimm& other)const
	{
		double  result = 0;
		fordimm<dimm>::loop([&](unsigned d)
		{
			auto dist = val[d] - other.val[d];
			result += dist* dist;			
		});
		return result;
		
	}

	double Lentgh() const
	{
		return sqrt(LengthSqr());
	}

	double LengthSqr() const
	{
		double  result = 0;
		fordimm<dimm>::loop([&](unsigned d)
		{
			auto dist = val[d];
			result += dist* dist;
		});
		return result;
	}

	static PointDimm minCoord(const PointDimm& p1, const PointDimm& p2)
	{
		PointDimm result;
		fordimm<dimm>::loop([&](unsigned d)
		{
			
			result.val[d] = min(p1.val[d], p2.val[d]);
		});
		return result;
	}

	static PointDimm maxCoord(const PointDimm& p1, const PointDimm& p2)
	{
		PointDimm result;
		fordimm<dimm>::loop([&](unsigned d)
		{
			result.val[d] = max(p1.val[d], p2.val[d]);
		});
		return result;
	}

	void operator= (const double& other)
	{
		fordimm<dimm>::loop([&](unsigned d)
		{

			val[d] = other;
		});
		check();
	}
	
};




class PointSimple
{
	double _x;
	double _y;
	

	int t = (int)0xFAFAFAFA2;
public:
	PointSimple() = default;
	PointSimple(const PointSimple&) = default;
	PointSimple(const double& other)
	{
		_x = other;
		_y = other;
	}

	double X()const { return _x; }
	double Y()const { return _y; }


	double& X(){ return _x; }
	double& Y(){ return _y; }

	bool operator==(const PointSimple& other)const
	{
		return _x == other._x && _y == other._y;
	}


	bool operator!=(const PointSimple& other)const
	{
		return _x != other._x || _y != other._y;
	}

	PointSimple(const double& x, const double& y) : _x(x), _y(y){}
	PointSimple(const int& x, const int& y) : _x(x), _y(y){}

	operator Gdiplus::Point() const
	{
		return Gdiplus::Point((int)_x, (int)_y);
	}

	PointSimple DirectionTo(const PointSimple& other)
	{
		auto dir = other - *this;
		dir *= 1 / dir.Lentgh();
		return dir;
	}

	double DotProduct(const PointSimple& other)
	{
		return _x*other._x + _y*other._y;
	}

	PointSimple operator- (const PointSimple& other)const
	{			
		PointSimple result;
		result._x = _x - other._x;
		result._y = _y - other._y;
		assert(!isnan(result._x));
		assert(!isnan(result._y));
		return result;
	}

	PointSimple operator+ (const PointSimple& other)const
	{
		PointSimple result;
		result._x = _x + other._x;
		result._y = _y + other._y;
		assert(!isnan(result._x));
		assert(!isnan(result._y));
		return result;
	}



	PointSimple operator* (const double& other) const
	{
		PointSimple result;
		result._x = _x*other;
		result._y = _y*other;
		assert(!isnan(result._x));
		assert(!isnan(result._y));
		return result;
	}

	PointSimple operator/ (const double& other) const
	{
		PointSimple result;
		result._x = _x/other;
		result._y = _y/other;
		assert(!isnan(result._x));
		assert(!isnan(result._y));
		return result;
	}

	void operator+= (const PointSimple& other) 
	{
		_x += other._x;
		_y += other._y;
		assert(!isnan(_x));
		assert(!isnan(_y));
	}

	void operator-= (const PointSimple& other) 
	{
		_x -= other._x;
		_y -= other._y;
		assert(!isnan(_x));
		assert(!isnan(_y));
	}

	void operator*= (const double& other) 
	{
		_x *= other;
		_y *= other;
		assert(!isnan(_x));
		assert(!isnan(_y));
	}

	double Distance(const PointSimple& other)const
	{
		return sqrt((_x - other._x)*(_x - other._x) + (_y - other._y)*(_y - other._y));
	}

	double DistanceSqr(const PointSimple& other)const
	{
		return (_x - other._x)*(_x - other._x) + (_y - other._y)*(_y - other._y);
	}

	double Lentgh() const
	{
		return sqrt(_x*_x + _y*_y);
	}

	double LengthSqr() const
	{
		return _x*_x + _y*_y;
	}

	static PointSimple minCoord(const PointSimple& p1, const PointSimple& p2)
	{
		PointSimple result;
		result._x = min(p1._x, p2._x);
		result._y = min(p1._y, p2._y);
		return result;
	}

	static PointSimple maxCoord(const PointSimple& p1, const PointSimple& p2)
	{
		PointSimple result;
		result._x = max(p1._x, p2._x);
		result._y = max(p1._y, p2._y);
		return result;
	}

	void operator= (const double& other)
	{
		_x = other;
		_y = other;
		assert(!isnan(_x));
		assert(!isnan(_y));
	}

	

};



typedef PointDimm<2> Point;
//typedef PointSimple Point;