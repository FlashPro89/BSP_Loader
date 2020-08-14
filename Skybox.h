#pragma once

#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "Resources.h"

class gSkybox : public gRenderable
{
public:
	gSkybox( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name = 0 );
	~gSkybox();

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
};

#endif

