#include "BSPLevel.h"
#include "FileSystem.h"
#include "BSPFile.h"
#include "BMPFile.h"
#include "RenderQueue.h"
#include "Scene.h"
#include "Materials.h"

#define GBSP_FVF D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2

typedef struct
{
	float x, y, z;
	float nx, ny, nz;
	float tu, tv;
	float tu2, tv2;
	//	DWORD color;
}gBSPVertex;

typedef unsigned __int16  gBSPIndex;

//-----------------------------------------------
//
//	CLASS: gResourceBSPLevel
//
//-----------------------------------------------

gResourceBSPLevel::gResourceBSPLevel(gResourceManager* mgr, GRESOURCEGROUP group,
	const char* filename, const char* name) : gRenderable(mgr, group, filename, name)
{
	m_pBSPHeader = 0;

	m_bspVertsNum = 0;
	m_bspEdgesNum = 0;
	m_bspSurfedgesNum = 0;
	m_bspFacesNum = 0;
	m_bspTexinfsNum = 0;
	m_bspPlanesNum = 0;
	m_bspMiptexsNum = 0;
	m_bspModelsNum = 0;
	m_bspNodesNum = 0;
	m_bspLeafsNum = 0;
	m_bspMarksurfacesNum = 0;
	m_bspClipnodesNum = 0;

	m_bspVerts = 0;
	m_bspEdges = 0;
	m_bspSurfedges = 0;
	m_bspFaces = 0;
	m_bspTexinfs = 0;
	m_bspPlanes = 0;
	m_bspModels = 0;
	m_bspNodes = 0;
	m_bspLeafs = 0;
	m_bspMarksurfaces = 0;
	m_bspClipnodes = 0;

	m_bspLightData = 0;
	m_bspTexData = 0;
	m_bspVisData = 0;
	m_bspTexDataSize = 0;
	m_bspLightDataSize = 0;
	m_bspVisDataSize = 0;

	m_visLeafsNum = 0;
	m_visRow = 0;

	m_trisNum = 0;
	m_vertsNum = 0;
	m_batchIBSize = 0;
	m_pVB = 0;
	m_pIB = 0;
	m_pBatchIB = 0;
	m_faceBounds = 0;
	m_rFaces = 0;

	m_lMapAtlasW = 0;
	m_lMapAtlasH = 0;

	m_facePositions = 0;
}

gResourceBSPLevel::~gResourceBSPLevel()
{
	unload();

	if (m_facePositions == 0)
		delete [] m_facePositions;
	m_facePositions = 0;

	if (m_pLMapTex)
		m_pLMapTex->release();
	m_pLMapTex = 0;

	if (m_faceBounds)
		delete[] m_faceBounds;
	m_faceBounds = 0;

	if (m_pBSPHeader)
		delete[]((byte*)m_pBSPHeader);
	m_pBSPHeader = 0;

	if (m_pBitmap)
		delete m_pBitmap;
	m_pBitmap = 0;

	if (m_rFaces)
		delete[] m_rFaces;
	m_rFaces = 0;
}

