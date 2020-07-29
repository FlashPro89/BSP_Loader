#include "Terrain.h"
#include "Scene.h"
#include <stdio.h>


gResourceTerrain::gResourceTerrain( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name )
	: gRenderable(mgr, group, filename, name)
{
	m_pHMap = 0;
	m_width = 0;
	m_depth = 0;
	m_minHeight = 0;
	m_maxHeight = 0;

	m_widthScaler = 1.f;
	m_depthScaler = 1.f;
	m_textureCoordScale = 1.f;
	m_detailCoordScale = 10.f;

	m_pIndexBuffer = 0;
	m_pVertexBuffer = 0;

	m_vertexesNum = 0;
	m_indexesNum = 0;
	m_pMaterialsNum = 0;
	m_trisNum = 0;
	m_pTex = 0;
	m_pTexDetail = 0;
	m_normals = 0;
}

gResourceTerrain::~gResourceTerrain()
{
	unload();
}

unsigned int gResourceTerrain::getHeightMapWidth() const
{
	return m_width;
}

unsigned int gResourceTerrain::getHeightMapDepth() const
{
	return m_depth;
}

float gResourceTerrain::getMinHeight() const
{
	return m_minHeight;
}

float gResourceTerrain::getMaxHeight() const
{
	return m_maxHeight;
}

void gResourceTerrain::setTexture(gResource* texture)
{
	m_pTex = (gResource2DTexture*)texture;
}

#define FBUFFSZ 1024
#define TBUFFSZ 256
#define EBUFFSZ 64

#define BEGIN_SERIALIZATION char buff[FBUFFSZ] = ""; char equation[EBUFFSZ]="";
#define FIND_TOKEN(s) do{sscanf_s(buff,"%s",equation,EBUFFSZ);if (!strncmp(equation, s, sizeof(s)))break;}while(fgets(buff, FBUFFSZ, f));
#define BEGIN_FIELDS FIND_TOKEN("{") while (fgets(buff, FBUFFSZ, f)){sscanf_s(buff, "%s", equation, EBUFFSZ);if(!strncmp(equation, "}", 1))break;
#define GET_S_FIELD( s, f, f_sz ) if (!strncmp(equation, s, sizeof(s))) sscanf_s(buff, "%s \"%[^\"]\"", equation, EBUFFSZ, f, f_sz);
#define GET_F_FIELD( s, f  ) if (!strncmp(equation, s, sizeof(s)))sscanf_s(buff,"%s %f", equation, EBUFFSZ, &f );
#define GET_I_FIELD( s, f  ) if (!strncmp(equation, s, sizeof(s)))sscanf_s(buff,"%s %i", equation, EBUFFSZ, &f );
#define GET_V_FIELD( s, f  ) if (!strncmp(equation, s, sizeof(s)))sscanf_s(buff,"%s %f %f %f", equation, EBUFFSZ, &f.x, &f.y, &f.z);
#define END_FIELDS }

#define BEGIN_SECTION(s) sscanf_s(buff, "%s", equation, EBUFFSZ);if(!strncmp(equation, s, sizeof(s))){FIND_TOKEN("{") do{ if(!strncmp(equation,"}",1))break;
#define END_SECTION }while(fgets(buff, FBUFFSZ, f));}

#define BEGIN_FILE(fl)	FILE* f=0;errno_t err=fopen_s(&f,fl,"rt");if((err!= 0)||(f==0))return false;while(fgets(buff, FBUFFSZ, f)){
#define END_FILE }fclose(f);

