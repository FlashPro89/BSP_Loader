#include "Terrain.h"
#include <stdio.h>


gResourceTerrain::gResourceTerrain( gResourceManager* mgr, gResourceGroup group, const char* filename, const char* name )
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
	m_materialsNum = 0;
	m_trisNum = 0;
	m_pTex = 0;
	m_pTexDetail = 0;
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
	m_pTex = (gResource2DTexture*)m_rmgr->loadTexture2D(fullFileName, "terrainTex");
	
	sprintf_s(fullFileName, FBUFFSZ, "%s%s", dirName, texDetailFileName);
	m_pTexDetail = (gResource2DTexture*)m_rmgr->loadTexture2D(fullFileName, "terrainDetailTex");

	m_hMapFilename = dirName;
	m_hMapFilename+= hMapFilename;

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
	
	if (m_pHMap)
	{
		delete[] m_pHMap;
	}

	m_pHMap = 0;
	m_isLoaded = false;
}

void gResourceTerrain::onFrameRender(const D3DXMATRIX& transform) const
{
	if (!m_isLoaded)
		return;

	LPDIRECT3DDEVICE9 pD3DDev = m_rmgr->getDevice();
	if (!pD3DDev)
		return;
	
	pD3DDev->SetTransform( D3DTS_WORLD, &transform );
	if (m_pTex)
		pD3DDev->SetTexture(0, m_pTex->getTexture());
	if(m_pTexDetail)
		pD3DDev->SetTexture(1, m_pTexDetail->getTexture()); //detail sampler
	
	pD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);

	pD3DDev->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(gTerrainVertex));
	pD3DDev->SetIndices(m_pIndexBuffer);
	pD3DDev->SetFVF( GTERRAIN_FVF );

	float fogDensity = 0.0005f;
	float fogStart = 5100.5f;
	float fogEnd = 6010.8f;

	//pD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);

	pD3DDev->SetRenderState(D3DRS_FOGENABLE, true);
	pD3DDev->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_EXP2 );
	pD3DDev->SetRenderState(D3DRS_FOGCOLOR, 0x007F7F7F);
	pD3DDev->SetRenderState(D3DRS_FOGDENSITY, *(DWORD*)(&fogDensity) );
	pD3DDev->SetRenderState(D3DRS_RANGEFOGENABLE, true);
	//pD3DDev->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&fogStart));
	//pD3DDev->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&fogEnd));
	

	pD3DDev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, (m_width - 1) * (m_depth - 1) * 4,
		0, (m_width-1) * (m_depth-1) * 2 );
	pD3DDev->SetRenderState(D3DRS_FOGENABLE, false);

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
	LPDIRECT3DDEVICE9 pDev = m_rmgr->getDevice();
	if (!pDev)
		return false;

	gTerrainVertex* vdata = 0;
	HRESULT hr = 0;
	unsigned int i = 0, j = 0;
	unsigned int* idata = 0;

	//create vbuffer
	hr = pDev->CreateVertexBuffer((m_width-1) * (m_depth-1) * 4 * sizeof(gTerrainVertex), D3DUSAGE_WRITEONLY, 0,
		D3DPOOL_DEFAULT, &m_pVertexBuffer, 0);

	if (FAILED(hr))
		return false;

	hr = m_pVertexBuffer->Lock(0, (m_width - 1) * (m_depth - 1) * 4 * sizeof(gTerrainVertex), (void**)& vdata, 0);
	if (FAILED(hr))
	{
		releaseBuffers();
		return false;
	}

	//for origin rotation
	D3DXVECTOR3 tmpVec[4];
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
	for (i = 0; i < (m_width-1); i++)
	{
		for (j = 0; j < (m_depth-1); j++)
		{
			int p = (i * (m_width-1) + j ) * 4;
			for (int x = 0; x < 2; x++)
			{
				for (int z = 0; z < 2; z++)
				{
					int offset = x * 2 + z;
					tmpVec[offset].x = (i+x) * m_widthScaler;
					tmpVec[offset].y = (float)(m_minHeight + ((float)m_pHMap[ (i+x) * m_width +  (j + z) ] / 65536.0f) * m_maxHeight);
					tmpVec[offset].z = (j+z) * m_depthScaler;

					D3DXVec3TransformCoord(&tmpVec[offset], &tmpVec[offset], &mrot);

					vdata[p + offset].x = tmpVec[offset].x + m_originPos.x;
					vdata[p + offset].y = tmpVec[offset].y + m_originPos.y;
					vdata[p + offset].z = tmpVec[offset].z + m_originPos.z;

					vdata[p + offset].u0 = (float)(j + z) / (float)m_width * m_textureCoordScale;
					vdata[p + offset].v0 = (float)(i + x) / (float)m_depth * m_textureCoordScale;
					vdata[p + offset].u1 = ((float)(j + z) / (float)m_width) * m_detailCoordScale;
					vdata[p + offset].v1 = ((float)(i + x) / (float)m_depth) * m_detailCoordScale;

					m_AABB.addPoint(D3DXVECTOR3(vdata[p + offset].x, vdata[p + offset].y, vdata[p + offset].z));
				
					//----------------------------------------------------
					//Smooth normals
				
				}
			}
		}
	}
