#include "BSPFile.h"
#include <vcruntime_string.h>


int BSPCopyLump( BSPMapHeader_t *header, unsigned int lump, void *dest, unsigned int elemSize )
{
	int		length, ofs;

	length = header->lumps[lump].filelen;
	ofs = header->lumps[lump].fileofs;
	
	if (length % elemSize)
		throw( "LoadBSPFile: odd lump size" );
	
	byte* buf = (byte * )header;
	
	memmove( dest, buf + ofs, length);

	return length / elemSize;
}

int BSPGetLumpItemsNum(BSPMapHeader_t* header, unsigned int lump)
{
	switch (lump)
	{
	case LUMP_LEAFS:
		return header->lumps[LUMP_LEAFS].filelen / sizeof(BSPLeaf_t);
	case LUMP_NODES:
		return header->lumps[LUMP_NODES].filelen / sizeof(BSPNode_t);
	case LUMP_MARKSURFACES:
		return header->lumps[LUMP_MARKSURFACES].filelen / sizeof(unsigned short);
	case LUMP_VERTEXES:
		return header->lumps[LUMP_VERTEXES].filelen / sizeof(BSPVertex_t);
	case LUMP_EDGES:
		return header->lumps[LUMP_EDGES].filelen / sizeof(BSPEdge_t);
	case LUMP_FACES:
		return header->lumps[LUMP_FACES].filelen / sizeof(BSPFace_t);
	case LUMP_TEXINFO:
		return header->lumps[LUMP_TEXINFO].filelen / sizeof(BSPTexinfo_t);
	case LUMP_PLANES:
		return header->lumps[LUMP_PLANES].filelen / sizeof(BSPPlane_t);
	case LUMP_MODELS:
		return header->lumps[LUMP_MODELS].filelen / sizeof(BSPModel_t);
	case LUMP_SURFEDGES:
		return header->lumps[LUMP_SURFEDGES].filelen / sizeof(int);
	case LUMP_LIGHTING:
		return header->lumps[LUMP_LIGHTING].filelen / sizeof(byte);
	case LUMP_CLIPNODES:
		return header->lumps[LUMP_CLIPNODES].filelen / sizeof(BSPClipnode_t);
	}
	return 0;
}

int BSPCompressVisRow(byte* vis, byte* dest, int visrow)
{
	int		j;
	int		rep;
	//int		visrow;
	byte* dest_p;

	dest_p = dest;
	//visrow = (num_leafs + 7) >> 3;


	for (j = 0; j < visrow; j++)
	{
		*dest_p++ = vis[j];
		if (vis[j])
			continue;

		rep = 1;
		for (j++; j < visrow; j++)
			if (vis[j] || rep == 255)
				break;
			else
				rep++;
		*dest_p++ = rep;
		j--;
	}

	return dest_p - dest;
}

int BSPDecompressVisRow(byte* visCompr, byte* dest, int visrow)
{
	//TEST:
	//int bitbytes = ((visLeafs + 63) & ~63) >> 3;

	//int visrow = (visLeafs + 7) >> 3;
	int counter = 0;
	byte* p = &dest[0];
	byte* v = &visCompr[0];

	int compReaded = 0;

	while (counter < visrow)
		//while (counter < bitbytes)
	{
		if ((*v) != 0)
		{
			(*p) = (*v);
			p++; v++; compReaded++;
			counter++;
		}
		else
		{
			v++;
			memset(p, 0, *v); //fill in "dest" zeros
			p += *v;
			counter += *v;
			v++;
			compReaded += 2;
		}
	}
	return v - visCompr;
}