#include "Resources.h"
#include "Scene.h"
#include "Mesh.h"
#include "Terrain.h"
#include <stdio.h>

iwadcolor colormap[256];
//icolor* img = new icolor[MAX_TEX_SIZE * 4];
unsigned char t[MAX_IMG_DISK_SZ];

struct gShapeVertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 norm;
};

void toUpper(char* str)
{
	int i = 0;
	while (str[i] != '\0')
	{
		str[i] = toupper(str[i]);
		i++;
	}
}

//-----------------------------------------------
//
//	CLASS: gRenderableSettings
//-----------------------------------------------
gRenderableSettings::gRenderableSettings()
{ 
	m_worldMatrixes = 0; 
	m_worldMarixesNum = 0; 
}

gRenderableSettings::~gRenderableSettings()
{

}

void gRenderableSettings::setWorldMatrixesArray( const D3DXMATRIX* matrixArray, unsigned int matrixNum )
{
	m_worldMatrixes = matrixArray;
	m_worldMarixesNum = matrixNum;
}

const D3DXMATRIX* gRenderableSettings::getWorldMatrixes() const
{
	return m_worldMatrixes;
}

const unsigned int gRenderableSettings::getWorldMatrixesNum() const
{
	return m_worldMarixesNum;
}


//-----------------------------------------------
//
//	CLASS: gRenderable
//-----------------------------------------------

gRenderable::gRenderable( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name )
	: gResource( mgr, group, filename, name)
{
	m_isVisible = true;
	m_isRenderable = true;
}

bool gRenderable::isVisible() const
{ 
	return m_isVisible;
}

void gRenderable::setVisible(bool visible)
{
	m_isVisible = visible;
}

const gAABB& gRenderable::getAABB()
{
	return m_AABB;
}

//-----------------------------------------------
//
//	CLASS: gResource
//-----------------------------------------------

gResource::gResource( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name )
{
	m_isLoaded = false;
	m_rmgr = mgr;
	m_group = group;
	m_isManaged = false;
	if( filename )
		m_fileName = filename;
	if (name)
		m_resName = name;
	else
		m_resName = m_fileName;

	m_isRenderable = false;
}

const char* gResource::getResourceName()
{
	return m_resName.c_str();
}

const char* gResource::getFileName()
{
	return m_fileName.c_str();
}

GRESOURCEGROUP gResource::getGroup()
{
	return m_group;
}

bool gResource::isManaged() const
{
	return m_isManaged;
}

bool gResource::isRenderable() const 
{
	return m_isRenderable;
}

bool gResource::isLoaded() const
{
	return m_isLoaded;
}

//-----------------------------------------------
//
//	CLASS: gResource2DTexture
//
//-----------------------------------------------

/*
gResource2DTexture::gResource2DTexture( gResourceManager* mgr, GRESOURCEGROUP group, 
										const char* filename, const char* name ) 
				: gResource( mgr, group, filename, name )
{
	m_pTex = 0;
	m_isRenderable = false;

	
	size_t sz = strlen(filename);

	if (((filename[sz - 1] == 'd') || (filename[sz - 1] == 'D')) &&
		((filename[sz - 2] == 'a') || (filename[sz - 2] == 'A')) &&
		((filename[sz - 3] == 'w') || (filename[sz - 3] == 'W')))
		m_isTexFromWAD = true;
	else
		m_isTexFromWAD = false;
	
}
*/

gResource2DTexture::gResource2DTexture(gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, 
	const char* name, WADLumpInfo_t* lumpinfo) : gResource(mgr, group, filename, name)
{
	m_pTex = 0;
	m_isRenderable = false;
	m_pLumpInfo = lumpinfo;
}

gResource2DTexture::~gResource2DTexture()
{
	unload();
}

bool gResource2DTexture::preload()
{
	if (m_pLumpInfo == 0)
	{
		D3DXIMAGE_INFO inf;
		D3DXGetImageInfoFromFile(m_fileName.c_str(), &inf);

		if (inf.ResourceType != D3DRTYPE_TEXTURE)
			return false;

		return true;
	}
	else
		return true;
}

