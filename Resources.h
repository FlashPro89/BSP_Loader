#pragma once

#ifndef _RESOURCES_H_
#define _RESOURCES_H_

#include <d3d9.h>
#include <d3dx9.h>
#include <map>
#include <string>
#include "ColDet.h"
#include "WADFile.h"
#include "RefCounter.h"

class gMaterialFactory;
class gFileSystem;
class gBMPFile;

enum  GVERTEXFORMAT
{
	GVF_RHW, // 2D overlay D3DFVF_XYZRHW | D3DFVF_TEX1

	//GVF_LIGHTMAPPED, // D3DFVF_XYZ| D3DFVF_NORMAL|D3DFVF_TEX2
	//GVF_TERRAIN,     // D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2  // same as bsp level?!?!?

	GVF_LEVEL,
	GVF_LINE,			// D3DFVF_XYZ | D3DFVF_DIFFUSE 
	GVF_STATICMESH,		 // D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1
	GVF_SHAPE,
	GVF_SKINNED0WEIGHTS, // D3DFVF_XYZB1 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX1 
	GVF_SKYBOX,         // D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0)
	GVF_RESERVED1,
	GVF_NUM
};

enum GPRIMITIVETYPE 
{
	GPT_POINTLIST, // ????
	GPT_LINELIST,
	GPT_TRIANGLELIST,
	GPT_D3DXMESH
};

DWORD getFVF( GVERTEXFORMAT fmt );
D3DPRIMITIVETYPE getPrimitiveType(GVERTEXFORMAT fmt);
unsigned int getVertexFormatStride( GVERTEXFORMAT fmt);

void toUpper(char* str);

struct gFontParameters
{
	gFontParameters(UINT _width, UINT _height, UINT _weight, bool _italic, const char* facename)
	{
		width = _width; height = _height; weight = _weight; italic = _italic; faceName = facename;
	}

	UINT width, height, weight;
	bool italic;
	std::string faceName;
};

class gEntity;
class gResourceSkinnedMesh;
class gResource2DTexture;
class gResourceCubeTexture;

enum GRESOURCEGROUP
{
	GRESGROUP_BSPLEVEL,
	GRESGROUP_TERRAIN,
	GRESGROUP_SHAPE,
	GRESGROUP_STATICMESH,
	GRESGROUP_SKINNEDMESH,
	GRESGROUP_SKINEDANIMATION,
	GRESGROUP_TEXTDRAWER,
	GRESGROUP_CUBETEXTURE,
	GRESGROUP_2DTEXTURE,
	GRESGROUP_SHADERSET,
	GRESGROUP_RESERVED_1,
	GRESGROUP_RESERVED_2,
	GRESGROUP_RESERVED_3,
	GRESGROUP_NUM
};

enum gShapeType
{
	GSHAPE_BOX,
	GSHAPE_CYLINDER,
	GSHAPE_SPHERE,
	GSHAPE_TORUS,
	GSHAPE_TEAPOT
};

class gResourceManager;

class gResource : public gReferenceCounter
{
public:
	gResource( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name = 0 );
	virtual ~gResource() {}

	void release();

	virtual bool preload() { return true; } //загрузка статических данных
	virtual bool load() = 0;
	virtual void unload() = 0; //данные, загруженые preload() в этой функции не измен€ютс€

	//void setResourceName( const char* name );
	//void setFileName( const char* name );
	
	const char* getResourceName() const;
	const char* getFileName() const;

	GRESOURCEGROUP getGroup() const;
	bool isManaged() const;
	bool isRenderable() const;

	bool isLoaded() const;

protected:
	GRESOURCEGROUP m_group;
	gResourceManager* m_pResMgr;
	bool m_isManaged;
	bool m_isRenderable;
	std::string m_resName;
	std::string m_fileName;
	bool m_isLoaded;
};


class gRenderableSettings
{
public:
	gRenderableSettings();
	virtual ~gRenderableSettings();

