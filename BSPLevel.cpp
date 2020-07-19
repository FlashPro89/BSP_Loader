#include "BSPLevel.h"

gResourceBSPLevel::gResourceBSPLevel(gResourceManager* mgr, GRESOURCEGROUP group,
	const char* filename, const char* name) : gRenderable(mgr, group, filename, name)
{

	m_vertsNum = 0;
	m_edgesNum = 0;
	m_surfedgesNum = 0;
	m_facesNum = 0;
	m_texinfsNum = 0;
	m_planesNum = 0;
	m_miptexsNum = 0;
	m_modelsNum = 0;
	m_nodesNum = 0;
	m_leafsNum = 0;
	m_marksurfacesNum = 0;
	m_clipnodesNum = 0;

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
}

gResourceBSPLevel::~gResourceBSPLevel()
{

}

bool gResourceBSPLevel::preload() //загрузка статических данных
{
	/*
	FILE* f = 0;

	errno_t err = fopen_s(&f, m_fileName.c_str(), "rb");
	if (!f || err) throw("BSP File Opening Error");

	int fl = filelength(f);
	BSPMapHeader_t* bsp_header = (BSPMapHeader_t*)(new byte[fl + 1]); //? +1?
	((byte*)bsp_header)[fl] = 0;
	//memset( bsp_header, 0, fl );

	if (fread_s(bsp_header, fl, 1, fl, f) != fl)
		throw("Cannot read BSP file!");

	fclose(f);
	f = 0;

	if (bsp_header->version != BSPVERSION) throw("Invalid BSP File Version");

	//swap bytes ?!?!? or no?

	//load verts
	num_verts = BSPGetLumpItemsNum(bsp_header, LUMP_VERTEXES);
	bsp_verts = new BSPVertex_t[num_verts];
	BSPCopyLump(bsp_header, LUMP_VERTEXES, bsp_verts, sizeof(BSPVertex_t));

	//load eges
	num_edges = BSPGetLumpItemsNum(bsp_header, LUMP_EDGES);
	bsp_edges = new BSPEdge_t[num_edges];
	BSPCopyLump(bsp_header, LUMP_EDGES, bsp_edges, sizeof(BSPEdge_t));

	//load surfedges
	num_surfedges = BSPGetLumpItemsNum(bsp_header, LUMP_SURFEDGES);
	bsp_surfedges = new int[num_surfedges];
	BSPCopyLump(bsp_header, LUMP_SURFEDGES, bsp_surfedges, sizeof(int));

	//load faces
	num_faces = BSPGetLumpItemsNum(bsp_header, LUMP_FACES);
	bsp_faces = new BSPFace_t[num_faces];
	BSPCopyLump(bsp_header, LUMP_FACES, bsp_faces, sizeof(BSPFace_t));

	//load texinfos
	num_texinfs = BSPGetLumpItemsNum(bsp_header, LUMP_TEXINFO);
	bsp_texinfs = new BSPTexinfo_t[num_texinfs];
	BSPCopyLump(bsp_header, LUMP_TEXINFO, bsp_texinfs, sizeof(BSPTexinfo_t));

	//load planes
	num_planes = BSPGetLumpItemsNum(bsp_header, LUMP_PLANES);
	bsp_planes = new BSPPlane_t[num_planes];
	BSPCopyLump(bsp_header, LUMP_PLANES, bsp_planes, sizeof(BSPPlane_t));

	///load models
	num_models = BSPGetLumpItemsNum(bsp_header, LUMP_MODELS);
	bsp_models = new BSPModel_t[num_models];
	BSPCopyLump(bsp_header, LUMP_MODELS, bsp_models, sizeof(BSPModel_t));

	//load leafs
	num_leafs = BSPGetLumpItemsNum(bsp_header, LUMP_LEAFS);
	bsp_leafs = new BSPLeaf_t[num_leafs];
	BSPCopyLump(bsp_header, LUMP_LEAFS, bsp_leafs, sizeof(BSPLeaf_t));

	//load nodes
	num_nodes = BSPGetLumpItemsNum(bsp_header, LUMP_NODES);
	bsp_nodes = new BSPNode_t[num_nodes];
	BSPCopyLump(bsp_header, LUMP_NODES, bsp_nodes, sizeof(BSPNode_t));

	//load marksurfaces
	num_marksurfaces = BSPGetLumpItemsNum(bsp_header, LUMP_MARKSURFACES);
	bsp_marksurfaces = new unsigned short[num_marksurfaces];
	BSPCopyLump(bsp_header, LUMP_MARKSURFACES, bsp_marksurfaces, sizeof(unsigned short));

	//load clipnodes
	num_clipnodes = BSPGetLumpItemsNum(bsp_header, LUMP_CLIPNODES);
	bsp_clipnodes = new BSPClipnode_t[num_clipnodes];
	BSPCopyLump(bsp_header, LUMP_CLIPNODES, bsp_clipnodes, sizeof(BSPClipnode_t));

	//load miptexs
	bsp_texdatasize = bsp_header->lumps[LUMP_TEXTURES].filelen;
	bsp_texdata = new byte[bsp_texdatasize];
	BSPCopyLump(bsp_header, LUMP_TEXTURES, bsp_texdata, bsp_header->lumps[LUMP_TEXTURES].filelen);

	//load visdata
	bsp_visdatasize = bsp_header->lumps[LUMP_VISIBILITY].filelen;
	if (bsp_visdatasize > 0)
	{
		bsp_visdata = new byte[bsp_visdatasize];
		BSPCopyLump(bsp_header, LUMP_VISIBILITY, bsp_visdata, bsp_visdatasize);
	}

	//load lightdata
	bsp_lightdatasize = bsp_header->lumps[LUMP_LIGHTING].filelen;
	bsp_lightdata = new byte[bsp_lightdatasize];
	BSPCopyLump(bsp_header, LUMP_LIGHTING, bsp_lightdata, bsp_lightdatasize);

	num_miptexs = bsp_texdatasize ? ((BSPMiptexlump_t*)bsp_texdata)->nummiptex : 0;

	//count leafs with PVS 
	visLeafs = 0;
	for (int i = 0; i < num_leafs; i++)
	{
		if (bsp_leafs[i].visofs >= 0)
			visLeafs++;
	}
	visrow = (visLeafs + 7) >> 3;

	//count triangles for vertex buffer
	int tri_num = 0;
	int vert_num = 0;
	unsigned short tmp[1024];

	for (int i = 0; i < num_faces; i++)
	{
		//собираем неповторяющиеся индексы вершин по гряням
		memset(tmp, 0xFFFF, sizeof(short) * 1024);

		int vert_in_face = 0;

		int last_edge = bsp_faces[i].firstedge + bsp_faces[i].numedges;
		for (int j = bsp_faces[i].firstedge; j < last_edge; j++)
		{
			bool isPresent = false;
			for (int k = 0; k < bsp_faces[i].numedges; k++)
			{
				if (tmp[k] == bsp_edges[abs(bsp_surfedges[j])].v[0])
				{
					isPresent = true;
				}
			}
			if (!isPresent)
			{
				tmp[vert_in_face] = bsp_edges[abs(bsp_surfedges[j])].v[0];
				vert_in_face++;
			}
			isPresent = false;

			for (int k = 0; k < bsp_faces[i].numedges; k++)
			{
				if (tmp[k] == bsp_edges[abs(bsp_surfedges[j])].v[1])
				{
					isPresent = true;
				}
			}
			if (!isPresent)
			{
				tmp[vert_in_face] = bsp_edges[abs(bsp_surfedges[j])].v[1];
				vert_in_face++;
			}
		}

		tri_num += vert_in_face - 2;
		vert_num += vert_in_face;
	}
	*/

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

unsigned int gResourceBSPLevel::getVertexStride() const
{
	return 0;
}

bool gResourceBSPLevel::isUseUserMemoryPointer()
{
	return false;
}