bool gResource2DTexture::load()
{
	if (m_isLoaded)
		return true;

	if( m_pLumpInfo )
	{
		size_t sz = WADLoadLumpFromFile( m_fileName.c_str(), (void*)t, m_pLumpInfo->filepos, m_pLumpInfo->disksize );
		if ( sz != m_pLumpInfo->disksize )
			return m_isLoaded; //не удалось считать текстуру

		//colormap offset
		WADPic* pic = ((WADPic*)t);
		unsigned int w = pic->width, h = pic->height;

		int offset = w * h + (w / 2) * (h / 2) + (w / 4) * (h / 4) + (w / 8) * (h / 8) + sizeof(short);
		memcpy(colormap, t + sizeof(WADPic) + offset, 768);

		//create tex in Sys mem
		LPDIRECT3DTEXTURE9 pTexTmp = 0;
		HRESULT hr = m_rmgr->getDevice()->CreateTexture( w, h, 1, 0, 
			D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pTexTmp, 0 );

		if (FAILED(hr))  
			return m_isLoaded;

		//create tex in Video mem
		hr = m_rmgr->getDevice()->CreateTexture( w, h, 1, D3DUSAGE_AUTOGENMIPMAP, 
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTex, 0 );

		if (FAILED(hr))
		{
			pTexTmp->Release();
			return m_isLoaded;
		}
		//unsigned int y = m_pTex->GetLevelCount();

		//Lock sysmem tex
		D3DLOCKED_RECT rect;
		hr = pTexTmp->LockRect(0, &rect, 0, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			pTexTmp->Release();
			m_pTex->Release();
			return m_isLoaded;
		}

		//bitmap
		byte* data = t + pic->data[0];
		icolor* img = (icolor*)rect.pBits;
		int counter = 0;
		for (unsigned int row = 0; row < h; row++)
		{
			for (unsigned int column = 0; column < w; column++)
			{
				if (colormap[data[counter]].r == 0 &&
					colormap[data[counter]].g == 0 &&
					colormap[data[counter]].b == 255 &&
					data[counter] == 255)
				{
					img[counter].r = 0;   // Transparent
					img[counter].g = 0;
					img[counter].b = 0;
					img[counter].a = 0;
				}
				else
				{
					img[counter].r = colormap[data[counter]].r;
					img[counter].g = colormap[data[counter]].g;
					img[counter].b = colormap[data[counter]].b;
					img[counter].a = 0xFF;
				}
				counter++;
			}
		}
		pTexTmp->UnlockRect(0);

		// move texture to video mem
		hr = m_rmgr->getDevice()->UpdateTexture( pTexTmp, m_pTex );
		if (FAILED(hr))
		{
			pTexTmp->Release();
			m_pTex->Release();
			return m_isLoaded;
		}

		pTexTmp->Release();

		m_pTex->GenerateMipSubLevels();
		//D3DXSaveTextureToFile("dest.dds", D3DXIFF_DDS, m_pTex, 0);


		m_isLoaded = true;
		return m_isLoaded;

	}
	else
	{
		HRESULT hr = D3DXCreateTextureFromFileEx(m_rmgr->getDevice(), m_fileName.c_str(), D3DX_DEFAULT_NONPOW2,
			D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_FROM_FILE, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DFMT_FROM_FILE, 0, 0, 0, &m_pTex);

		if (FAILED(hr))
			return false;
		else
			m_isLoaded = true;

		//TEST:
		D3DSURFACE_DESC desc;
		m_pTex->GetLevelDesc( 0, &desc );
		D3DLOCKED_RECT dlr;
		RECT rc;
		rc.left = 0; rc.top = 0; rc.right = desc.Width - 1; rc.bottom = desc.Height - 1;

		hr = m_pTex->LockRect(0, &dlr, &rc, D3DLOCK_READONLY);

		m_pTex->UnlockRect(0);

		return m_isLoaded;
	}
}

