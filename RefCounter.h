#pragma once
#ifndef _REF_COUNTER_H_
#define _REF_COUNTER_H_

#define COUNTER_TYPE unsigned short

class gReferenceCounter
{
public:
	gReferenceCounter();
	virtual ~gReferenceCounter();

	virtual void addRef();
	virtual void release() = 0;
	COUNTER_TYPE getRefCounter() const;

	/*
	void* operator new (size_t sz)
	{
		COUNTER_TYPE* p = (COUNTER_TYPE*)::operator new[](sz + sizeof(COUNTER_TYPE));
		printf("new()\n");
		return ++p;
	}

	void operator delete (void* v)
	{
		COUNTER_TYPE* p = (COUNTER_TYPE*)v;
		::operator delete[](--p);
		printf("delete()\n");
	}
	*/

protected:
	unsigned short m_refCounter; // сколько байт поставить в счетчик???

private:
	const gReferenceCounter& operator = (const gReferenceCounter&) {}
	const gReferenceCounter* operator = (const gReferenceCounter*) {}
	gReferenceCounter* operator = (gReferenceCounter*) {}
};

#endif