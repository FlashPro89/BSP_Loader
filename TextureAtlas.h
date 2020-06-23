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
	unsigned int key; //sorting key
	//void* userData; //lockedRect bits ??
}; //32bytes on x86

class gTextureAtlas
{
public:
	gTextureAtlas();
	gTextureAtlas( unsigned int texturesNum );
	~gTextureAtlas();
	
	void beginAtlas( unsigned int texturesNum );
	bool pushTexture( unsigned short width, unsigned short height, void* userData = 0 );
	bool mergeToAtlas( unsigned short maxWidth, unsigned short maxHeight, unsigned char border = 0 );

	unsigned int getAtlasWidth() const;
	unsigned int getAtlasHeight() const;

	unsigned short getTextureBaseIndexInSortedOrder(unsigned int index) const;

	unsigned short getTextureWidthBySortedOrder(unsigned int index) const;
	unsigned short getTextureHeightBySortedOrder(unsigned int index) const;
	unsigned short getTextureRemapedXPosBySortedOrder(unsigned int index) const;
	unsigned short getTextureRemapedYPosBySortedOrder(unsigned int index) const;

	unsigned short getTextureWidthByBaseIndex(unsigned int baseIndex) const;
	unsigned short getTextureHeightByBaseIndex(unsigned int baseIndex) const;
	unsigned short getTextureRemapedXPosByBaseIndex( unsigned int baseIndex ) const;
	unsigned short getTextureRemapedYPosByBaseIndex( unsigned int baseIndex ) const;
	//__inline const void* getTextureUserData( unsigned int baseIndex ) const;

protected:
	gTextureSizeSortingStruct* m_pSortableTextureSizes;
	gTextureSizeSortingStruct** m_pSortableTextureSizesPointers;
	unsigned int m_texturesNum;
	unsigned int m_curPosInArray;
	unsigned int m_atlasWidth;
	unsigned int m_atlasHeight;

	void _reset();
};

#endif