bool gResourceTerrain::preload() //загрузка статических данных
{
	char hMapFilename[TBUFFSZ] = "";
	char texFileName[TBUFFSZ] = "";
	char texDetailFileName[TBUFFSZ] = "";

	D3DXVECTOR3 pos, rot;

	BEGIN_SERIALIZATION
	BEGIN_FILE(m_fileName.c_str())
		BEGIN_SECTION("terrain")
			BEGIN_SECTION("heightmap")
				BEGIN_FIELDS
					GET_S_FIELD("file", hMapFilename, TBUFFSZ)
					GET_I_FIELD("width", m_width)
					GET_I_FIELD("depth", m_depth)
					GET_F_FIELD("height_min", m_minHeight)
					GET_F_FIELD("height_max", m_maxHeight)
					GET_F_FIELD("width_scaler", m_widthScaler)
					GET_F_FIELD("depth_scaler", m_depthScaler)
				END_FIELDS
			END_SECTION
			BEGIN_SECTION("origin")
				BEGIN_FIELDS
					GET_V_FIELD("position", m_originPos)
					GET_V_FIELD("orientation", m_originRot)
				END_FIELDS
			END_SECTION
			BEGIN_SECTION("layers")
				BEGIN_FIELDS
					GET_F_FIELD("textureScale", m_textureCoordScale)
					GET_F_FIELD("detailScale", m_detailCoordScale)
					GET_S_FIELD("texture", texFileName, TBUFFSZ)
					GET_S_FIELD("detail", texDetailFileName, TBUFFSZ)
				END_FIELDS
			END_SECTION
		END_SECTION
	END_FILE
	
	//extract dirName
	char dirName[FBUFFSZ] = "";
	strcpy_s(dirName, FBUFFSZ - 1, m_fileName.c_str());
	unsigned int l = strlen(dirName) - 1;

	while ((dirName[l] != '/') && (l > 0)) l--;
	dirName[l + 1] = 0;								//UNSAFE !?!?

	//load textures
	char fullFileName[FBUFFSZ];
	sprintf_s(fullFileName, FBUFFSZ, "%s%s", dirName, texFileName);
	m_pTex = (gResource2DTexture*)m_pResMgr->loadTexture2D(fullFileName, "terrainTex");
	
	sprintf_s(fullFileName, FBUFFSZ, "%s%s", dirName, texDetailFileName);
	m_pTexDetail = (gResource2DTexture*)m_pResMgr->loadTexture2D(fullFileName, "terrainDetailTex");

	m_hMapFilename = dirName;
	m_hMapFilename+= hMapFilename;

	m_trisNum = (m_width - 1) * (m_depth - 1) * 2;
	m_vertexesNum = m_width * m_depth;
	
	gMaterial* pMaterial = m_pResMgr->getMaterialFactory()->getMaterial(m_resName.c_str());
	if (!pMaterial)
	{
		pMaterial = m_pResMgr->getMaterialFactory()->createMaterial(m_resName.c_str());
		pMaterial->setTexture(0, m_pTex);
		pMaterial->setTexture(1, m_pTexDetail);
	}
	else
	{
		pMaterial->addRef();
	}

	m_defaultMatMap[m_resName.c_str()] = pMaterial;

	return true;
}

bool gResourceTerrain::load() //загрузка видеоданных POOL_DEFAULT
{
	m_AABB.reset();

	if (!loadHeightMap())
		return false;
	if (!fillBuffers())
		return false;

	m_isLoaded = true;
	return true;
}

void gResourceTerrain::unload()
{
	releaseBuffers();
	
	if (m_normals)
		delete[] m_normals;
	m_normals = 0;

	if (m_pHMap)
	{
		delete[] m_pHMap;
	}

	m_pHMap = 0;
	m_isLoaded = false;
}

