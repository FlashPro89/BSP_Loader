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

gRenderElement::gRenderElement()
{
	m_key = 0;
	m_renderable = 0;
	m_material = 0;
	m_distance = 0;
}

gRenderElement::gRenderElement( gRenderable* renderable, gMaterial* material, float distance )
{
	m_renderable = renderable;
	m_material = material;
	m_distance = distance * 0xFFFF; //distance 0.f ... 1.f 
	_buildKey();
}


gRenderElement::~gRenderElement()
{

}

unsigned __int64 gRenderElement::getKey() const
{
	return m_key;
}

void gRenderElement::_buildKey()
{
	m_key = m_distance;
	if (m_material)
		m_key |= m_material->getId();
}

//-----------------------------------------------
//
//	CLASS: gRenderQueue
//
//-----------------------------------------------

gRenderQueue::gRenderQueue()
{
	m_elements = 0;
	m_elementsArraySize = 0;
	m_arrayPos = 0;
}

gRenderQueue::~gRenderQueue()
{
	if (m_elements)
		delete[] m_elements;
}

void gRenderQueue::initialize( unsigned int elementsMaxNum )
{
	if (m_elements)
		delete[] m_elements;

	m_elements = new gRenderElement[elementsMaxNum];
}

int compRE( const void* i, const void* j ) // сделать чтото с этим
{
	__int64 x = ((gRenderElement*)i)->getKey();
	__int64 y = ((gRenderElement*)j)->getKey();

	if (x > y) return 1;
	else if (x == y) return 0;
	else
		return -1;
}

void gRenderQueue::sort()
{
	qsort( m_elements, m_arrayPos, sizeof(gRenderElement), compRE);
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
	element = m_elements[m_arrayPos++];
	return true;
}
