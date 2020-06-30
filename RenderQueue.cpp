#include "RenderQueue.h"
#include "Resources.h"
#include "Materials.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//-----------------------------------------------
//
//	CLASS: gRenderElement
//
//-----------------------------------------------

/*
gRenderElement::gRenderElement( gRenderElement& other )
{

}
*/

gRenderElement::gRenderElement()
{
	m_key = 0;
	m_pRenderable = 0;
	m_pMaterial = 0;
	m_distance = 0;
}

gRenderElement::gRenderElement( gRenderable* renderable, gMaterial* material, float distance )
{
	if (!renderable || !material)
		throw("Null pointers in Material!");

	m_pRenderable = renderable;
	m_pMaterial = material;
	m_distance = distance * 0xFFFF; //distance 0.f ... 1.f 
	_buildKey();
}

gRenderElement::~gRenderElement()
{

}

GRQSORTINGKEY gRenderElement::getKey() const
{
	return m_key;
}

void gRenderElement::_buildKey()
{
	m_key = m_distance; //16 bit of dist 0bit-first
	
	m_key = m_key << 16;
	m_key |= m_pMaterial->getId(); //16 bit of matId 16bit-first
	
	m_key = m_key << 32;
	m_key |= m_pRenderable->getId(); //16 bit of renderableId 32bit-first
}

//-----------------------------------------------
//
//	CLASS: gRenderQueue
//
//-----------------------------------------------

gRenderQueue::gRenderQueue()
{
	m_elements = 0;
	m_elementsPointers = 0;
	m_elementsArraySize = 0;
	m_arrayPos = 0;
}

gRenderQueue::~gRenderQueue()
{
	if( m_elements )
		delete[] m_elements;
	if (m_elementsPointers)
		delete[] m_elementsPointers;
}

void gRenderQueue::initialize( unsigned int elementsMaxNum )
{
	if (m_elements)
		delete[] m_elements;

	m_elementsArraySize = elementsMaxNum;
	m_elements = new gRenderElement[m_elementsArraySize];
	m_elementsPointers = new gRenderElement*[m_elementsArraySize];

	for (unsigned int i = 0; i < m_elementsArraySize; i++)
	{
		m_elementsPointers[i] = &m_elements[i];
	}
}

int compRE( const void* i, const void* j ) // ������� ����� � ����
{
	__int64 x = (*((gRenderElement**)i))->getKey();
	__int64 y = (*((gRenderElement**)j))->getKey();

	if (x > y) return 1;
	else if (x == y) return 0;
	else
		return -1;
}

void gRenderQueue::sort()
{
	qsort( m_elementsPointers, m_arrayPos, sizeof(gRenderElement), compRE);
}

bool gRenderQueue::pushBack(const gRenderElement& element)
{
	if (!m_elements || m_arrayPos >= m_elementsArraySize)
		return false;
	m_elements[m_arrayPos++] = element;
	return true;
}

bool gRenderQueue::popBack(gRenderElement& element)
{
	if( !m_elements || m_arrayPos == 0 )
		return false;
	element = *m_elementsPointers[m_arrayPos--];
	return true;
}

void gRenderQueue::clear()
{
	m_arrayPos = 0;
}