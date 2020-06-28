#pragma once

#ifndef _RESOURCES_H_
#define _RESOURCES_H_

#include <d3d9.h>
#include <d3dx9.h>
#include <map>
#include <string>
#include "ColDet.h"
#include "WADFile.h"

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

enum GRESOURCEGROUP
{
	GRESGROUP_2DTEXTURE,
	GRESGROUP_CUBETEXTURE,
	GRESGROUP_SHAPE,
	GRESGROUP_SHADERSET,
	GRESGROUP_STATICMESH,
	GRESGROUP_SKINNEDMESH,
	GRESGROUP_SKINEDANIMATION,
	GRESGROUP_TERRAIN,
	GRESGROUP_TEXTDRAWER,
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

class gResource
{
public:
	gResource( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name = 0 );
	virtual ~gResource() {}

	virtual bool preload() { return true; } //загрузка статических данных
	virtual bool load() = 0;
	virtual void unload() = 0; //данные, загруженые preload() в этой функции не измен€ютс€

	//void setResourceName( const char* name );
	//void setFileName( const char* name );
	
	const char* getResourceName();
	const char* getFileName();

	GRESOURCEGROUP getGroup();
	bool isManaged() const;
	bool isRenderable() const;

	bool isLoaded() const;

protected:
	GRESOURCEGROUP m_group;
	gResourceManager* m_rmgr;
	bool m_isManaged;
	bool m_isRenderable;
	std::string m_resName;
	std::string m_fileName;
	bool m_isLoaded;
};

// ????
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

class gRenderable : public gResource
{
public:
	gRenderable( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name = 0 );
	virtual ~gRenderable() {};

	virtual void onFrameRender( const D3DXMATRIX& transform ) const {};
	virtual void onFrameMove(float delta) {};

	bool isVisible() const;
	void setVisible(bool visible);

	const gAABB& getAABB();

protected:
	bool m_isVisible;
	gAABB m_AABB;

};

class gResource2DTexture : public gResource
{
public:
	//gResource2DTexture( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name = 0 );
	gResource2DTexture( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name = 0, WADLumpInfo_t* lumpinfo = 0 );
	~gResource2DTexture();

	bool preload();//загрузка статических данных
	bool load();
	void unload(); //данные, загруженые preload() в этой функции не измен€ютс€

	const LPDIRECT3DTEXTURE9 getTexture() const;

	unsigned short getTextureWidth() const;
	unsigned short getTextureHeight() const;

protected:
	WADLumpInfo_t* m_pLumpInfo;
	//bool m_isTexFromWAD;
	LPDIRECT3DTEXTURE9 m_pTex;
	unsigned short m_width;
	unsigned short m_height;
};

class gResourceShape :  public gRenderable
{
public:
	gResourceShape( gResourceManager* mgr, GRESOURCEGROUP group, gShapeType type, const char* name );
	~gResourceShape();

	bool load();
	void unload(); //данные, загруженые preload() в этой функции не измен€ютс€

	void onFrameRender( const D3DXMATRIX& transform ) const;

	LPD3DXMESH getMesh();

	void setSizes( float height, float width, float depth, float r1, float r2 );

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
	gResourceManager( LPDIRECT3DDEVICE9* pDev );
	~gResourceManager();

	void onRenderDeviceLost();
	bool onRenderDeviceReset();

	void setWADFolder( const char* folder );
	const char* getWADFolder() const;

	gResource* loadTexture2DFromWADList( const char* name );
	gResource* loadTexture2D( const char* filename, const char* name = 0 );
	gResource* loadStaticMesh( const char* filename, const char* name = 0 );
	gResource* loadSkinnedMeshSMD(const char* filename, const char* name );
	gResource* loadTerrain( const char* filename, const char* name );
	gResource* loadSkinnedAnimationSMD( const char* filename, const char* name, gResourceSkinnedMesh* ref );
	gResource* createShape( const char* name, gShapeType type );
	gResource* createTextDrawer( const char* name, const gFontParameters& params);
	gResourceLineDrawer* getLineDrawer() const;

	const LPDIRECT3DDEVICE9 getDevice() const;
	const gResource* getResource(const char* name, GRESOURCEGROUP group) const;
	
	bool destroyResource(const char* name, GRESOURCEGROUP group);

	void unloadAllResources();

protected:

	void _clearWADFilesList();
	bool _loadWADFileHeader( const char* filename );

	std::string m_wadFolder;
	LPDIRECT3DDEVICE9* m_ppDev;
	std::map < std::string, gResource* > m_resources[ GRESGROUP_NUM ];
	gResourceLineDrawer* m_pLineDrawer;
	std::map < std::string, WADFile* > m_wadFiles;
};

#endif