bool gResourceBSPLevel::preload() //загрузка статических данных
{
	//freeMem();

	if (!m_pResMgr->getFileSystem()->isFileExist(m_fileName.c_str() ) )
		return false;

	gFile* pFile = m_pResMgr->getFileSystem()->openFile( m_fileName.c_str(), false, true );
	if (!pFile)
		return false;

	size_t fl = pFile->getFileSize();

	m_pBSPHeader = (BSPMapHeader_t*)(new byte[fl + 1]); //? +1?
	((byte*)m_pBSPHeader)[fl] = 0;
	m_pBSPHeader->version = 0;

	size_t readed = pFile->read( (byte*)m_pBSPHeader, fl );
	delete pFile;
	if( (readed != fl ) || (m_pBSPHeader->version != BSPVERSION) )
	{
		return false;
	}

	// Little / Big Endian
	//swap bytes ?!?!? or no?

	//load verts
	m_bspVertsNum = BSPGetLumpItemsNum(m_pBSPHeader, LUMP_VERTEXES);
	m_bspVerts = (BSPVertex_t*)getLump(LUMP_VERTEXES);

	//load eges
	m_bspEdgesNum = BSPGetLumpItemsNum(m_pBSPHeader, LUMP_EDGES);
	m_bspEdges = (BSPEdge_t*)getLump(LUMP_EDGES);

	//load surfedges
	m_bspSurfedgesNum = BSPGetLumpItemsNum(m_pBSPHeader, LUMP_SURFEDGES);
	m_bspSurfedges = (int*)getLump(LUMP_SURFEDGES);

	//load faces
	m_bspFacesNum = BSPGetLumpItemsNum(m_pBSPHeader, LUMP_FACES);
	m_bspFaces = (BSPFace_t*)getLump(LUMP_FACES);

	//load texinfos
	m_bspTexinfsNum = BSPGetLumpItemsNum(m_pBSPHeader, LUMP_TEXINFO);
	m_bspTexinfs = (BSPTexinfo_t*)getLump(LUMP_TEXINFO);

	//load planes
	m_bspPlanesNum = BSPGetLumpItemsNum(m_pBSPHeader, LUMP_PLANES);
	m_bspPlanes = (BSPPlane_t*)getLump(LUMP_PLANES);

	///load models
	m_bspModelsNum = BSPGetLumpItemsNum(m_pBSPHeader, LUMP_MODELS);
	m_bspModels = (BSPModel_t*)getLump(LUMP_MODELS);

	//load leafs
	m_bspLeafsNum = BSPGetLumpItemsNum(m_pBSPHeader, LUMP_LEAFS);
	m_bspLeafs = (BSPLeaf_t*)getLump(LUMP_LEAFS);

	//load nodes
	m_bspNodesNum = BSPGetLumpItemsNum(m_pBSPHeader, LUMP_NODES);
	m_bspNodes = (BSPNode_t*)getLump(LUMP_NODES);

	//load marksurfaces
	m_bspMarksurfacesNum = BSPGetLumpItemsNum(m_pBSPHeader, LUMP_MARKSURFACES);
	m_bspMarksurfaces = (unsigned short*)getLump(LUMP_MARKSURFACES);

	//load clipnodes
	m_bspClipnodesNum = BSPGetLumpItemsNum(m_pBSPHeader, LUMP_CLIPNODES);
	m_bspClipnodes = (BSPClipnode_t*)getLump(LUMP_CLIPNODES);

	//load miptexs
	m_bspTexDataSize = m_pBSPHeader->lumps[LUMP_TEXTURES].filelen;
	m_bspTexData = (byte*)getLump(LUMP_TEXTURES);

	//load visdata
	m_bspVisDataSize = m_pBSPHeader->lumps[LUMP_VISIBILITY].filelen;
	if( m_bspVisDataSize > 0 )
	{
		m_bspVisData = (byte*)getLump(LUMP_VISIBILITY);
	}

	//load lightdata
	m_bspLightDataSize = m_pBSPHeader->lumps[LUMP_LIGHTING].filelen;
	if (m_bspLightDataSize > 0)
		m_bspLightData = (byte*)getLump(LUMP_LIGHTING);

	m_bspMiptexsNum = m_bspTexDataSize ? ((BSPMiptexlump_t*)m_bspTexData)->nummiptex : 0;

	//count leafs with PVS 
	m_visLeafsNum = 0;
	for ( unsigned int i = 0; i < m_bspLeafsNum; i++)
	{
		if (m_bspLeafs[i].visofs >= 0)
			m_visLeafsNum++;
	}
	m_visRow = ( m_visLeafsNum + 7 ) >> 3;

	
	//count triangles for vertex buffer
	unsigned short tmp[1024];
	unsigned int lightedFacesNum = 0;
	BSPFace_t* face = 0;
	BSPVertex_t* vertex = 0;
	BSPTexinfo_t* texinfo = 0;;
	int e = 0;
	float value = 0;

	m_faceBounds = new gBSPFaceBounds[m_bspFacesNum]; // need zeroMem ?

	for (unsigned int i = 0; i <m_bspFacesNum; i++)
	{
		face = &m_bspFaces[i];
		texinfo = &m_bspTexinfs[face->texinfo];

		if (i == 121)
		{
			i++; i--;
		}

		if ( face->styles[0] == 0 )
		{
			gBSPFaceBounds* pFaceBounds = &m_faceBounds[i];
			pFaceBounds->faceIndex = i;

			// compute face bounds
			pFaceBounds->mins[0] = 99999.f; pFaceBounds->mins[1] = 99999.f;
			pFaceBounds->maxs[0] = -99999.f; pFaceBounds->maxs[1] = -99999.f;

			for (int j = face->firstedge; j < face->numedges + face->firstedge; j++)
			{
				e = m_bspSurfedges[j];
				if (e >= 0)
					vertex = m_bspVerts + m_bspEdges[e].v[0];
				else
					vertex = m_bspVerts + m_bspEdges[-e].v[1];

				for (int k = 0; k < 2; k++)
				{
					value = vertex->point[0] * texinfo->vecs[k][0] +
						vertex->point[2] * texinfo->vecs[k][2] +
						vertex->point[1] * texinfo->vecs[k][1] + texinfo->vecs[k][3];

					if (value < pFaceBounds->mins[k])
						pFaceBounds->mins[k] = value;
					if (value > pFaceBounds->maxs[k])
						pFaceBounds->maxs[k] = value;
				}
			}

			for (int l = 0; l < 2; l++)
			{
				pFaceBounds->mins[l] = (float)floor(pFaceBounds->mins[l] / 16);
				pFaceBounds->maxs[l] = (float)ceil(pFaceBounds->maxs[l] / 16);

				pFaceBounds->texsize[l] = (int)(pFaceBounds->maxs[l] - pFaceBounds->mins[l] + 1);
				if (pFaceBounds->texsize[l] > 18) //17+1
					throw("Bad surface extents");
			}

			lightedFacesNum++;
		}

		//собираем неповтор€ющиес€ индексы вершин по гр€н€м
		memset(tmp, 0xFFFF, sizeof(short) * 1024);

		unsigned short vert_in_face = 0;

		int last_edge = m_bspFaces[i].firstedge + m_bspFaces[i].numedges;
		for (int j = m_bspFaces[i].firstedge; j < last_edge; j++)
		{
			bool isPresent = false;
			for (int k = 0; k < m_bspFaces[i].numedges; k++)
			{
				if (tmp[k] == m_bspEdges[abs(m_bspSurfedges[j])].v[0])
				{
					isPresent = true;
				}
			}
			if (!isPresent)
			{
				tmp[vert_in_face] = m_bspEdges[abs(m_bspSurfedges[j])].v[0];
				vert_in_face++;
			}
			isPresent = false;

			for (int k = 0; k < m_bspFaces[i].numedges; k++)
			{
				if (tmp[k] == m_bspEdges[abs(m_bspSurfedges[j])].v[1])
				{
					isPresent = true;
				}
			}
			if (!isPresent)
			{
				tmp[vert_in_face] = m_bspEdges[abs(m_bspSurfedges[j])].v[1];
				vert_in_face++;
			}
		}

		m_trisNum += vert_in_face - 2;
		m_vertsNum += vert_in_face;
	}
	
	loadLightmaps( lightedFacesNum );

	m_AABB.setMinBounds(m_bspNodes[0].mins[0], m_bspNodes[0].mins[2], m_bspNodes[0].mins[1]);
	m_AABB.setMaxBounds(m_bspNodes[0].maxs[0], m_bspNodes[0].maxs[2], m_bspNodes[0].maxs[1]);

	buildFacePositions();

	return true;
}

