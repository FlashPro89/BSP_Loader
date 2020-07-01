#include "BSPLevel.h"

gResourceBSPLevel::gResourceBSPLevel(gResourceManager* mgr, GRESOURCEGROUP group,
	const char* filename, const char* name) : gRenderable(mgr, group, filename, name)
{

}

gResourceBSPLevel::~gResourceBSPLevel()
{

}

bool gResourceBSPLevel::preload() //загрузка статических данных
{
	return true;
}

bool gResourceBSPLevel::load() //загрузка видеоданных POOL_DEFAULT
{
	return true;
}

void gResourceBSPLevel::unload() //данные, загруженые preload() в этой функции не изменяются
{
	
}

void gResourceBSPLevel::onFrameRender(const D3DXMATRIX& transform) const
{

}

