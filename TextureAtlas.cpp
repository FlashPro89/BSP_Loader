#include "TextureAtlas.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//-----------------------------------------------
//
//	CLASS: gTextureAtlas
//
//-----------------------------------------------

gTextureAtlas::gTextureAtlas()
{
	m_pSortableTextureSizes = 0;
	m_pSortableTextureSizesPointers = 0;
	_reset();
}

gTextureAtlas::gTextureAtlas( unsigned int texturesNum )
{
	m_pSortableTextureSizes = 0;
	m_pSortableTextureSizesPointers = 0;
	beginAtlas( texturesNum );
}

gTextureAtlas::~gTextureAtlas()
 {
	_reset();
}

void gTextureAtlas::beginAtlas( unsigned int texturesNum )
{
	_reset();
	m_texturesNum = texturesNum;
	m_pSortableTextureSizes = new gTextureSizeSortingStruct[m_texturesNum];
	m_pSortableTextureSizesPointers = new gTextureSizeSortingStruct* [m_texturesNum];
	memset( m_pSortableTextureSizes, 0, sizeof(gTextureSizeSortingStruct) * m_texturesNum );
	for (unsigned int i = 0; i < texturesNum; i++)
		m_pSortableTextureSizesPointers[i] = &m_pSortableTextureSizes[i];
}

bool gTextureAtlas::pushTexture( unsigned short width, unsigned short height, void* userData )
{
	if ( (m_curPosInArray >= m_texturesNum) || (m_pSortableTextureSizes == 0) )
		return false;

	m_pSortableTextureSizes[m_curPosInArray].baseIndex = m_curPosInArray;
	m_pSortableTextureSizes[m_curPosInArray].width = width;
	m_pSortableTextureSizes[m_curPosInArray].height = height;
	m_pSortableTextureSizes[m_curPosInArray].userData = userData;
	m_pSortableTextureSizes[m_curPosInArray].key = (height << 16) | width;

	m_curPosInArray++;
	return true;
}


//for texsizes qsort
int compareByTexSz( const void* i, const void* j )
{
	const gTextureSizeSortingStruct* t1 = *((const gTextureSizeSortingStruct**)i);
	const gTextureSizeSortingStruct* t2 = *(( const gTextureSizeSortingStruct**)j);

	int ret = 1;

	if (t1->height > t2->height)
	{
		ret = -1;
	}
	else if (t1->height == t2->height)
	{
		if (t1->width > t2->width)
			ret = -1;
		else if ( t1->width == t2->width )
			ret = 0;
	}
	return ret;
}

int compareByKey(const void* i, const void* j)
{
	const gTextureSizeSortingStruct* t1 = *((const gTextureSizeSortingStruct**)i);
	const gTextureSizeSortingStruct* t2 = *((const gTextureSizeSortingStruct**)j);

	return (int)t2->key - (int)t1->key;
}

int compareByBaseIndex(const void* i, const void* j)
{
	const gTextureSizeSortingStruct* t1 = *((const gTextureSizeSortingStruct**)i);
	const gTextureSizeSortingStruct* t2 = *((const gTextureSizeSortingStruct**)j);

	return t1->baseIndex - t2->baseIndex;
}


bool gTextureAtlas::mergeTexturesToAtlas( unsigned short maxWidth, unsigned short maxHeight, unsigned char border )
{

	//qsort( (void*)m_pSortableTextureSizesPointers, m_curPosInArray, sizeof(gTextureSizeSortingStruct*), compareByTexSz );
	
	// NEW: sort by key
	qsort( (void*)m_pSortableTextureSizesPointers, m_curPosInArray, sizeof(gTextureSizeSortingStruct*), compareByKey );

	//TEST
	
	FILE* f = 0;
	errno_t err = fopen_s(&f, "out_sort.txt", "wt");

	for (unsigned int i = 0; i < m_curPosInArray; i++)
		fprintf(f, "w:%i h:%i key:%i baseindex:%i\n", m_pSortableTextureSizesPointers[i]->width,
			m_pSortableTextureSizesPointers[i]->height, m_pSortableTextureSizesPointers[i]->key,
			m_pSortableTextureSizesPointers[i]->baseIndex);
	fclose(f);
	
	
	//sort by base index
	/*
	qsort((void*)m_pSortableTextureSizesPointers, m_curPosInArray, sizeof(gTextureSizeSortingStruct*), compareByBaseIndex);
	err = fopen_s(&f, "out_sort_byIndex.txt", "wt");

	for (unsigned int i = 0; i < m_curPosInArray; i++)
		fprintf(f, "w:%i h:%i baseindex:%i\n", m_pSortableTextureSizesPointers[i]->width,
			m_pSortableTextureSizesPointers[i]->height, m_pSortableTextureSizesPointers[i]->baseIndex);
	fclose(f);
	*/

	int lMapMaxSideSize = 256;

	//рассчитаем габариты текстуры
	unsigned short tWidth, tHeight;
	//int border = 0;
	unsigned short tXPos = border, tYPos = border;
	unsigned short rowHeight;

remapping:

	tWidth = 0, tHeight = 0;
	tXPos = border, tYPos = border;
	rowHeight = m_pSortableTextureSizesPointers[0]->height;

	//размечаем атлас
	for (unsigned short i = 0; i < m_curPosInArray; i++)
	{
		if (tXPos < (lMapMaxSideSize - m_pSortableTextureSizesPointers[i]->width - border))
		{
			m_pSortableTextureSizesPointers[i]->remappedX = tXPos;
			m_pSortableTextureSizesPointers[i]->remappedY = tYPos;

			tXPos += m_pSortableTextureSizesPointers[i]->width + border;
		}
		else //сдвиг на один ряд вниз размером с текущую текстуру и бордюр
		{
			tYPos += rowHeight + border;
			tXPos = border;
			rowHeight = m_pSortableTextureSizesPointers[i]->height;

			tWidth = lMapMaxSideSize;
			tHeight = tYPos + m_pSortableTextureSizesPointers[i]->height + border;

			if (tHeight > lMapMaxSideSize)
			{
				lMapMaxSideSize *= 2;
				goto remapping;
			}

			m_pSortableTextureSizesPointers[i]->remappedX = tXPos;
			m_pSortableTextureSizesPointers[i]->remappedY = tYPos;

			//передигаем курсор на позицию правее
			tXPos += m_pSortableTextureSizesPointers[i]->width + border;
		}
	}

	if ((tWidth <= maxWidth) && (tHeight <= maxHeight))
	{
		m_atlasWidth = tWidth;
		m_atlasHeight = tHeight;
		return true;
	}
	else
		return false;
}