/*
	FILE* fd = 0;
	fopen_s(&fd, "out_vb.txt", "wt");
	for (unsigned int i = 0; i < (m_width-1) * (m_depth-1) * 4; i++)
	{
		fprintf_s( fd, "vertex №%i: xyz: %f %f %f\n", i, vdata[i].x, vdata[i].y, vdata[i].z);
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

	// проходим по всем вершинам(крове крайних справа и снизу), связывая их в треугольники 
	D3DXVECTOR3 normal, p0, p1, p2;

	for (i = 0; i < (m_width-1); i++)
	{
		// Порядок прорисовки треугольников для каждого квада: ??? уже не так
		//	    1__2	1__X
		//		| /|	| /|
		//		|/ |	|/ |
		//	    Х--3	3--2

		for (j = 0; j < (m_depth-1); j++)
		{
			int p = ( j + (m_width-1) * i) * 6;
			int v = (j + (m_width-1) * i) * 4;

			idata[p] = v;
			idata[p+1] = v+1;
			idata[p+2] = v+2;
			
			//----------------------------------------------------------------------------		
			//compute triangle normal
			p0.x = vdata[v].x; p0.y = vdata[v].y; p0.z = vdata[v].z;
			p1.x = vdata[v+1].x; p1.y = vdata[v+1].y; p1.z = vdata[v+1].z;
			p2.x = vdata[v+2].x;p2.y = vdata[v+2].y; p2.z = vdata[v+2].z;

			p1 = p0 - p1; p2 = p0 - p2;
			D3DXVec3Cross(&normal, &p1, &p2);
			D3DXVec3Normalize(&normal, &normal);

			vdata[v].nx = normal.x; vdata[v].ny = normal.y; vdata[v].nz = normal.z;
			vdata[v+1].nx = normal.x; vdata[v+1].ny = normal.y; vdata[v+1].nz = normal.z;
			vdata[v+2].nx = normal.x; vdata[v+2].ny = normal.y; vdata[v+2].nz = normal.z;
			//----------------------------------------------------------------------------

			idata[p+3] = v+2;
			idata[p+4] = v+1;
			idata[p+5] = v+3;

			//----------------------------------------------------------------------------		
			//compute triangle normal
			p0.x = vdata[v+2].x; p0.y = vdata[v+2].y; p0.z = vdata[v+2].z;
			p1.x = vdata[v+1].x; p1.y = vdata[v+1].y; p1.z = vdata[v+1].z;
			p2.x = vdata[v+3].x; p2.y = vdata[v+3].y; p2.z = vdata[v+3].z;

			p1 = p0 - p1; p2 = p0 - p2;
			D3DXVec3Cross(&normal, &p1, &p2);
			D3DXVec3Normalize(&normal, &normal);

			vdata[v+2].nx = normal.x; vdata[v+2].ny = normal.y; vdata[v+2].nz = normal.z;
			vdata[v+1].nx = normal.x; vdata[v+1].ny = normal.y; vdata[v+1].nz = normal.z;
			vdata[v+3].nx = normal.x; vdata[v+3].ny = normal.y; vdata[v+3].nz = normal.z;
			//----------------------------------------------------------------------------
		}
	}
/*
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



