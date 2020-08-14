#include "Skybox.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "Materials.h"
#include "Resources.h"

#define GSB_FVF D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0)

struct gSkyBoxVertex
{
	gSkyBoxVertex(float _x, float _y, float _z, float _tu, float _tv, float _tw)
	{
		x = _x; y = _y; z = _z; tu = _tu; tv = _tv; _tw = _tw;
	}

	union
	{
		struct { float x, y, z; };
		D3DXVECTOR3 v;
	};
	union
	{
		struct { float tu, tv, tw; };
		D3DXVECTOR3 t;
	};
};

//-----------------------------------------------
//
//	CLASS: gSkinBone
//
//-----------------------------------------------

gResourceSkyBox::gResourceSkyBox( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name )
				: gRenderable(mgr, group, filename, name)
{
	m_pVB = 0;
	m_pIB = 0;
	m_pTex = 0;
}

gResourceSkyBox::~gResourceSkyBox()
{
	unload();
}

bool gResourceSkyBox::preload()
{
	return true;
}

bool gResourceSkyBox::load()
{

	gSkyBoxVertex cubeVerts[8] =
	{ 
		gSkyBoxVertex( -1.f, -1.f, -1.f,	-1.f, -1.f, -1.f ),
		gSkyBoxVertex(  1.f, -1.f, -1.f,	 1.f, -1.f, -1.f ),
		gSkyBoxVertex(  1.f, 1.f, -1.f,		 1.f, 1.f, -1.f  ),
		gSkyBoxVertex( -1.f, 1.f, -1.f,		-1.f, 1.f, -1.f  ),

		gSkyBoxVertex( -1.f, -1.f, 1.f,		-1.f, -1.f, 1.f ),
		gSkyBoxVertex(  1.f, -1.f, 1.f,		 1.f, -1.f, 1.f ),
		gSkyBoxVertex(  1.f, 1.f, 1.f,		 1.f, 1.f, 1.f  ),
		gSkyBoxVertex( -1.f, 1.f, 1.f,		-1.f, 1.f, 1.f  ),
	};

	unsigned short cubeIndexes[36] =
	{
		0, 2, 1,  0, 3, 2,  // back
		4, 5, 6,  4, 6, 7,  // front

		1, 6, 5,  1, 2, 6,  // right
		0, 4, 7,  0, 7, 3,  // left 

		3, 2, 6,  3, 6, 7,  // top
		0, 1, 5,  0, 5, 4   // bottom
	};

	HRESULT hr;
	LPDIRECT3DDEVICE9 pDev = m_pResMgr->getDevice();

	const unsigned short vbSize = sizeof(gSkyBoxVertex) * 8;
	const unsigned short ibSize = sizeof(short) * 36;

	hr = pDev->CreateVertexBuffer( vbSize, 0, GSB_FVF, D3DPOOL_DEFAULT, &m_pVB, 0);
	if (FAILED(hr))
		return false;

	hr = pDev->CreateIndexBuffer( ibSize, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, 0);
	if (FAILED(hr))
		return false;

	void* pData = 0;
	hr = m_pVB->Lock(0, vbSize, &pData, D3DLOCK_NO_DIRTY_UPDATE );
	if (FAILED(hr))
		return false;

	memcpy(pData, cubeVerts, vbSize);
	hr = m_pVB->Unlock();
	if (FAILED(hr))
		return false;

	hr = m_pIB->Lock(0, ibSize, &pData, D3DLOCK_NO_DIRTY_UPDATE );
	if (FAILED(hr))
		return false;

	memcpy( pData, cubeIndexes, ibSize );

	hr = m_pIB->Unlock();
	if (FAILED(hr))
		return false;

	//TODO: apply actual resource name
	//create material
	gResourceTexture* pTex = (gResourceTexture*)m_pResMgr->loadTextureCube(m_fileName.c_str() , "skybox_tex");

	gMaterial* pMat = m_pResMgr->getMaterialFactory()->createMaterial("skybox_mat");
	pMat->setTexture(0, pTex);
	m_defaultMatMap["skybox_mat"] = pMat;

	m_isLoaded = true;
	return true;
}

void gResourceSkyBox::unload() 
{
	if (m_pVB)
		m_pVB->Release();
	if (m_pIB)
		m_pIB->Release();
	if (m_pTex)
		m_pTex->Release();

	m_pVB = 0;
	m_pIB = 0;
	m_pTex = 0;

	m_isLoaded = false;
}

void gResourceSkyBox::onFrameRender(gRenderQueue* queue, const gEntity* entity, const gCamera* camera) const
{
	D3DXVECTOR3 vCamPos = camera->getPosition();
	D3DXMatrixTranslation( &m_invCamPosMat, -vCamPos.x, -vCamPos.y, -vCamPos.z);
	
	gMaterial* pMaterial = m_defaultMatMap.begin()->second;

	gRenderElement element = gRenderElement( this, pMaterial, 0, 1, &m_invCamPosMat, 0, 12 );
	queue->pushBack(element);
}

void gResourceSkyBox::onFrameMove(float delta)
{
}

void* gResourceSkyBox::getVBuffer() const
{
	return m_pVB;
}

void* gResourceSkyBox::getIBuffer() const
{
	return m_pIB;
}

unsigned int gResourceSkyBox::getIBufferSize() const
{
	return sizeof(short) * 36;
}

unsigned int gResourceSkyBox::getVBufferSize() const
{
	return sizeof(gSkyBoxVertex) * 8;
}

void* gResourceSkyBox::getBatchIBuffer() const
{
	return 0;
}

unsigned int gResourceSkyBox::getBatchIBufferSize() const
{
	return 0;
}

GPRIMITIVETYPE gResourceSkyBox::getPrimitiveType() const
{
	return GPT_TRIANGLELIST;
}

unsigned int gResourceSkyBox::getVertexStride() const
{
	return sizeof(gSkyBoxVertex);
}

GVERTEXFORMAT gResourceSkyBox::getVertexFormat() const
{
	return GVERTEXFORMAT::GVF_SKYBOX;
}

bool gResourceSkyBox::isUseUserMemoryPointer()
{
	return false;
}
