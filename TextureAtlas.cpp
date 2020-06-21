#include "TextureAtlas.h"

//-----------------------------------------------
//
//	CLASS: gTextureAtlas
//
//-----------------------------------------------

gTextureAtlas::gTextureAtlas()
{
	m_pSortableTextureSizes = 0;
}

gTextureAtlas::~gTextureAtlas()
{
	if (m_pSortableTextureSizes)
		delete m_pSortableTextureSizes;
}