	void setWorldMatrixesArray( const D3DXMATRIX* matrixArray, unsigned int matrixNum);

	const D3DXMATRIX* getWorldMatrixes() const;
	const unsigned int getWorldMatrixesNum() const;

protected:
	mutable const D3DXMATRIX* m_worldMatrixes;
	unsigned int m_worldMarixesNum; //int or char??
};

class gMaterial;
class gRenderQueue;
class gCamera;

class gRenderable : public gResource
{
public:
	gRenderable( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name = 0 );
	virtual ~gRenderable();

	//virtual void onFrameRender( const D3DXMATRIX& transform ) const {};
	virtual void onFrameRender( gRenderQueue* queue, const gEntity* entity, const gCamera* camera ) const = 0;
	virtual void onFrameMove(float delta) {};

	unsigned short getId() const;

	bool isVisible() const;
	void setVisible(bool visible);

	const gAABB& getAABB();

	virtual unsigned short getDefaultMaterialsNum() const;
	virtual gMaterial* getDefaultMaterialByIndex(unsigned short subMeshIndex) const;
	virtual gMaterial* getDefaultMaterialByName(const char* name) const;

	virtual void* getVBuffer() const = 0;
	virtual void* getIBuffer() const = 0;

	virtual unsigned int getIBufferSize() const = 0;
	virtual unsigned int getVBufferSize() const = 0;

	virtual void* getBatchIBuffer() const { return 0; };
	virtual unsigned int getBatchIBufferSize() const { return 0; };

	virtual GPRIMITIVETYPE getPrimitiveType() const = 0;
	virtual unsigned int getVertexStride() const = 0;
	virtual GVERTEXFORMAT getVertexFormat() const = 0;

	virtual bool isUseUserMemoryPointer() = 0;
	
protected:
	bool m_isVisible;
	gAABB m_AABB;
	unsigned short m_resourceId;

	std::map< std::string, gMaterial* > m_defaultMatMap;
};

enum class gTextureType
{
	GTT_2DTEXTURE,
	GTT_CUBETEXTURE,
	GTT_VOLUMETEXTURE
};

class gResourceTexture : public gResource
{
public:

	gResourceTexture( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name = 0 );
	virtual ~gResourceTexture() {};

	virtual gTextureType getTextureType() const = 0;
	virtual void* getTexture() const = 0;

	virtual unsigned short getTextureWidth() const;
	virtual unsigned short getTextureHeight() const;
	virtual unsigned short getTextureDepth() const;

protected:
	unsigned short m_width;
	unsigned short m_height;
	unsigned short m_depth;
};

class gResource2DTexture : public gResourceTexture
{
public:
	//gResource2DTexture( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name = 0 );
	gResource2DTexture( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, 
		const char* name = 0, WADLumpInfo_t* lumpinfo = 0, gBMPFile* bitmap = 0 );
	~gResource2DTexture();

	gTextureType getTextureType() const;
	void* getTexture() const;

	bool preload();//загрузка статических данных
	bool load();
	void unload(); //данные, загруженые preload() в этой функции не измен€ютс€

protected:
	WADLumpInfo_t* m_pLumpInfo;
	gBMPFile* m_pBitmap;
	LPDIRECT3DTEXTURE9 m_pTex;
};

class gResourceCubeTexture : public gResourceTexture
{
public:
	gResourceCubeTexture(gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name = 0 );
	~gResourceCubeTexture();

	gTextureType getTextureType() const;
	void* getTexture() const;

	bool preload();//загрузка статических данных
	bool load();
	void unload(); //данные, загруженые preload() в этой функции не измен€ютс€

protected:
	LPDIRECT3DCUBETEXTURE9 m_pTex;
};

class gResourceShape :  public gRenderable
{
public:
	gResourceShape( gResourceManager* mgr, GRESOURCEGROUP group, gShapeType type, const char* name );
	~gResourceShape();

	bool load();
	void unload(); //данные, загруженые preload() в этой функции не измен€ютс€

