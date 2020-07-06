#include "RefCounter.h"

gReferenceCounter::gReferenceCounter()
{
	m_refCounter = 0; // 1 or 0??
}

gReferenceCounter::~gReferenceCounter()
{

}

void gReferenceCounter::addRef()
{
	m_refCounter++;
}

void gReferenceCounter::release()
{
	m_refCounter--;
}

COUNTER_TYPE gReferenceCounter::getRefCounter() const
{
	return m_refCounter;
}