void gResource2DTexture::unload() //данные, загруженые preload() в этой функции не измен€ютс€
{
	if (m_pTex)
		m_pTex->Release();
	m_pTex = 0;

	m_isLoaded = false;
}

const LPDIRECT3DTEXTURE9 gResource2DTexture::getTexture() const
{
	return m_pTex;
}

//-----------------------------------------------
//
//	CLASS: gResourceShape
//
//-----------------------------------------------

gResourceShape::gResourceShape(gResourceManager* mgr, GRESOURCEGROUP group, gShapeType type, const char* name )
	: gRenderable( mgr, group, 0, name )
{
	m_pMesh = 0;
	m_shType = type;
	m_width = 10.f;
	m_height = 10.f;
	m_depth = 10.f;
	m_r1 = 10.f;
	m_r2 = 20.f;
	m_isRenderable = true;
}

gResourceShape::~gResourceShape()
{
	unload();
}

bool gResourceShape::load()
{
	unload();

	HRESULT hr;
	switch (m_shType)
	{
	case GSHAPE_BOX:
		hr = D3DXCreateBox( m_rmgr->getDevice(), m_width, m_height, m_depth, &m_pMesh, 0 );
		break;
	case GSHAPE_CYLINDER:
		hr = D3DXCreateCylinder( m_rmgr->getDevice(), m_r1, m_r2, m_height, 16, 2, &m_pMesh, 0 );
		break;
	case GSHAPE_SPHERE:
		hr = D3DXCreateSphere( m_rmgr->getDevice(), m_r1, 10, 10, &m_pMesh, 0 );
		break;
	case GSHAPE_TORUS:
		hr = D3DXCreateTorus( m_rmgr->getDevice(), m_r1, m_r2, 16, 16, &m_pMesh, 0 );
		break;
	case GSHAPE_TEAPOT:
		hr = D3DXCreateTeapot( m_rmgr->getDevice(), &m_pMesh, 0 );
		break;
	default: return false;
	}

	if (SUCCEEDED(hr))
	{
		gShapeVertex* v = 0;
		hr = m_pMesh->LockVertexBuffer( D3DLOCK_READONLY, (void**)&v );

		if ( SUCCEEDED(hr) )
		{
			DWORD num_vert = m_pMesh->GetNumVertices();
			for ( DWORD i = 0; i < num_vert; i++ )
			{
				m_AABB.addPoint(v[i].pos);
			}
			return true;
		}

		hr = m_pMesh->UnlockVertexBuffer();
	}

	m_isLoaded = true;

	return m_isLoaded;
}

void gResourceShape::unload() //данные, загруженые preload() в этой функции не измен€ютс€
{
	if (m_pMesh)
		m_pMesh->Release();
	m_pMesh = 0;
	m_AABB.reset();

	m_isLoaded = false;
}

LPD3DXMESH gResourceShape::getMesh()
{
	return m_pMesh;
}

void gResourceShape::setSizes(float height, float width, float depth, float r1, float r2)
{
	m_height = height;
	m_width = width;
	m_depth = depth;
	m_r1 = r1;
	m_r2 = r2;
}

void gResourceShape::onFrameRender( const D3DXMATRIX& transform ) const
{
	if ( m_pMesh!=0 && m_rmgr->getDevice()!=0 )
	{
		m_rmgr->getDevice()->SetTransform( D3DTS_WORLD, &transform );
		m_pMesh->DrawSubset(0);
	}
}

//-----------------------------------------------
//
//	CLASS: gResourceLineDrawer
//
//-----------------------------------------------

gResourceLineDrawer::gResourceLineDrawer(gResourceManager* mgr, GRESOURCEGROUP group, const char* name)
	: gResource(mgr, group, 0, name)
{
	m_pLine = 0;
	m_isManaged = false;
}
gResourceLineDrawer::~gResourceLineDrawer()
{
	unload();
}

