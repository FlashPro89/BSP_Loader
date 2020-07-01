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

void gResourceBSPLevel::onFrameRender(const D3DXMATRIX& transform) const
{

}

