#include "Skybox.h"

struct gSkyBoxVertex
{
	float x, y, z;
	float tu, tv;
};

//-----------------------------------------------
//
//	CLASS: gSkinBone
//
//-----------------------------------------------

gSkybox::gSkybox( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name )
				: gRenderable(mgr, group, filename, name)
{

}

gSkybox::~gSkybox()
{

}

void gSkybox::onFrameRender(gRenderQueue* queue, const gEntity* entity, const gCamera* camera) const
{

}

void gSkybox::onFrameMove(float delta)
{

}

void* gSkybox::getVBuffer() const
{
	return 0;
}

void* gSkybox::getIBuffer() const
{
	return 0;
}

unsigned int gSkybox::getIBufferSize() const
{
	return 0;
}

unsigned int gSkybox::getVBufferSize() const
{
	return 0;
}

void* gSkybox::getBatchIBuffer() const
{
	return 0;
}

unsigned int gSkybox::getBatchIBufferSize() const
{
	return 0;
}

GPRIMITIVETYPE gSkybox::getPrimitiveType() const
{
	return GPT_TRIANGLELIST;
}

unsigned int gSkybox::getVertexStride() const
{
	return sizeof(gSkyBoxVertex);
}

GVERTEXFORMAT gSkybox::getVertexFormat() const
{
	return GVERTEXFORMAT::GVF_SKYBOX;
}

bool gSkybox::isUseUserMemoryPointer()
{
	return false;
}
