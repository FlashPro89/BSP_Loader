#pragma once

#ifndef _BSP_LEVEL_H_
#define _BSP_LEVEL_H_

#include "Resources.h"
#include "TextureAtlas.h"

struct BSPMapHeader_t;
struct BSPVertex_t;
struct BSPFace_t;
struct BSPEdge_t;
struct BSPTexinfo_t;
struct BSPPlane_t;
struct BSPModel_t;
struct BSPNode_t;
struct BSPLeaf_t;
struct BSPClipnode_t;


class gResourceBSPLevel : public gRenderable
{
public:
	gResourceBSPLevel(gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name);
	~gResourceBSPLevel();

	bool preload(); //загрузка статических данных
	bool load(); //загрузка видеоданных POOL_DEFAULT
	void unload(); //данные, загруженые preload() в этой функции не изменяются

	//void onFrameRender(const D3DXMATRIX& transform) const;
	void onFrameRender( gRenderQueue* queue, const gEntity* entity, const gCamera* cam ) const;

	void* getVBuffer() const;
	void* getIBuffer() const;

	GPRIMITIVETYPE getPrimitiveType() const;
	unsigned int getVertexStride() const;
	GVERTEXFORMAT getVertexFormat() const;

	bool isUseUserMemoryPointer();

protected:
	gResourceBSPLevel();
	gResourceBSPLevel(gResourceBSPLevel&);

	void freeMem();
	void* getLump(unsigned char lump) const;

	bool loadLightmaps();
	bool buildLightmapAtlas( unsigned int lightedFacesNum );

	//	BSP format part -----------------------------------------
	BSPMapHeader_t* m_pBSPHeader;

	unsigned int  m_bspVertsNum;
	unsigned int  m_bspEdgesNum;
	unsigned int  m_bspSurfedgesNum;
	unsigned int  m_bspFacesNum;
	unsigned int  m_bspTexinfsNum;
	unsigned int  m_bspPlanesNum;
	unsigned int  m_bspMiptexsNum;
	unsigned int  m_bspModelsNum;
	unsigned int  m_bspNodesNum;
	unsigned int  m_bspLeafsNum;
	unsigned int  m_bspMarksurfacesNum;
	unsigned int  m_bspClipnodesNum;

	BSPVertex_t* m_bspVerts;
	BSPEdge_t* m_bspEdges;
	int* m_bspSurfedges;
	BSPFace_t* m_bspFaces;
	BSPTexinfo_t* m_bspTexinfs;
	BSPPlane_t* m_bspPlanes;
	BSPModel_t* m_bspModels;
	BSPNode_t* m_bspNodes;
	BSPLeaf_t* m_bspLeafs;
	unsigned short* m_bspMarksurfaces;
	BSPClipnode_t* m_bspClipnodes;

	byte* m_bspLightData;
	byte* m_bspTexData;
	byte* m_bspVisData;
	int m_bspTexDataSize;
	int m_bspLightDataSize;
	int m_bspVisDataSize;

	int m_visLeafsNum;
	int m_visRow;

	//  Lightmaps part -----------------------------------------
	gTextureAtlas m_lMapTexAtlas;

	//	Rendering part -----------------------------------------
	unsigned int m_trisNum;
	unsigned int m_vertsNum;
};

#endif

