#pragma once



template<class DerivedType>
struct LastEmpty{
	unsigned  aba = 0;
	DerivedType* node = 0;
};

extern	 concurrency::critical_section base_object_cs;

template<class DerivedType,int TypeID>
class BaseObject{
	
	int typeId = TypeID;
	DerivedType* nextEmpty=0;
	BaseObject* thisAddress;
	char lifeCountSpace[sizeof(atomic<int>)];
	
	static atomic<LastEmpty<DerivedType>> lastEmpty;
	

	

	DerivedType*& EmptyPointer()
	{
		return nextEmpty;
		return *(DerivedType**)(&typeId + 1);
	}

public:
	BaseObject()
	{
		thisAddress = this;
	}
	virtual ~BaseObject()
	{
		assert(typeId == TypeID);

	}
	#ifdef sdfasdfsadf
	void* operator new(size_t size)
	{
		LastEmpty<DerivedType> next, orig = lastEmpty.load();;
		DerivedType* newOb;
		do
		{			
			if (orig.node == nullptr)
				break;
			
			next.aba = orig.aba + 1;
			next.node = orig.node->EmptyPointer();
		} while (!lastEmpty.compare_exchange_weak(orig, next));

		if (orig.node == nullptr)
		{
			
			
			auto newBatch = (DerivedType*) new char[size];
			newOb= newBatch;
			

#ifndef NDEBUG
			new (&newOb->lifeCountSpace) atomic < int >;
#endif
		}
		else
		{
			newOb = orig.node;
			assert(orig.node->typeId == TypeID);
		}
#ifndef NDEBUG
		int expectedLife = 0;
		((atomic < int >*)(&newOb->lifeCountSpace))->compare_exchange_weak(expectedLife, 1);
		assert(expectedLife == 0);
#endif
		return newOb;
/*
		if (lastEmpty != nullptr)
		{
			auto foundEmpty = lastEmpty;
			lastEmpty = lastEmpty->EmptyPointer();
			return foundEmpty;
		}
		else
		{
			return new char[size];
		}*/
	}

	void	operator delete(void *mem)
	{		

		auto ptr = (DerivedType*)mem;
		assert(ptr->typeId == TypeID);
		LastEmpty<DerivedType> next, orig = lastEmpty.load();;
#ifndef NDEBUG
		int expectedLife = 1;
		((atomic < int >*)(&ptr->lifeCountSpace))->compare_exchange_weak(expectedLife, 0);		
		assert(expectedLife == 1);
#endif
		
		do
		{
			
			ptr->EmptyPointer() = orig.node;
			next.aba = orig.aba + 1;
			next.node = ptr;
		} while (!lastEmpty.compare_exchange_weak(orig, next));


		/*auto ptr = (DerivedType*)mem;
		ptr->EmptyPointer() = lastEmpty;
		lastEmpty = ptr;*/
	}
#endif
};

template <class DerivedType, int TypeId> atomic<LastEmpty<DerivedType>> BaseObject<DerivedType, TypeId>::lastEmpty;