bool gResourceBSPLevel::load() //загрузка видеоданных POOL_DEFAULT
{
	LPDIRECT3DDEVICE9 pD3DDev9 = m_pResMgr->getDevice();

	/////////////////////////////////////
	// create DX buffers
	/////////////////////////////////////	

	//create vertex buffer
	HRESULT hr = pD3DDev9->CreateVertexBuffer(sizeof(gBSPVertex) * m_vertsNum, GBSP_FVF, D3DPOOL_DEFAULT, &m_pVB, 0);
	if (FAILED(hr))
		throw("ќшибка при создании буффера вершин!");

	//create index buffer
	hr = pD3DDev9->CreateIndexBuffer(sizeof(short) *m_trisNum * 3, D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_SYSTEMMEM, &m_pIB, 0);
	if (FAILED(hr))
		throw("ќшибка при создании буффера индексов!");
	
	//create batch index buffer
	m_batchIBSize = sizeof(short) * m_trisNum * 3;
	hr = pD3DDev9->CreateIndexBuffer(m_batchIBSize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pBatchIB, 0);
	if (FAILED(hr))
		throw("ќшибка при создании batch буффера индексов!");

	gBSPVertex* p_vdata = 0;
	gBSPIndex* p_idata = 0;

	hr = m_pVB->Lock(0, sizeof(gBSPVertex) * m_vertsNum, (void**)&p_vdata, 0);
	if (FAILED(hr))
		throw("ќшибка при блокировке вершинного буффера!");

	hr = m_pIB->Lock(0, sizeof(short) * m_trisNum * 3, (void**)&p_idata, 0);
	if (FAILED(hr))
		throw("ќшибка при блокировке индексного буффера!");

	gBSPFaceBounds* pBounds;
	BSPTexinfo_t* pTexinfo;
	BSPVertex_t* pVertex;
	BSPFace_t* pFace;
	BSPMiptex_t* pMiptex;
	int* offs = (int*)(m_bspTexData + sizeof(int));
	float nx, ny, nz;
	float tex_center[2], tex_bounds[2];
	int pos_in_vbuffer = 0;
	int pos_in_ibuffer = 0;
	int first_idx = 0;
	unsigned short tmp[1024];
	m_rFaces = new gBSPRendingFace[m_bspFacesNum];

	for (int i = 0; i < m_bspFacesNum; i++)
	{
		pBounds = &m_faceBounds[i];
		pTexinfo = &m_bspTexinfs[m_bspFaces[i].texinfo];
		pFace = &m_bspFaces[i];

		pMiptex = (BSPMiptex_t*)(m_bspTexData + offs[m_bspTexinfs[m_bspFaces[i].texinfo].miptex]);

		////////////////////////////////////////////////
		//индексируем вершины на фейсе
		////////////////////////////////////////////////

		//собираем неповтор€ющиес€ индексы вершин по гр€н€м
		memset(tmp, 0xFFFF, sizeof(short) * 1024);

		int vert_in_face = 0;

		int last_edge = m_bspFaces[i].firstedge + m_bspFaces[i].numedges;
		for (int j = m_bspFaces[i].firstedge; j < last_edge; j++)
		{
			if (m_bspSurfedges[j] > 0)
			{
				bool isPresent = false;
				for (int k = 0; k < m_bspFaces[i].numedges; k++)
				{
					if (tmp[k] == m_bspEdges[abs(m_bspSurfedges[j])].v[0])
					{
						isPresent = true;
					}
				}
				if (!isPresent)
				{
					tmp[vert_in_face] = m_bspEdges[abs(m_bspSurfedges[j])].v[0];
					vert_in_face++;
				}
				isPresent = false;

				for (int k = 0; k < m_bspFaces[i].numedges; k++)
				{
					if (tmp[k] == m_bspEdges[abs(m_bspSurfedges[j])].v[1])
					{
						isPresent = true;
					}
				}
				if (!isPresent)
				{
					tmp[vert_in_face] = m_bspEdges[abs(m_bspSurfedges[j])].v[1];
					vert_in_face++;
				}
			}
			else
			{
				bool isPresent = false;
				for (int k = 0; k < m_bspFaces[i].numedges; k++)
				{
					if (tmp[k] == m_bspEdges[abs(m_bspSurfedges[j])].v[1])
					{
						isPresent = true;
					}
				}
				if (!isPresent)
				{
					tmp[vert_in_face] = m_bspEdges[abs(m_bspSurfedges[j])].v[1];
					vert_in_face++;
				}
				isPresent = false;

				for (int k = 0; k < m_bspFaces[i].numedges; k++)
				{
					if (tmp[k] == m_bspEdges[abs(m_bspSurfedges[j])].v[0])
					{
						isPresent = true;
					}
				}
				if (!isPresent)
				{
					tmp[vert_in_face] = m_bspEdges[abs(m_bspSurfedges[j])].v[0];
					vert_in_face++;
				}
			}
		}

		/////////////////////////////////////////
		//заполн€ем вершинный буфер
		/////////////////////////////////////////

		if (m_bspFaces[i].side != 0)
		{
			nx = -m_bspPlanes[m_bspFaces[i].planenum].normal[0];
			ny = -m_bspPlanes[m_bspFaces[i].planenum].normal[2];
			nz = -m_bspPlanes[m_bspFaces[i].planenum].normal[1];
		}
		else
		{
			nx = m_bspPlanes[m_bspFaces[i].planenum].normal[0];
			ny = m_bspPlanes[m_bspFaces[i].planenum].normal[2];  ///swap normal z<->y
			nz = m_bspPlanes[m_bspFaces[i].planenum].normal[1];
		}

		int pre_pos_in_vbuffer = pos_in_vbuffer;
		for (int j = 0; j < vert_in_face; j++)
		{
			int g = pos_in_vbuffer;

			//p_vdata[g].color = 0xFFFFFFFF;
			p_vdata[g].x = m_bspVerts[tmp[j]].point[0];
			p_vdata[g].y = m_bspVerts[tmp[j]].point[2]; // swap y <-> z
			p_vdata[g].z = m_bspVerts[tmp[j]].point[1];

			p_vdata[g].nx = nx;
			p_vdata[g].ny = ny;
			p_vdata[g].nz = nz;

			//u = tv00 * x + tv01 * z + tv02 * y + tv03
			//v = tv10 * x + tv11 * z + tv12 * y + tv13


			//NEW:
			p_vdata[g].tu = (
				pTexinfo->vecs[0][0] * m_bspVerts[tmp[j]].point[0] +
				pTexinfo->vecs[0][2] * m_bspVerts[tmp[j]].point[2] +
				pTexinfo->vecs[0][1] * m_bspVerts[tmp[j]].point[1] +
				pTexinfo->vecs[0][3]);
			p_vdata[g].tv = (
				pTexinfo->vecs[1][0] * m_bspVerts[tmp[j]].point[0] +
				pTexinfo->vecs[1][2] * m_bspVerts[tmp[j]].point[2] +
				pTexinfo->vecs[1][1] * m_bspVerts[tmp[j]].point[1] +
				pTexinfo->vecs[1][3]);

			if (pFace->styles[0] == 0)
			{
				tex_bounds[0] = m_faceBounds[i].maxs[0] - m_faceBounds[i].mins[0];
				tex_bounds[1] = m_faceBounds[i].maxs[1] - m_faceBounds[i].mins[1];
				tex_center[0] = tex_bounds[0] * 0.5f + m_faceBounds[i].mins[0];
				tex_center[1] = tex_bounds[1] * 0.5f + m_faceBounds[i].mins[1];

				float scaleU = (float)((float)m_faceBounds[i].texsize[0] - 1.f) 
					/ (float)m_faceBounds[i].texsize[0];
				float scaleV = (float)((float)m_faceBounds[i].texsize[1] - 1.f) 
					/ (float)m_faceBounds[i].texsize[1];

				float dott[2] = { (p_vdata[g].tu * 0.0625f), (p_vdata[g].tv * 0.0625f) }; // /16 = *0.0625

				p_vdata[g].tu2 = ((dott[0] - tex_center[0]) / tex_bounds[0]) * scaleU + 0.5f;
				p_vdata[g].tv2 = ((dott[1] - tex_center[1]) / tex_bounds[1]) * scaleV + 0.5f;

				float pxU = 1.f / m_lMapAtlasW;
				float pxV = 1.f / m_lMapAtlasH;

				//remap for Lightmap atlas
				p_vdata[g].tu2 = (pxU * m_faceBounds[i].remapped[0] + 
					p_vdata[g].tu2 * (float)m_faceBounds[i].texsize[0] / m_lMapAtlasW);
				p_vdata[g].tv2 = (pxV * m_faceBounds[i].remapped[1] + 
					p_vdata[g].tv2 * (float)m_faceBounds[i].texsize[1] / m_lMapAtlasH);

			}
			else
			{
				p_vdata[g].tu2 = 1.0f; //BUGFIX:
				p_vdata[g].tv2 = 1.0f;
			}

			p_vdata[g].tu /= pMiptex->width;
			p_vdata[g].tv /= pMiptex->height;

			pos_in_vbuffer++;
		}


		///////////////////////////////////////////////////////
		//по проиндексированным вершинам строим многоугольники
		///////////////////////////////////////////////////////
		int* offs = (int*)(m_bspTexData + sizeof(int));
		BSPMiptex_t* miptex = (BSPMiptex_t*)(m_bspTexData + offs[m_bspTexinfs[m_bspFaces[i].texinfo].miptex]);

		int tris_in_face = vert_in_face - 2;

		m_rFaces[i].needDraw = false;
		m_rFaces[i].start_indx = pos_in_ibuffer;
		m_rFaces[i].num_prim = tris_in_face;
		m_rFaces[i].miptex = m_bspTexinfs[m_bspFaces[i].texinfo].miptex;

		toUpper(miptex->name);
		gMaterial* pMat = m_pResMgr->getMaterialFactory()->getMaterial(miptex->name);
		if (!pMat)
		{
			pMat = m_pResMgr->getMaterialFactory()->createMaterial(miptex->name);
			m_defaultMatMap[miptex->name] = pMat;

			gResource2DTexture* pTex = (gResource2DTexture*)m_pResMgr->getResource(miptex->name, GRESGROUP_2DTEXTURE);
			if( !pTex )
				pTex = (gResource2DTexture*)m_pResMgr->loadTexture2DFromWADList(miptex->name);

			pTex->addRef();

			pMat->setTexture( 0, pTex );
			if( pFace->styles[0] == 0 )
				pMat->setTexture( 1, m_pLMapTex );
		}
		else
			m_rFaces[i].pMaterial = pMat;

		first_idx = pre_pos_in_vbuffer;
		for (int j = 0; j < tris_in_face; j++)
		{
			int g = pos_in_ibuffer;
			p_idata[g] = first_idx;
			p_idata[g + 1] = j + 1 + first_idx;
			p_idata[g + 2] = j + 2 + first_idx;
			pos_in_ibuffer += 3;

		}
	}


	m_pVB->Unlock();
	m_pIB->Unlock();

	return true;
}

