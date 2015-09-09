


template<class T>
class pool
{
	static T* freeList;

	
public:

	template<class... Args>
	static T* New(Args&&... args)
	{



		if (freeList == nullptr)
		{
			auto ob = new T;			
			ob->Init(args...);
			
			return ob;
		}
		else
		{
			auto ob = freeList;
			freeList = ob->nextFree;
			ob->Init(args...);
			return ob;
		}
	}

	static void Delete(T* ob)
	{				
		ob->nextFree = freeList;
		freeList = ob;
	}
};


template<class T>
T* pool<T>::freeList=nullptr;