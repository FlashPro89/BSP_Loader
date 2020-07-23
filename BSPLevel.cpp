#include "BSPLevel.h"
#include "FileSystem.h"
#include "BSPFile.h"
#include "BMPFile.h"

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
	m_pVB = 0;
	m_pIB = 0;
	m_faceBounds = 0;
}

gResourceBSPLevel::~gResourceBSPLevel()
{
	freeMem();
}

bool gResourceBSPLevel::preload() //загрузка статических данных
{
	freeMem();

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
		freeMem();
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
	hr = pD3DDev9->CreateIndexBuffer(sizeof(short) *m_trisNum * 3, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, 0);
	if (FAILED(hr))
		throw("ќшибка при создании буффера индексов!");

	gBSPVertex* p_vdata = 0;
	gBSPIndex* p_idata = 0;

	hr = m_pVB->Lock(0, sizeof(gBSPVertex) * m_vertsNum, (void**)&p_vdata, 0);
	if (FAILED(hr))
		throw("ќшибка при блокировке вершинного буффера!");

	hr = m_pIB->Lock(0, sizeof(short) * m_trisNum * 3, (void**)&p_idata, 0);
	if (FAILED(hr))
		throw("ќшибка при блокировке индексного буффера!");

	// DO SOMETHING

	m_pVB->Unlock();
	m_pIB->Unlock();

	return true;
}

void gResourceBSPLevel::unload() //данные, загруженые preload() в этой функции не измен€ютс€
{
	if (m_pVB)
		m_pVB->Release();
	if (m_pIB)
		m_pIB->Release();

	m_pVB = 0;
	m_pIB = 0;
}

void gResourceBSPLevel::onFrameRender( gRenderQueue* queue, const gEntity* entity, const gCamera* cam ) const
{

}

void* gResourceBSPLevel::getVBuffer() const
{
	return 0;
}

void* gResourceBSPLevel::getIBuffer() const
{
	return 0;
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

	gBMPFile atlasBMP;
	atlasBMP.createBitMap( mapTexAtlas.getAtlasWidth(), mapTexAtlas.getAtlasHeight() );

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
			atlasBMP.overlapOther( faceLightBitmap, pFaceBounds->remapped[0], pFaceBounds->remapped[1] );
			

			//fprintf(fd, "atlas ind: %i   face index: %i   lightofs: %i   w:%i h%i\n", i, pFaceBounds->faceIndex, 
			//	m_bspFaces[pFaceBounds->faceIndex].lightofs, pFaceBounds->texsize[0], pFaceBounds->texsize[1]);
		}
	}

	//fclose(fd);

	gFile* f = m_pResMgr->getFileSystem()->openFile( "new_atlas.bmp", true, true );
	atlasBMP.swapRGBtoBGR();
	atlasBMP.saveToFile(f);
	delete f;

	return true;
}


void gResourceBSPLevel::freeMem()
{
	if (m_pVB)
		m_pVB->Release();
	if (m_pIB)
		m_pIB->Release();

	m_pVB = 0; 
	m_pIB = 0;

	if (m_faceBounds)
		delete[] m_faceBounds;
	m_faceBounds = 0;

	if (m_pBSPHeader)
		delete[] ( (byte*)m_pBSPHeader );
	m_pBSPHeader = 0;
}

void* gResourceBSPLevel::getLump(unsigned char lump) const
{
	if ( lump >= HEADER_LUMPS )
		return 0;
	return (((byte*)m_pBSPHeader) + m_pBSPHeader->lumps[lump].fileofs);
}
