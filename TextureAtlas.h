#pragma once

#ifndef _TEXTURE_ATLAS_H_
#define _TEXTURE_ATLAS_H_

struct gTextureSizeSortingStruct
{
	unsigned short remappedX;
	unsigned short remappedY;
	unsigned short width;
	unsigned short height;
	unsigned int baseIndex;
};

class gTextureAtlas
{
public:
	gTextureAtlas();
	~gTextureAtlas();
		
protected:
	gTextureSizeSortingStruct* m_pSortableTextureSizes;
};

#endif