bool gResourceLineDrawer::load()
{
	unload();
	HRESULT hr = D3DXCreateLine( m_rmgr->getDevice(), &m_pLine );
	if (SUCCEEDED(hr)) 
		m_isLoaded = true;
	return m_isLoaded;
}

void gResourceLineDrawer::unload() //данные, загруженые preload() в этой функции не измен€ютс€
{
	if (m_pLine)
		m_pLine->Release();
	m_pLine = 0;
}

const LPD3DXLINE gResourceLineDrawer::getLine() const
{
	return m_pLine;
}

//-----------------------------------------------
//
//	CLASS: gResourceTextDrawer
//
//-----------------------------------------------


gResourceTextDrawer::gResourceTextDrawer(gResourceManager* mgr, GRESOURCEGROUP group, 
	const char* name, const gFontParameters& params)
	: gResource(mgr, group, 0, name), m_fontParams(params)
{
	m_pFont = 0;
	m_isManaged = false;
}

gResourceTextDrawer::~gResourceTextDrawer()
{
	unload();
}

bool gResourceTextDrawer::load()
{
	unload();

	HRESULT hr = D3DXCreateFont(m_rmgr->getDevice(), m_fontParams.height, m_fontParams.width, m_fontParams.weight,
		0, m_fontParams.italic, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, 0, DEFAULT_PITCH | FF_MODERN, m_fontParams.faceName.c_str(), &m_pFont);

	if( SUCCEEDED(hr) ) 
		m_isLoaded = true;
	return m_isLoaded;
}

void gResourceTextDrawer::unload() //данные, загруженые preload() в этой функции не измен€ютс€
{
	if (m_pFont)
		m_pFont->Release();
	m_pFont = 0;
	m_isLoaded = false;
}

const LPD3DXFONT gResourceTextDrawer::getFont() const
{
	return m_pFont;
}

void gResourceTextDrawer::drawInScreenSpace(const char* text, int x, int y, DWORD color, UINT viewportW, UINT viewportH ) const
{
	if (!m_isLoaded) return;
	RECT rc;
	rc.left = x, rc.top = y;
	rc.right = viewportW; rc.bottom = viewportH;

	m_pFont->DrawTextA( 0, text, -1, &rc, DT_LEFT|DT_TOP, color );
}

//-----------------------------------------------
//
//	CLASS: gResourceManager
//
//-----------------------------------------------

gResourceManager::gResourceManager( LPDIRECT3DDEVICE9* pDev )
{
	m_ppDev = pDev;

	m_pLineDrawer = new gResourceLineDrawer( this, GRESGROUP_RESERVED_1, "*_line" );
	m_resources[GRESGROUP_RESERVED_1]["*_line"] = m_pLineDrawer;
}
gResourceManager::~gResourceManager()
{
	onRenderDeviceLost();
}

void gResourceManager::onRenderDeviceLost()
{
	for (int i = 0; i < GRESGROUP_NUM; i++)
	{
		auto it = m_resources[i].begin();
		while ( it != m_resources[i].end() )
		{
			it->second->unload();
		}
	}
}

bool gResourceManager::onRenderDeviceReset()
{
	bool result = true;

	for (int i = 0; i < GRESGROUP_NUM; i++)
	{
		auto it = m_resources[i].begin();
		while ( it != m_resources[i].end() )
		{
			if ( !it->second->load() )
				result = false;
			it++;
		}
	}
	return result;
}

void gResourceManager::setWADFolder(const char* folder)
{
	this->_clearWADFilesList();
	m_wadFolder = folder;
	
	//precache wad headers
	WIN32_FIND_DATA FindFileData;
	HANDLE hf;
	
	std::string fullName = folder;
	fullName += "*.wad";

	std::string tmp;

	hf = FindFirstFile(fullName.c_str(), &FindFileData );
	if ( hf != INVALID_HANDLE_VALUE )
	{
		int count = 0;
		do
		{
			tmp = folder;
			tmp += FindFileData.cFileName;
			_loadWADFileHeader( tmp.c_str() ); // TODO: test succes of file loading
			printf("Preload WAD header: %s\n", FindFileData.cFileName);
			count++;
		} while (FindNextFile(hf, &FindFileData) != 0);
		FindClose(hf);
	}
}