void gResourceBSPLevel::unload() //данные, загруженые preload() в этой функции не измен€ютс€
{
	if (m_pBatchIB)
		m_pBatchIB->Release();
	if (m_pVB)
		m_pVB->Release();
	if (m_pIB)
		m_pIB->Release();

	m_pVB = 0;
	m_pIB = 0;
	m_pBatchIB = 0;
}

void gResourceBSPLevel::onFrameRender(gRenderQueue* queue, const gEntity* entity, const gCamera* cam) const
{
	int currentLeaf = getLeafAtPoint( cam->getPosition() );

	if (currentLeaf > 0)
	{
		drawVisibleLeafs(currentLeaf, *cam);
	}
	else if (currentLeaf == 0)
	{
		for (int i = 0; i < m_visLeafsNum; i++)
		{
			this->drawLeaf(i);
		}
	}

	//запись в очередь
	for( unsigned int i = 0; i < m_bspFacesNum; i++ )
	{
		if (!m_rFaces[i].needDraw)
			continue;

		int* offs = (int*)(m_bspTexData + sizeof(int));
		BSPMiptex_t* miptex = (BSPMiptex_t*)(m_bspTexData + offs[m_bspTexinfs[m_bspFaces[i].texinfo].miptex]);
		gMaterial* pMat = m_pResMgr->getMaterialFactory()->getMaterial(miptex->name);
		pMat->setLightingEnable(false);
		const D3DXMATRIX& matrix = entity->getHoldingNode()->getAbsoluteMatrix();

		queue->pushBack(gRenderElement(this, pMat, 0, 1, 
			&matrix, m_rFaces[i].start_indx, m_rFaces[i].num_prim, m_vertsNum));

		m_rFaces[i].needDraw = false;
	}
}

