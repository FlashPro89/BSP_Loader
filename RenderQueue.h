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

struct gRenderableParameters
{
};

typedef __int64 GRQSORTINGKEY;

class gRenderElement
{
public:
	gRenderElement();
	gRenderElement( gRenderable* renderable, gMaterial* material, float distance );
	~gRenderElement();

	unsigned __int64 getKey() const;

protected:
	__int64 m_key;

	void _buildKey();

	gRenderable* m_renderable;
	gMaterial* m_material;
	unsigned short m_distance;
};

class gRenderQueue
{
public:
	gRenderQueue();
	~gRenderQueue();

	void initialize(unsigned int materialsNum);
	void sort();
	bool pushBack( const gRenderElement& element );
	bool popBack( gRenderElement& element );

protected:
	gRenderElement* m_elements;
	unsigned int m_elementsArraySize;
	unsigned int m_arrayPos;
	
};

#endif

