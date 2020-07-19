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
struct IDirect3DDevice9;

typedef unsigned __int64 GRQSORTINGKEY;

struct D3DXMATRIX;

enum gRenderPrimitiveType
{
	GRP_LINELIST,
	GPR_TRIANGLELIST
};

struct gSkinBoneGroup;

class gRenderElement
{
public:
	//gRenderElement(gRenderElement& other);
	gRenderElement();
	gRenderElement( const gRenderable* renderable, const gMaterial* material, unsigned short distance, unsigned char matrixPaleteSize, 
		const D3DXMATRIX* matrixPalete, unsigned int startIndex, unsigned int primitiveCount, unsigned int vertexesNum = 0, const gSkinBoneGroup* remapedBones = 0 );
	~gRenderElement();

	GRQSORTINGKEY getKey() const;

	const gRenderable* getRenderable() const;
	const gMaterial* getMaterial() const;
	const D3DXMATRIX* getMatrixPalete() const;
	unsigned char getMatrixPaleteSize() const;

	unsigned short getDistance() const;

	unsigned int getStartIndex() const;
	unsigned int getPrimitiveCount()  const;
	unsigned int getVertexesNum()  const;
	const gSkinBoneGroup* getSkinBoneGroup() const;

protected:
	GRQSORTINGKEY m_key;

	void _buildKey();

	const gRenderable* m_pRenderable;
	const gMaterial* m_pMaterial;
	unsigned short m_distance;
	const D3DXMATRIX* m_pMatPalete; // world matrixes
	unsigned char m_paleteSize; // num of world matrixes
	unsigned int m_startBufferIndex;
	unsigned int m_primitiveCount;
	unsigned int m_vertexesNum;
	const gSkinBoneGroup* m_pSkinBoneGroup;

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

	void render(IDirect3DDevice9* pDevice);

	//DEBUG
	void _debugOutSorted( const char* fname );
	void _debugOutUnsorted(const char* fname);

protected:
	gRenderQueue(gRenderQueue&) {};
	gRenderQueue(const gRenderQueue&) {};

	gRenderElement** m_elementsPointers;
	gRenderElement* m_elements;
	unsigned int m_elementsArraySize;
	unsigned int m_arrayPos;

	int m_lastRenderable;
	int m_lastMaterial;
	int m_lastMatSkinned;
	int m_lastMatUseBlending;
	
};

#endif

