#pragma once

#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "Resources.h"

class gResourceSkyBox : public gRenderable
{
public:
	gResourceSkyBox( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name = 0 );
	~gResourceSkyBox();

	bool preload(); //�������� ����������� ������
	bool load(); //�������� ����������� POOL_DEFAULT
	void unload(); //������, ���������� preload() � ���� ������� �� ����������

	void onFrameRender(gRenderQueue* queue, const gEntity* entity, const gCamera* camera) const;
	void onFrameMove(float delta);

	void* getVBuffer() const;
	void* getIBuffer() const;

	unsigned int getIBufferSize() const;
	unsigned int getVBufferSize() const;

	void* getBatchIBuffer() const;
	unsigned int getBatchIBufferSize() const;

	GPRIMITIVETYPE getPrimitiveType() const;
	unsigned int getVertexStride() const;
	GVERTEXFORMAT getVertexFormat() const;

	bool isUseUserMemoryPointer();

protected:
	IDirect3DVertexBuffer9* m_pVB;
	IDirect3DIndexBuffer9* m_pIB;
	IDirect3DCubeTexture9* m_pTex;
	mutable D3DXMATRIX m_tranformMatrixes[2]; //first - world transform, second - texcoord0 transform
};

#endif

