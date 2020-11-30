#pragma once

#ifndef _BSP_LEVEL_H_
#define _BSP_LEVEL_H_

#pragma warning ( disable : 4244 ) //conv ushort to uchar

#include "Resources.h"
#include "TextureAtlas.h"
#include <vector>

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

struct gBSPFaceBounds
{
	gBSPFaceBounds() { mins[0] = mins[1] = maxs[0] = maxs[1] = 0.f;  texsize[0] = texsize[1] = remapped[0] = remapped[1] = 0; faceIndex = -1; }
	float mins[2];
	float maxs[2];
	unsigned char texsize[2];
	unsigned short remapped[2];
	int faceIndex;
};

class gMaterial;
class gBMPFile;

struct gBSPRendingFace
{
	gBSPRendingFace() {	start_indx = num_prim = miptex = 0; pMaterial = 0; needDraw = false;}
	int start_indx;
	int num_prim;
	int miptex;
	gMaterial* pMaterial;
	bool needDraw;
};


typedef std::map< std::string, std::string >  gEntityTextBlock;
typedef std::vector< gEntityTextBlock > gEntityTextBlocks;

class gResourceBSPLevel : public gRenderable
{
public:
	gResourceBSPLevel(gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name);
	~gResourceBSPLevel();

	bool preload(); //загрузка статических данных
	bool load(); //загрузка видеоданных POOL_DEFAULT
	void unload(); //данные, загруженые preload() в этой функции не измен€ютс€

	//void onFrameRender(const D3DXMATRIX& transform) const;
	void onFrameRender( gRenderQueue* queue, const gEntity* entity, const gCamera* cam ) const;

	inline float testPointOnPlane(const D3DXVECTOR3& point, int plane)const;
	int getLeafAtPoint(const D3DXVECTOR3& point) const;
	void drawVisibleLeafs(int leaf, const gCamera& cam) const;
	bool isLeafVisible(byte* decomprPVS, int leafBit) const;
	bool isLeafInFrustum(int leaf, const gCamera& cam) const;
	void drawLeaf(int leaf) const;
	void drawFace(int face) const;

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

	const char* getSkyBoxName() const;


	//пока что public функции
	inline float _testPointOnPlane(const D3DXVECTOR3& point, int plane) const;
	int  _getClipnodeContentInPoint(const D3DXVECTOR3& point, int clipnode) const;
	int  _getClipnodeContentInBoundingSphere(const D3DXVECTOR3& point, float radius, int clipnode) const;

	const gEntityTextBlocks& _getEntityTextBlocks() const;

protected:
	gResourceBSPLevel();
	gResourceBSPLevel(gResourceBSPLevel&);

/*
	inline float _testPointOnPlane(const D3DXVECTOR3& point, int plane) const;
	int  _getClipnodeContentInPoint(const D3DXVECTOR3& point, int clipnode) const;
	int  _getClipnodeContentInBoundingSphere(const D3DXVECTOR3& point, float radius, int clipnode) const;
*/

	void* getLump(unsigned char lump) const;
	bool loadLightmaps( unsigned int lightedFacesNum );
	void buildFacePositions();
	void parceEntities();

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
	char* m_bspEntData;
	int m_bspTexDataSize;
	int m_bspLightDataSize;
	int m_bspVisDataSize;
	int m_bspEntDataSize;

	int m_visLeafsNum;
	int m_visRow;

	//  Lightmaps part -----------------------------------------
	gResource2DTexture* m_pLMapTex;
	gBMPFile* m_pBitmap;
	gBSPFaceBounds* m_faceBounds;
	unsigned short m_lMapAtlasW;
	unsigned short m_lMapAtlasH;

	//	Rendering part -----------------------------------------
	unsigned int m_trisNum;
	unsigned int m_vertsNum;
	unsigned int m_batchIBSize;
	IDirect3DVertexBuffer9* m_pVB;
	IDirect3DIndexBuffer9* m_pIB;
	IDirect3DIndexBuffer9* m_pBatchIB;
	gBSPRendingFace* m_rFaces;
	D3DXVECTOR3* m_facePositions = 0;
	gEntityTextBlocks m_entityTextBlocks;
};

#endif