// returned dist point % plane
inline float gResourceBSPLevel::testPointOnPlane(const D3DXVECTOR3& point, int plane) const
{
	return (point.x * m_bspPlanes[plane].normal[0] +
		point.y * m_bspPlanes[plane].normal[2] +
		point.z * m_bspPlanes[plane].normal[1] - m_bspPlanes[plane].dist);
}

int gResourceBSPLevel::getLeafAtPoint( const D3DXVECTOR3& point ) const
{
	int node = 0;
	do
	{
		if (testPointOnPlane( point, m_bspNodes[node].planenum) >= 0)
			node = m_bspNodes[node].children[0];
		else
			node = m_bspNodes[node].children[1];

		//if (steps != 0) (*steps)++;

	} while (node > 0);

	return  -(node + 1);
}

void gResourceBSPLevel::drawVisibleLeafs( int camLeaf, const gCamera& cam ) const
{
	byte pvs[2048];
	memset(&pvs[0], 0, 2048);

	int decomprSz = 0;
	if (m_bspVisData > 0)
		decomprSz = BSPDecompressVisRow( &m_bspVisData[m_bspLeafs[camLeaf].visofs], &pvs[0], m_visRow );

	
	for (int leaf = 0; leaf < m_visLeafsNum; leaf++)
	{
		if (isLeafVisible(&pvs[0], leaf)) //&& isLeafInFrustum(leaf + 1))
		{
			if( isLeafInFrustum( (leaf + 1), cam ) )
				drawLeaf(leaf + 1);
		}
	}
	

	for (int leaf = m_visLeafsNum; leaf < m_bspLeafsNum ; leaf++)
	{
		drawLeaf(leaf);
	}
}