void gResourceTerrain::onFrameRender(gRenderQueue* queue, const gEntity* entity, const gCamera* cam) const
{
	if (!m_isLoaded)
		return;

	LPDIRECT3DDEVICE9 pD3DDev = m_pResMgr->getDevice();
	if (!pD3DDev)
		return;
	
	//pD3DDev->SetTransform( D3DTS_WORLD, &entity->getHoldingNode()->getAbsoluteMatrix() );
	//if (m_pTex)
	//	pD3DDev->SetTexture(0, m_pTex->getTexture());
	//if(m_pTexDetail)
	//	pD3DDev->SetTexture(1, m_pTexDetail->getTexture()); //detail sampler
	
	//pD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);

	//pD3DDev->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(gTerrainVertex));
	//pD3DDev->SetIndices(m_pIndexBuffer);
	//pD3DDev->SetFVF( GTERRAIN_FVF );

	float fogDensity = 0.0001f;
	float fogStart = 5100.5f;
	float fogEnd = 6010.8f;

	//pD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);

	//pD3DDev->SetRenderState(D3DRS_FOGENABLE, true);
	//pD3DDev->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_EXP2 );
	//pD3DDev->SetRenderState(D3DRS_FOGCOLOR, 0x007F7F7F);
	//pD3DDev->SetRenderState(D3DRS_FOGDENSITY, *(DWORD*)(&fogDensity) );
	//pD3DDev->SetRenderState(D3DRS_RANGEFOGENABLE, true);
	//pD3DDev->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&fogStart));
	//pD3DDev->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&fogEnd));
	
	const D3DXMATRIX& matrix = entity->getHoldingNode()->getAbsoluteMatrix();
	unsigned short distance = cam->getDistanceToPointUS(D3DXVECTOR3(matrix._41, matrix._42, matrix._43));


	gMaterial* entMat = entity->getMaterial(0);
	if (entMat == 0)
		entMat = m_defaultMatMap.begin()->second; //very unsafe??

	gRenderElement re( this, entMat, distance, 1, &matrix, 0, m_trisNum, m_vertexesNum );
	queue->pushBack(re);

	//позже удалить
	//pD3DDev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_vertexesNum,
	//	0, m_trisNum );

	//pD3DDev->SetRenderState(D3DRS_FOGENABLE, false);

	//drawNormals();
}

GVERTEXFORMAT gResourceTerrain::getVertexFormat() const
{
	return GVF_LEVEL;
}

void* gResourceTerrain::getVBuffer() const
{
	return m_pVertexBuffer;
}

void* gResourceTerrain::getIBuffer() const
{
	return m_pIndexBuffer;
}

unsigned int gResourceTerrain::getIBufferSize() const
{
	return (m_width - 1) * (m_depth - 1) * 6 * sizeof(int);
}

unsigned int gResourceTerrain::getVBufferSize() const
{
	return m_width* m_depth * sizeof(gTerrainVertex);
}

GPRIMITIVETYPE gResourceTerrain::getPrimitiveType() const
{
	return GPRIMITIVETYPE::GPT_TRIANGLELIST;
}
unsigned int gResourceTerrain::getVertexStride() const
{
	return sizeof(gTerrainVertex);
}

bool gResourceTerrain::isUseUserMemoryPointer()
{
	return false;
}

void gResourceTerrain::drawNormals() const
{
	if (!m_normals)
		return;

	LPDIRECT3DDEVICE9 pD3DDev9 = m_pResMgr->getDevice();
	if (!pD3DDev9)
		return;

	DWORD lastFVF;
	pD3DDev9->GetFVF(&lastFVF);
	pD3DDev9->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);
	pD3DDev9->SetTexture(0, 0);
	//pD3DDev9->SetRenderState(D3DRS_ZENABLE, false);

	DWORD oldLightingState;
	pD3DDev9->GetRenderState(D3DRS_LIGHTING, &oldLightingState);
	pD3DDev9->SetRenderState(D3DRS_LIGHTING, false);

	pD3DDev9->DrawPrimitiveUP( D3DPT_LINELIST, m_width*m_depth, (void*)m_normals, sizeof(gDebugNormal) );

	pD3DDev9->SetRenderState(D3DRS_LIGHTING, oldLightingState);
	pD3DDev9->SetFVF(lastFVF);
	//pD3DDev9->SetRenderState(D3DRS_ZENABLE, true);
}

bool gResourceTerrain::loadHeightMap()
{
	//load heightmap from RAW file
	FILE* f = 0;
	errno_t err;
	size_t sz, hmap_size = m_width * m_depth * sizeof(short);

	unload();

	err = fopen_s(&f, m_hMapFilename.c_str(), "rb");
	if ( (err != 0) || (f==0) )
		return false;

	m_pHMap = new unsigned short[hmap_size];

	sz = fread(m_pHMap, 1, hmap_size, f);

	if (sz != hmap_size)
	{
		delete[] m_pHMap;
		return false;
	}
	fclose(f);

	return true;
}


