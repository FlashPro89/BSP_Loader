#pragma once

#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "ColDet.h"
#include <d3d9.h>
#include "Resources.h"

struct gTerrainVertex
{
	float x, y, z;
	float nx, ny, nz;
	float u0, v0;
	float u1, v1;
};
#define GTERRAIN_FVF ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 )

struct gDebugNormal
{
	float x, y, z;
	unsigned int color;
};

class gResourceTerrain : public gRenderable
{

public:
	gResourceTerrain(gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name);
	~gResourceTerrain();

	unsigned int getHeightMapWidth() const;
	unsigned int getHeightMapDepth() const;
	float getMinHeight() const;
	float getMaxHeight() const;

	void setTexture( gResource* texture );

	bool preload(); //загрузка статических данных
	bool load(); //загрузка видеоданных POOL_DEFAULT
	void unload(); //данные, загруженые preload() в этой функции не изменяются

	//void onFrameRender(const D3DXMATRIX& transform) const;
	void onFrameRender( gRenderQueue* queue, const gEntity* entity, const gCamera* cam ) const;

	GVERTEXFORMAT getVertexFormat();

	void* getVBuffer();
	void* getIBuffer();

	bool isUseUserMemoryPointer();

protected:
	gResourceTerrain();

	bool loadHeightMap();
	bool fillBuffers();
	void releaseBuffers();

	float m_widthScaler;
	float m_depthScaler;
	unsigned int m_width;
	unsigned int m_depth;
	float m_minHeight;
	float m_maxHeight;

	//debug
	gDebugNormal* m_normals;
	void drawNormals() const;

	D3DXVECTOR3 m_originPos;
	D3DXVECTOR3 m_originRot;
	float m_textureCoordScale;
	float m_detailCoordScale;

	unsigned short* m_pHMap;

	unsigned int m_vertexesNum;
	unsigned int m_indexesNum;
	unsigned int m_pMaterialsNum;
	unsigned int m_trisNum;

	std::string m_hMapFilename;

	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;
	LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;
	gResource2DTexture* m_pTex;
	gResource2DTexture* m_pTexDetail;
};

#endif