	//void onFrameRender( const D3DXMATRIX& transform ) const;
	void onFrameRender( gRenderQueue* queue, const gEntity* entity, const gCamera* cam) const;

	LPD3DXMESH getMesh();

	void setSizes( float height, float width, float depth, float r1, float r2 );

	void* getVBuffer() const;
	void* getIBuffer() const;

	unsigned int getIBufferSize() const;
	unsigned int getVBufferSize() const;

	GPRIMITIVETYPE getPrimitiveType() const;
	unsigned int getVertexStride() const;

	bool isUseUserMemoryPointer();

	GVERTEXFORMAT getVertexFormat() const;

protected:
	LPD3DXMESH m_pMesh;
	gShapeType m_shType;
	float m_height, m_width, m_depth, m_r1, m_r2;
};

class gResourceLineDrawer : public gResource
{
public:
	gResourceLineDrawer( gResourceManager* mgr, GRESOURCEGROUP group, const char* name );
	~gResourceLineDrawer();

	bool load();
	void unload(); //данные, загруженые preload() в этой функции не измен€ютс€

	const LPD3DXLINE getLine() const;

protected:
	LPD3DXLINE m_pLine;
};

class gResourceTextDrawer : public gResource
{
public:
	gResourceTextDrawer(gResourceManager* mgr, GRESOURCEGROUP group, const char* name, const gFontParameters& params);
	~gResourceTextDrawer();

	bool load();
	void unload(); //данные, загруженые preload() в этой функции не измен€ютс€

	const LPD3DXFONT getFont() const;

	void drawInScreenSpace( const char* text, int x, int y, DWORD color, UINT viewportW, UINT viewportH ) const;

protected:
	LPD3DXFONT m_pFont;
	gFontParameters m_fontParams;
};

class gResourceManager
{
public:
	gResourceManager( LPDIRECT3DDEVICE9* pDev, gMaterialFactory* pMaterialFactory, gFileSystem* pFileSystem );
	~gResourceManager();

	void onRenderDeviceLost();
	bool onRenderDeviceReset();

	void setWADFolder( const char* folder );
	const char* getWADFolder() const;

	gResource* loadTexture2DFromWADList( const char* name );
	gResource* loadTextureFromBitmap( gBMPFile* bitmap, const char* name );
	gResource* loadTexture2D( const char* filename, const char* name = 0 );
	gResource* loadTextureCube(const char* filename, const char* name = 0);
	gResource* loadStaticMeshSMD( const char* filename, const char* name = 0 );
	gResource* loadSkinnedMeshSMD( const char* filename, const char* name );
	gResource* loadTerrain( const char* filename, const char* name );
	gResource* loadBSPLevel( const char* filename, const char* name );
	gResource* loadSkinnedAnimationSMD( const char* filename, const char* name, gResourceSkinnedMesh* ref );
	gResource* createShape( const char* name, gShapeType type );
	gResource* createTextDrawer( const char* name, const gFontParameters& params);
	gResourceLineDrawer* getLineDrawer() const;

	const LPDIRECT3DDEVICE9 getDevice() const;
	const gResource* getResource(const char* name, GRESOURCEGROUP group) const;
	gMaterialFactory* getMaterialFactory() const;
	gFileSystem* getFileSystem() const;
	
	bool destroyResource(const char* name, GRESOURCEGROUP group);
	void unloadAllResources();

	unsigned int _incrementRenderableIdCounter();

protected:

	void _clearWADFilesList();
	bool _loadWADFileHeader( const char* filename );

	gMaterialFactory* m_pMatFactory;
	gFileSystem* m_pFileSystem;
	std::string m_wadFolder;
	LPDIRECT3DDEVICE9* m_ppDev;
	std::map < std::string, gResource* > m_resources[ GRESGROUP_NUM ];
	gResourceLineDrawer* m_pLineDrawer;
	std::map < std::string, WADFile* > m_wadFiles;

	unsigned int m_nextRenderableId;
};

#endif