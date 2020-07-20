#include "BSPLevel.h"
#include "FileSystem.h"
#include "BSPFile.h"

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
}

gResourceBSPLevel::~gResourceBSPLevel()
{
	freeMem();
}

bool gResourceBSPLevel::preload() //загрузка статических данных
{
	
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

	for (unsigned int i = 0; i <m_bspFacesNum; i++)
	{
		if (m_bspFaces[i].styles[0] == 0)
			lightedFacesNum++;

		//собираем неповторяющиеся индексы вершин по гряням
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
	
	buildLightmapAtlas();

	return true;
}

bool gResourceBSPLevel::load() //загрузка видеоданных POOL_DEFAULT
{
	return true;
}

void gResourceBSPLevel::unload() //данные, загруженые preload() в этой функции не изменяются
{
	
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
	return 0;
}

bool gResourceBSPLevel::isUseUserMemoryPointer()
{
	return false;
}

bool gResourceBSPLevel::loadLightmaps()
{
	return true;
}

bool gResourceBSPLevel::buildLightmapAtlas( unsigned int lightedFacesNum )
{
	m_lMapTexAtlas.beginAtlas(lightedFacesNum);

	return true;
}


void gResourceBSPLevel::freeMem()
{
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
