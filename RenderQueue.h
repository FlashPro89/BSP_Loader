#pragma once

#ifndef _RENDER_QUEUE_H_
#define _RENDER_QUEUE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string>

/*
int num[10] = {
  1, 3, 6, 5, 8, 7, 9, 6, 2, 0
};

int comp(const void*, const void*);

int main(void)
{
	int i;

	printf("Исходный массив: ");
	for (i = 0; i < 10; i++) printf("%d ", num[i]);

	qsort(num, 10, sizeof(int), comp);

	printf("Отсортированный массив: ");
	for (i = 0; i < 10; i++) printf("%d ", num[i]);

	return 0;
}

// сравнение целых 
int comp(const void* i, const void* j)
{
	return *(int*)i - *(int*)j;
}

*/

class gRenderable;
class gMaterial;


typedef unsigned __int64 GRQSORTINGKEY;

class gRenderElement
{
public:
	//gRenderElement(gRenderElement& other);
	gRenderElement();
	gRenderElement( gRenderable* renderable, gMaterial* material, float distance );
	~gRenderElement();

	GRQSORTINGKEY getKey() const;

protected:
	GRQSORTINGKEY m_key;

	void _buildKey();

	gRenderable* m_pRenderable;
	gMaterial* m_pMaterial;
	unsigned short m_distance;
};

class gRenderQueue
{
public:
	gRenderQueue();
	~gRenderQueue();

	void initialize( unsigned int elementsMaxNum ); //only one time
	void sort();
	bool pushBack( const gRenderElement& element );
	bool popBack( gRenderElement** element );
	void clear();

protected:
	gRenderQueue(gRenderQueue&) {};
	gRenderQueue(const gRenderQueue&) {};

	gRenderElement** m_elementsPointers;
	gRenderElement* m_elements;
	unsigned int m_elementsArraySize;
	unsigned int m_arrayPos;
	
};

#endif

