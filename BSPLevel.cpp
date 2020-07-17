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

void gResourceBSPLevel::onFrameRender( gRenderQueue* queue, const gEntity* entity, const gCamera* cam ) const
{

}

void* gResourceBSPLevel::getVBuffer() const
{
	return 0;
}

void* gResourceBSPLevel::getIBuffer() const
{
	return 0;
}

GPRIMITIVETYPE gResourceBSPLevel::getPrimitiveType() const
{
	return GPRIMITIVETYPE::GPT_TRIANGLELIST;
}

unsigned int gResourceBSPLevel::getVertexStride() const
{
	return 0;
}

bool gResourceBSPLevel::isUseUserMemoryPointer()
{
	return false;
}

