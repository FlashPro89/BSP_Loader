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
		x = _x; y = _y; z = _z; tu = _tu; tv = _tv; tw = _tw;
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
	m_AABB.setMaxBounds(99999.f, 99999.f, 99999.f);
	m_AABB.setMinBounds(-99999.f, -99999.f, -99999.f);
}

gResourceSkyBox::~gResourceSkyBox()
{
	unload();
}

bool gResourceSkyBox::preload()
{
	//разделный порядок загрузки:
	//pos_Z = right
	//neg_Z = left
	//pos_X = front
	//neg_X = back
	//pos_Y = up
	//pos_Z = down

	//TODO: apply actual resource name
	char matName[1024] = "";
	sprintf_s( matName, 1024, "%s_mat", m_resName.c_str() );

	gMaterial* pMat = m_pResMgr->getMaterialFactory()->getMaterial(matName);
	if (!pMat)
	{
		//create materials
		char texName[1024] = "";
		sprintf_s( texName, 1024, "%s_tex", m_resName.c_str() );

		//пробуем грузить из dds
		gResourceTexture* pTex = (gResourceTexture*)m_pResMgr->loadTextureCube( m_fileName.c_str(), texName );
		if (pTex)
		{
			pTex->addRef();
		}
		else // составляем кубмап из 6 частей
		{
			gResourceTexture* pTex = (gResourceTexture*)m_pResMgr->loadTextureCube(m_fileName.c_str(), texName);
		}

		pMat = m_pResMgr->getMaterialFactory()->createMaterial( matName );
		pMat->setTexture(0, pTex);
		pMat->setLightingEnable(false);
		//pMat->setZWriteEnable(false);
	}
	else
	{
		//use existed mat
		pMat->addRef();
	}
	m_defaultMatMap[matName] = pMat;

	return true;
}

bool gResourceSkyBox::load()
{
	const float s = 0.99999f;
	gSkyBoxVertex cubeVerts[8] =
	{ 
		gSkyBoxVertex( -s, -s, -s,	-1.f, -1.f, -1.f ), //0
		gSkyBoxVertex(  s, -s, -s,	 1.f, -1.f, -1.f ), //1
		gSkyBoxVertex(  s,  s, -s,	 1.f,  1.f, -1.f ), //2
		gSkyBoxVertex( -s,  s, -s,	-1.f,  1.f, -1.f ), //3

		gSkyBoxVertex( -s, -s,  s,	-1.f, -1.f,  1.f ), //4
		gSkyBoxVertex(  s, -s,  s,	 1.f, -1.f,  1.f ), //5
		gSkyBoxVertex(  s,  s,  s,	 1.f,  1.f,  1.f ), //6
		gSkyBoxVertex( -s,  s,  s,	-1.f,  1.f,  1.f ), //7
	};

	unsigned short cubeIndexes[36] =
	{
		0, 1, 2,  0, 2, 3,  // back
		4, 6, 5,  4, 7, 6,  // front

		1, 5, 6,  1, 6, 2,  // right
		0, 7, 4,  0, 3, 7,  // left 

		3, 2, 6,  3, 6, 7,  // up
		0, 5, 1,  0, 4, 5   // down
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
	float det;
	D3DXMatrixInverse( &m_tranformMatrixes[0], &det, &camera->getViewProjMatrix());

	D3DXQUATERNION qrot;
	//D3DXQuaternionInverse(&qrot, &camera->getOrientation());
	qrot = camera->getOrientation();
	D3DXMatrixRotationQuaternion( &m_tranformMatrixes[1], &qrot );

	auto it = m_defaultMatMap.begin();
	gMaterial* pMaterial = it->second;

	gRenderElement element = gRenderElement( this, pMaterial, 0, 2, m_tranformMatrixes, 0, 12, 8 );
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
