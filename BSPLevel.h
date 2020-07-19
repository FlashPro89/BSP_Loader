#pragma once

#ifndef _BSP_LEVEL_H_
#define _BSP_LEVEL_H_

#include "Resources.h"
#include "BSPFile.h"

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

	bool isUseUserMemoryPointer();

protected:
	gResourceBSPLevel();
	gResourceBSPLevel(gResourceBSPLevel&);

	int m_vertsNum;
	int m_edgesNum;
	int m_surfedgesNum;
	int m_facesNum;
	int m_texinfsNum;
	int m_planesNum;
	int m_miptexsNum;
	int m_modelsNum;
	int m_nodesNum;
	int m_leafsNum;
	int m_marksurfacesNum;
	int m_clipnodesNum;

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
};

#endif