const char* gResourceManager::getWADFolder() const
{
	return m_wadFolder.c_str();
}

bool gResourceManager::destroyResource(const char* name, GRESOURCEGROUP group)
{
	if ((group < 0) && (group > GRESGROUP_NUM - 1))
		return false;

	auto it = m_resources[group].find(name);

	if (it == m_resources[group].end())
		return false;
	else if (it->second != 0)
		delete it->second;
	m_resources[group].erase(it);
	return true;
}

void gResourceManager::unloadAllResources()
{
	for (int i = 0; i < GRESGROUP_NUM; i++)
	{
		auto it = m_resources[i].begin();
		while (it != m_resources[i].end())
		{
			if (it->second)
			{
				delete it->second;
				it->second = 0;
			}
			it++;
		}
		m_resources[i].clear();
	}
	_clearWADFilesList();
}

void gResourceManager::_clearWADFilesList()
{
	auto it = m_wadFiles.begin();
	while (it != m_wadFiles.end())
	{
		if (it->second) delete it->second;
		it++;
	}
	m_wadFiles.clear();
}

bool gResourceManager::_loadWADFileHeader( const char* filename )
{
	WADHeader h;
	WADLumpInfo_t* p = WADLoadFromFile ( &h, filename);

	if (!p)
		return false;

	WADFile* wad = new WADFile();
	wad->name = filename;
	wad->lumpInfo = p;
	wad->header = h;

	m_wadFiles[filename] = wad;
	return true;
}

const LPDIRECT3DDEVICE9 gResourceManager::getDevice() const
{
	return *m_ppDev;
}

gResource* gResourceManager::loadTexture2DFromWADList( const char* name )
{
	FILE* f = 0;
	std::string tmp;

	if (!strcmp("06_CHALETI_WOO", name))
		int i = 23 * 73;

	//if (!strcmp("06_CHALETI_WOO", name))
	//	fopen_s( &f, "out_06.txt", "wt" );

	auto it = m_wadFiles.begin();
	while (it != m_wadFiles.end()) 
	{
		//if (!strcmp("06_CHALETI_WOO", name))
		//	fprintf( f, "wad: %s\n", it->first.c_str());

		for (int i = 0; i < it->second->header.numlumps; i++)
		{
			toUpper( it->second->lumpInfo[i].name );
			
			//if (!strcmp("06_CHALETI_WOO", name))
			//	fprintf( f, "miptex: %s\n", it->second->lumpInfo[i].name );

			if (!strcmp(name, it->second->lumpInfo[i].name)) // текстура найдена в текущем WAD файле
			{
				gResource* pRes = new gResource2DTexture(this, GRESGROUP_2DTEXTURE, it->first.c_str(), name, &it->second->lumpInfo[i]);
				m_resources[GRESGROUP_2DTEXTURE][name] = pRes;
				pRes->preload();
				return pRes;

				/*
				size_t sz = WADLoadLumpFromFile( it->first.c_str(), (void*)t, it->second->lumpInfo[i].filepos, it->second->lumpInfo[i].disksize );
				if (sz != it->second->lumpInfo[i].disksize)
					return 0; //неудалось считать текстуру

				byte* data = t + sizeof(WADPic);

				//colormap offset
				unsigned int w = ((WADPic*)t)->width, h = ((WADPic*)t)->height;
				int offset = w * h + (w / 2) * (h / 2) + (w / 4) * (h / 4) + (w / 8) * (h / 8) + sizeof(short);
				memcpy(colormap, t + sizeof(WADPic) + offset, 768);

				//bitmap
				int counter = 0;
				for (int row = 0; row < h; row++)
				{
					for (int column = 0; column < w; column++)
					{
						if (colormap[data[counter]].r == 0 &&
							colormap[data[counter]].g == 0 &&
							colormap[data[counter]].b == 255 &&
							data[counter] == 255)
						{
							img[counter].r = 0;   // Transparent
							img[counter].g = 0;
							img[counter].b = 0;
							img[counter].a = 0;
						}
						else
						{
							img[counter].r = colormap[data[counter]].r;
							img[counter].g = colormap[data[counter]].g;
							img[counter].b = colormap[data[counter]].b;
							img[counter].a = 0xFF;
						}
						counter++;
					}
				}

				//создаем ресурс
				*/
			}
		}
		it++;
	}

	//if (!strcmp("06_CHALETI_WOO", name))
		//fclose(f);

	return 0;
}