bool gResourceBSPLevel::isLeafVisible(byte* decomprPVS, int leafBit) const
{
	if (m_bspVisData == 0)
		return true;

	/*
	unsigned short b = leafBit >> 3; // leaf/8;
	unsigned short bitOffset = leafBit - ( b << 3 ); // b*8

	//extract bit
	b = ( decomprPVS[b] >> bitOffset ) & 1;

	if (!useFrustum)
		return b != 0;
	else
		return (b != 0) && isLeafInFrustum(leafBit+1);
	*/

	//bool inFrustum = true;
	//if (useFrustum)
	//	inFrustum = isLeafInFrustum(leafBit + 1);

	return ((decomprPVS[leafBit >> 3] & (1 << (leafBit & 7))));// && inFrustum; //Original Valve Pvs test
}

bool gResourceBSPLevel::isLeafInFrustum(int leaf, const gCamera& cam ) const
{
	D3DXVECTOR3 bmin(m_bspLeafs[leaf].mins[0], m_bspLeafs[leaf].mins[2], m_bspLeafs[leaf].mins[1]);
	D3DXVECTOR3 bmax(m_bspLeafs[leaf].maxs[0], m_bspLeafs[leaf].maxs[2], m_bspLeafs[leaf].maxs[1]);

	return 	cam.getViewingFrustum().testAABB(gAABB(bmin, bmax));
}

