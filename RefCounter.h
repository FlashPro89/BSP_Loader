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
	virtual void release();
	COUNTER_TYPE getRefCounter() const;

protected:
	unsigned short m_refCounter; // сколько байт поставить в счетчик???
};

#endif