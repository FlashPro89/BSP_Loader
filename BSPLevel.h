#pragma once

#ifndef _BSP_LEVEL_H_
#define _BSP_LEVEL_H_

#include "Resources.h"

class gResourceBSPLevel : public gRenderable
{
public:
	gResourceBSPLevel(gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name);
	~gResourceBSPLevel();


	bool preload(); //�������� ����������� ������
	bool load(); //�������� ����������� POOL_DEFAULT
	void unload(); //������, ���������� preload() � ���� ������� �� ����������

	//void onFrameRender(const D3DXMATRIX& transform) const;
	void onFrameRender( gRenderQueue* queue, const gEntity* entity, const gCamera* cam ) const;

	void* getVBuffer() const;
	void* getIBuffer() const;

	GPRIMITIVETYPE getPrimitiveType() const;
	unsigned int getVertexStride() const;

	bool isUseUserMemoryPointer();

protected:
	gResourceBSPLevel();
	gResourceBSPLevel(gResourceBSPLevel&);
};

#endif