unsigned int gTextureAtlas::getTexturesNum() const
{
	return m_texturesNum;
}

unsigned int gTextureAtlas::getAtlasWidth() const
{
	return m_atlasWidth;
}
unsigned int gTextureAtlas::getAtlasHeight() const
{
	return m_atlasHeight;
}

unsigned short gTextureAtlas::getTextureBaseIndexInSortedOrder(unsigned int index) const
{
	if ((index > m_texturesNum) || (m_pSortableTextureSizes == 0))
		return 0;

	return m_pSortableTextureSizesPointers[index]->baseIndex;
}

unsigned short  gTextureAtlas::getTextureWidthBySortedOrder(unsigned int index) const
{
	if ((index > m_texturesNum) || (m_pSortableTextureSizes == 0))
		return 0;

	return m_pSortableTextureSizesPointers[index]->width;
}

unsigned short  gTextureAtlas::getTextureHeightBySortedOrder(unsigned int index) const
{
	if ((index > m_texturesNum) || (m_pSortableTextureSizes == 0))
		return 0;

	return m_pSortableTextureSizesPointers[index]->height;
}
unsigned short  gTextureAtlas::getTextureRemapedXPosBySortedOrder(unsigned int index) const
{
	if ((index > m_texturesNum) || (m_pSortableTextureSizes == 0))
		return 0;

	return m_pSortableTextureSizesPointers[index]->remappedX;
}

unsigned short  gTextureAtlas::getTextureRemapedYPosBySortedOrder(unsigned int index) const
{
	if ((index > m_texturesNum) || (m_pSortableTextureSizes == 0))
		return 0;

	return m_pSortableTextureSizesPointers[index]->remappedY;
}

void* gTextureAtlas::getUserDataBySortedIndex(unsigned int index) const
{
	if ((index > m_texturesNum) || (m_pSortableTextureSizes == 0))
		return 0;

	return m_pSortableTextureSizesPointers[index]->userData;
}


unsigned short gTextureAtlas::getTextureWidthByBaseIndex(unsigned int baseIndex) const
{
	if ((baseIndex > m_texturesNum) || (m_pSortableTextureSizes == 0))
		return 0;

	return m_pSortableTextureSizes[baseIndex].width;
}
unsigned short gTextureAtlas::getTextureHeightByBaseIndex(unsigned int baseIndex) const
{
	if ((baseIndex > m_texturesNum) || (m_pSortableTextureSizes == 0))
		return 0;
	return m_pSortableTextureSizes[baseIndex].height;
}

unsigned short gTextureAtlas::getTextureRemapedXPosByBaseIndex(unsigned int baseIndex) const
{
	if ((baseIndex > m_texturesNum) || (m_pSortableTextureSizes == 0))
		return 0;
	return  m_pSortableTextureSizes[baseIndex].remappedX;
}

unsigned short gTextureAtlas::getTextureRemapedYPosByBaseIndex(unsigned int baseIndex) const
{
	if ( (baseIndex > m_texturesNum) || ( m_pSortableTextureSizes == 0 ) )
		return 0;
	return  m_pSortableTextureSizes[baseIndex].remappedY;
}

/*
const void* gTextureAtlas::getTextureUserData(unsigned int baseIndex) const
{
	if ((baseIndex > m_texturesNum) || (m_pSortableTextureSizes == 0))
		return 0;

	return m_pSortableTextureSizes[baseIndex].userData;
}
*/

void gTextureAtlas::_reset()
{
	if (m_pSortableTextureSizes)
		delete[] m_pSortableTextureSizes;
	m_pSortableTextureSizes = 0;

	if (m_pSortableTextureSizesPointers)
		delete[] m_pSortableTextureSizesPointers;
	m_pSortableTextureSizesPointers = 0;

	m_texturesNum = 0;
	m_curPosInArray = 0;
	m_atlasWidth = 0;
	m_atlasHeight = 0;
	m_atlasWidth = 0;
	m_atlasHeight = 0;
}