void gResourceBSPLevel::drawLeaf(int leaf) const
{
	for (int f = m_bspLeafs[leaf].firstmarksurface;
		f < m_bspLeafs[leaf].firstmarksurface + m_bspLeafs[leaf].nummarksurfaces; f++)
	{
		drawFace( m_bspMarksurfaces[f]);
	}
}

void gResourceBSPLevel::drawFace(int face) const
{
	m_rFaces[face].needDraw = true;
}

void* gResourceBSPLevel::getVBuffer() const
{
	return m_pVB;
}

void* gResourceBSPLevel::getIBuffer() const
{
	return m_pIB;
}

unsigned int gResourceBSPLevel::getIBufferSize() const
{
	return m_trisNum * 3 * sizeof(short); // 16bit indexes
}

unsigned int gResourceBSPLevel::getVBufferSize() const
{
	return m_vertsNum * sizeof(gBSPVertex);
}

void* gResourceBSPLevel::getBatchIBuffer() const
{
	return m_pBatchIB;
}

unsigned int gResourceBSPLevel::getBatchIBufferSize() const
{
	return m_batchIBSize;
}

GPRIMITIVETYPE gResourceBSPLevel::getPrimitiveType() const
{
	return GPRIMITIVETYPE::GPT_TRIANGLELIST;
}

GVERTEXFORMAT gResourceBSPLevel::getVertexFormat() const
{
	return GVF_LEVEL;
}

unsigned int gResourceBSPLevel::getVertexStride() const
{
	return sizeof(gBSPVertex);
}

bool gResourceBSPLevel::isUseUserMemoryPointer()
{
	return false;
}