bool gResourceTerrain::fillBuffers()
{
	LPDIRECT3DDEVICE9 pDev = m_pResMgr->getDevice();
	if (!pDev)
		return false;

	gTerrainVertex* vdata = 0;
	HRESULT hr = 0;
	unsigned int i = 0, j = 0;
	unsigned int* idata = 0;

	//create vbuffer
	hr = pDev->CreateVertexBuffer(m_width * m_depth * sizeof(gTerrainVertex), D3DUSAGE_WRITEONLY, 0,
		D3DPOOL_DEFAULT, &m_pVertexBuffer, 0);

	if (FAILED(hr))
		return false;

	hr = m_pVertexBuffer->Lock(0, m_width * m_depth * sizeof(gTerrainVertex), (void**)& vdata, 0);
	if (FAILED(hr))
	{
		releaseBuffers();
		return false;
	}

	//for origin rotation
	D3DXVECTOR3 tmpVec;
	D3DXMATRIX mrot;
	D3DXMATRIX mrot_x;
	D3DXMATRIX mrot_y;
	D3DXMATRIX mrot_z;

	D3DXMatrixRotationX(&mrot_x, m_originRot.x);
	D3DXMatrixRotationY(&mrot_y, m_originRot.y);
	D3DXMatrixRotationZ(&mrot_z, m_originRot.z);

	D3DXMatrixMultiply(&mrot, &mrot_x, &mrot_y);
	D3DXMatrixMultiply(&mrot, &mrot, &mrot_z);

	//load to vbuffer
	for (unsigned int j = 0; j < m_depth; j++)
	{
		for (unsigned int i = 0; i < m_width; i++)
		{
			int p = i + m_width * j;

			tmpVec.x = i * m_widthScaler;
			tmpVec.y = (float)(m_minHeight + ((float)m_pHMap[p] / 65536.0f) * m_maxHeight);
			tmpVec.z = j * m_depthScaler;

			D3DXVec3TransformCoord(&tmpVec, &tmpVec, &mrot);

			vdata[p].x = tmpVec.x + m_originPos.x;
			vdata[p].y = tmpVec.y + m_originPos.y;
			vdata[p].z = tmpVec.z + m_originPos.z;

			vdata[p].u0 = ((float)j / (float)m_width) * m_textureCoordScale;
			vdata[p].v0 = ((float)i / (float)m_depth) * m_textureCoordScale;
			vdata[p].u1 = ((float)j / (float)m_width) * m_detailCoordScale;
			vdata[p].v1 = ((float)i / (float)m_depth) * m_detailCoordScale;

			m_AABB.addPoint(D3DXVECTOR3(vdata[p].x, vdata[p].y, vdata[p].z));
		}
	}
	/* DEBUG OUTPUT
		FILE* fd = 0;
		fopen_s(&fd, "out_vb.txt", "wt");
		for (unsigned int i = 0; i < m_width * m_depth; i++)
		{
			fprintf_s( fd, "vertex №%i: xyz: %f %f %f\n", i, vdata[i].x, vdata[i].y, vdata[i].z);
		}
		fclose(fd);
	*/

	//----------------------------------------------------
	//	Smooth normals
	//-----------------------------------------------------

	D3DXVECTOR3 vCenter(0.f, 0.f, 0.f);
	D3DXVECTOR3 vUpper(0.f, 0.f, 0.f);
	D3DXVECTOR3 vLower(0.f, 0.f, 0.f);
	D3DXVECTOR3 vLeft(0.f, 0.f, 0.f);
	D3DXVECTOR3 vRight(0.f, 0.f, 0.f);
	D3DXVECTOR3 vNormal(0.f, 0.f, 0.f);
	D3DXVECTOR3 vTmp1(0.f, 0.f, 0.f);
	D3DXVECTOR3 vTmp2(0.f, 0.f, 0.f);

	m_normals = new gDebugNormal[m_width * m_depth *2];

	for (unsigned int j = 0; j < m_depth; j++)
	{
		for (unsigned int i = 0; i < m_width; i++)
		{
			int p = i + m_width * j;
			vCenter = D3DXVECTOR3(vdata[p].x, vdata[p].y, vdata[p].z);

			//LOWER -----------------------------------------------
			if (j < m_depth - 1)
			{
				p = i + m_width * (j + 1);
				vLower = D3DXVECTOR3(vdata[p].x, vdata[p].y, vdata[p].z);
			}
			else
				vLower = D3DXVECTOR3(0.f, 0.f, 0.f);
			//UPPER -----------------------------------------------
			if (j > 0)
			{
				p = i + m_width * (j - 1);
				vUpper = D3DXVECTOR3(vdata[p].x, vdata[p].y, vdata[p].z);
			}
			else
				vUpper = D3DXVECTOR3(0.f, 0.f, 0.f);
			//LEFT	 -----------------------------------------------
			if (i > 0)
			{
				p = (i - 1) + m_width * j;
				vLeft = D3DXVECTOR3(vdata[p].x, vdata[p].y, vdata[p].z);
			}
			else
				vLeft = D3DXVECTOR3(0.f, 0.f, 0.f);
			//RIGHT	 -----------------------------------------------
			if (i < m_width - 1)
			{
				p = (i + 1) + m_width * j;
				vRight = D3DXVECTOR3(vdata[p].x, vdata[p].y, vdata[p].z);
			}
			else
				vRight = D3DXVECTOR3(0.f, 0.f, 0.f);

			//=============================================================	
			D3DXVECTOR3 vNull = D3DXVECTOR3(0.f, 0.f, 0.f);
	
			//LOWER
			if ((vLower != vNull))
			{
				vTmp1 = vCenter - vLower;
				if (vRight != vNull)
				{
					vTmp2 = vLower - vRight;
					D3DXVec3Cross(&vTmp2, &vTmp1, &vTmp2);

					D3DXVec3Normalize(&vTmp2, &vTmp2);
					vNormal += vTmp2;
				}
				if (vLeft != vNull)
				{
					vTmp2 =  vLeft - vLower;
					D3DXVec3Cross(&vTmp2, &vTmp1, &vTmp2 );
					D3DXVec3Normalize(&vTmp2, &vTmp2);
					vNormal += vTmp2;
				}
			}
			//UPPER
			if(vUpper!=vNull) 
			{	
				vTmp1 = vCenter - vUpper;
				if (vRight != vNull)
				{
					vTmp2 = vUpper - vRight;
					D3DXVec3Cross(&vTmp2, &vTmp2, &vTmp1);

					D3DXVec3Normalize(&vTmp2, &vTmp2);
					vNormal += vTmp2;
				}
				if (vLeft != vNull)
				{
					vTmp2 = vUpper- vLeft;
					D3DXVec3Cross(&vTmp2, &vTmp1, &vTmp2);
					D3DXVec3Normalize(&vTmp2, &vTmp2);
					vNormal += vTmp2;
				}
				
			}

			//LEFT
			if (vLeft != vNull)
			{
				vTmp1 = vCenter - vLeft;
				if (vLower != vNull)
				{
					vTmp2 = vLeft - vLower;
					D3DXVec3Cross(&vTmp2, &vTmp1, &vTmp2);
					D3DXVec3Normalize(&vTmp2, &vTmp2);
					vNormal += vTmp2;
				}
				if (vUpper != vNull)
				{
					vTmp2 = vLeft- vUpper;
					D3DXVec3Cross(&vTmp2, &vTmp2, &vTmp1);
					D3DXVec3Normalize(&vTmp2, &vTmp2);
					vNormal += vTmp2;
				}
			}

			//RIGHT
			if (vRight != vNull)
			{
				vTmp1 = vCenter - vRight;
				if (vLower != vNull)
				{
					vTmp2 = vRight - vLower;
					D3DXVec3Cross(&vTmp2, &vTmp2, &vTmp1 );
					D3DXVec3Normalize(&vTmp2, &vTmp2);
					vNormal += vTmp2;
				}
				if (vUpper != vNull)
				{
					vTmp2 =  vRight-vUpper;
					D3DXVec3Cross(&vTmp2, &vTmp1, &vTmp2);
					D3DXVec3Normalize(&vTmp2, &vTmp2);
					vNormal += vTmp2;
				}
				
				D3DXVec3Normalize(&vTmp1, &vTmp1);
				vNormal -= vTmp1;
			}

			D3DXVec3Normalize(&vNormal, &vNormal);
			p = i + m_width * j;
			
			vdata[p].nx = vNormal.x;
			vdata[p].ny = vNormal.y;
			vdata[p].nz = vNormal.z;

			m_normals[p*2].x = vdata[p].x;
			m_normals[p*2].y = vdata[p].y;
			m_normals[p*2].z = vdata[p].z;

			m_normals[p*2+1].x = vdata[p].x + vdata[p].nx * 50.f;
			m_normals[p*2+1].y = vdata[p].y + vdata[p].ny * 50.f;
			m_normals[p*2+1].z = vdata[p].z + vdata[p].nz * 50.f;

			m_normals[p*2].color = 0xFF00FF00;
			m_normals[p*2+1].color = 0xFFFF0000;
		}
	}
/* DEBUG OUTPUT
	// DEBUG OUTPUT
	FILE* fd = 0;
	fopen_s(&fd, "out_debug_norms.txt", "wt");
	for (unsigned int i = 0; i < m_width * m_depth * 2; i++)
	{
		fprintf_s( fd, "vertex №%i: xyz: %f %f %f\n", i, m_normals[i].x, m_normals[i].y, m_normals[i].z);
	}
	fclose(fd);
*/
	
	//Create Index buffer
	hr = pDev->CreateIndexBuffer((m_width - 1) * (m_depth - 1) * 6 * 4, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_pIndexBuffer, 0);
	if (FAILED(hr))
	{
		releaseBuffers();
		return false;
	}

	hr = m_pIndexBuffer->Lock(0, (m_width - 1) * (m_depth - 1) * 6 * 4, (void**)& idata, 0);
	if (FAILED(hr))
	{
		releaseBuffers();
		return false;
	}

	//-----------------------------------
	//	prepare index data
	//-----------------------------------
	
	// проходим по всем точкам карты высот(кроме крайних справа и снизу), связывая их в треугольники 
	for (unsigned int j = 0; j < m_depth - 1; j++)// проходим по всем вершинам(крове крайних справа и снизу), связывая их в треугольники 
	{
		// Порядок прорисовки треугольников для каждого квада:
		//	    1__3	1__X
		//		|\ |	|\ |
		//		| \|	| \|
		//	    Х--2	2--3
		for (unsigned int i = 0; i < m_width - 1; i++)
		{
			int p = (m_depth - 1) * j * 3 * 2 + i * 3 * 2;
			idata[p++] = m_depth * j + i;
			idata[p++] = m_depth * (j + 1) + i + 1;
			idata[p++] = m_depth * j + i + 1;

			idata[p++] = m_depth * j + i;
			idata[p++] = m_depth * (j + 1) + i;
			idata[p++] = m_depth * (j + 1) + i + 1;
		}
	}

/* DEBUG OUTPUT
	fd = 0;
	fopen_s(&fd, "out_ib.txt", "wt");
	for (unsigned int i = 0; i < (m_width-1) * (m_depth-1) * 6; i+=6)
	{
		fprintf_s( fd, "quad №%i: %i %i %i - %i %i %i\n", i/6, idata[i], idata[i+1], idata[i+2], idata[i+3], idata[i + 4], idata[i + 5]);
	}
	fclose(fd);
*/

	hr = m_pIndexBuffer->Unlock();
	if (FAILED(hr))
	{
		releaseBuffers();
		return false;
	}

	hr = m_pVertexBuffer->Unlock();
	if (FAILED(hr))
	{
		releaseBuffers();
		return false;
	}

	return true;
}

void gResourceTerrain::releaseBuffers()
{
	if (m_pIndexBuffer)
		m_pIndexBuffer->Release();
	m_pIndexBuffer = 0;

	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();
	m_pVertexBuffer = 0;
}



