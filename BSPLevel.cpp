#include "BSPLevel.h"

gResourceBSPLevel::gResourceBSPLevel(gResourceManager* mgr, GRESOURCEGROUP group,
	const char* filename, const char* name) : gRenderable(mgr, group, filename, name)
{

}

gResourceBSPLevel::~gResourceBSPLevel()
{

}

bool gResourceBSPLevel::preload() //�������� ����������� ������
{
	return true;
}

bool gResourceBSPLevel::load() //�������� ����������� POOL_DEFAULT
{
	return true;
}

void gResourceBSPLevel::unload() //������, ���������� preload() � ���� ������� �� ����������
{
	
}

void gResourceBSPLevel::onFrameRender( gRenderQueue* queue, const gEntity* entity, const gCamera* cam ) const
{

}

void* gResourceBSPLevel::getVBuffer()
{
	return 0;
}

void* gResourceBSPLevel::getIBuffer()
{
	return 0;
}

bool gResourceBSPLevel::isUseUserMemoryPointer()
{
	return false;
}