bool gResourceBSPLevel::loadLightmaps( unsigned int lightedFacesNum )
{
	gTextureAtlas mapTexAtlas;
	mapTexAtlas.beginAtlas(lightedFacesNum);

	for (unsigned int i = 0; i < m_bspFacesNum; i++)
	{
		if (m_bspFaces[i].styles[0] == 0)
		{
			mapTexAtlas.pushTexture(m_faceBounds[i].texsize[0], m_faceBounds[i].texsize[1], &m_faceBounds[i]);

		}
	}

	if (!mapTexAtlas.mergeTexturesToAtlas(4096, 4096))
		return false;

	m_lMapAtlasW = mapTexAtlas.getAtlasWidth();
	m_lMapAtlasH = mapTexAtlas.getAtlasHeight();


	m_pBitmap = new gBMPFile();
	m_pBitmap->createBitMap( m_lMapAtlasW, m_lMapAtlasH );

	//FILE* fd = 0;
	//errno_t err = fopen_s(&fd, "out_bsplevel_order.txt", "wt");

	for ( unsigned int i = 0; i < mapTexAtlas.getTexturesNum(); i++ )
	{
		gBSPFaceBounds* pFaceBounds = (gBSPFaceBounds*)mapTexAtlas.getUserDataBySortedIndex(i);
		if (pFaceBounds)
		{
			iwadcolor* pFaceLightBitmapData = (iwadcolor*)(m_bspLightData + m_bspFaces[pFaceBounds->faceIndex].lightofs);
			gBMPFile faceLightBitmap;

			pFaceBounds->remapped[0] = mapTexAtlas.getTextureRemapedXPosBySortedOrder(i);
			pFaceBounds->remapped[1] = mapTexAtlas.getTextureRemapedYPosBySortedOrder(i);

			faceLightBitmap.loadFromMemory( pFaceLightBitmapData, pFaceBounds->texsize[0], pFaceBounds->texsize[1] );
			m_pBitmap->overlapOther( faceLightBitmap, pFaceBounds->remapped[0], pFaceBounds->remapped[1] );
			

			//fprintf(fd, "atlas ind: %i   face index: %i   lightofs: %i   w:%i h%i\n", i, pFaceBounds->faceIndex, 
			//	m_bspFaces[pFaceBounds->faceIndex].lightofs, pFaceBounds->texsize[0], pFaceBounds->texsize[1]);
		}
	}

	//fclose(fd);

	//gFile* f = m_pResMgr->getFileSystem()->openFile( "new_atlas.bmp", true, true );
	m_pBitmap->swapRGBtoBGR();
	//atlasBMP.saveToFile(f);
	//delete f;

	std::string name = m_resName + ".lmap";

	// m_pLMapTex loaded in MANAGED_POOL
	m_pLMapTex = (gResource2DTexture*)m_pResMgr->loadTextureFromBitmap(m_pBitmap, name.c_str() );
	m_pLMapTex->addRef();

	return true;
}

void gResourceBSPLevel::buildFacePositions()
{
	if (m_facePositions)
		delete [] m_facePositions;

	m_facePositions = new D3DXVECTOR3[m_bspFacesNum];

	gAABB faceBB;
	D3DXVECTOR3 pos;

	for (unsigned int i = 0; i < m_bspFacesNum; i++)
	{
		faceBB.reset();

		for (unsigned int e = m_bspFaces[i].firstedge; e < m_bspFaces[i].numedges +
			m_bspFaces[i].firstedge; e++)
		{
			faceBB.addPoint(D3DXVECTOR3(
				m_bspVerts[m_bspEdges[abs(m_bspSurfedges[e])].v[0]].point[0],
				m_bspVerts[m_bspEdges[abs(m_bspSurfedges[e])].v[0]].point[2],
				m_bspVerts[m_bspEdges[abs(m_bspSurfedges[e])].v[0]].point[1] ) );

			faceBB.addPoint(D3DXVECTOR3(
				m_bspVerts[m_bspEdges[abs(m_bspSurfedges[e])].v[1]].point[0],
				m_bspVerts[m_bspEdges[abs(m_bspSurfedges[e])].v[1]].point[2],
				m_bspVerts[m_bspEdges[abs(m_bspSurfedges[e])].v[1]].point[1]));
		}
		faceBB.getCenterPoint( &m_facePositions[i] );
	}
}

void* gResourceBSPLevel::getLump(unsigned char lump) const
{
	if ( lump >= HEADER_LUMPS )
		return 0;
	return (((byte*)m_pBSPHeader) + m_pBSPHeader->lumps[lump].fileofs);
}