gResource* gResourceManager::loadTexture2D( const char* filename, const char* name )
{
	gResource* pRes = new gResource2DTexture(this, GRESGROUP_2DTEXTURE, filename, name);
	if (name == 0)
		m_resources[GRESGROUP_2DTEXTURE][filename] = pRes;
	else
		m_resources[GRESGROUP_2DTEXTURE][name] = pRes;
	pRes->preload(); 
	return pRes;
}

gResource* gResourceManager::loadStaticMesh(const char* filename, const char* name)
{
	gResource* pRes = new gResourceStaticMesh(this, GRESGROUP_STATICMESH, filename, name);
	if (name == 0)
		m_resources[GRESGROUP_STATICMESH][filename] = pRes;
	else
		m_resources[GRESGROUP_STATICMESH][name] = pRes;
	pRes->preload();
	return pRes;
}

gResource* gResourceManager::loadSkinnedMeshSMD(const char* filename, const char* name )
{
	gResource* pRes = new gResourceSkinnedMesh(this, GRESGROUP_SKINNEDMESH, filename, name);
	if (name == 0)
		m_resources[GRESGROUP_SKINNEDMESH][filename] = pRes;
	else
		m_resources[GRESGROUP_SKINNEDMESH][name] = pRes;
	pRes->preload();
	return pRes;
}

gResource* gResourceManager::loadTerrain(const char* filename, const char* name)
{
	gResource* pRes = new gResourceTerrain(this, GRESGROUP_TERRAIN, filename, name);
	if (name == 0)
		m_resources[GRESGROUP_TERRAIN][filename] = pRes;
	else
		m_resources[GRESGROUP_TERRAIN][name] = pRes;
	pRes->preload();
	return pRes;
}

gResource* gResourceManager::loadSkinnedAnimationSMD( const char* filename, const char* name, gResourceSkinnedMesh* ref )
{
	gResource* pRes = new gResourceSkinAnimation( this, GRESGROUP_SKINEDANIMATION, filename, name, ref);
	if (name == 0)
		m_resources[GRESGROUP_SKINEDANIMATION][filename] = pRes;
	else
		m_resources[GRESGROUP_SKINEDANIMATION][name] = pRes;
	pRes->preload();
	return pRes;
}

gResource* gResourceManager::createShape( const char* name, gShapeType type )
{
	gResourceShape* pRes = new gResourceShape( this, GRESGROUP_SHAPE, type, name );
	m_resources[GRESGROUP_SHAPE][name] = (gResource*)pRes;
	
	return (gResource*)pRes;
}

gResource* gResourceManager::createTextDrawer( const char* name, const gFontParameters& params )
{
	gResourceTextDrawer* pRes = new gResourceTextDrawer( this, GRESGROUP_TEXTDRAWER, name, params );
	m_resources[GRESGROUP_TEXTDRAWER][name] = (gResource*)pRes;

	return (gResource*)pRes;
}

gResourceLineDrawer* gResourceManager::getLineDrawer() const
{
	return m_pLineDrawer;
}

const gResource* gResourceManager::getResource(const char* name, GRESOURCEGROUP group) const
{
	if( (group<0) && (group > GRESGROUP_NUM - 1) )
		return (gResource * )0;

	gResource* res = 0;
	auto it = m_resources[group].find(name);

	if (it != m_resources[group].end())
		res = it->second;

	return res;
}