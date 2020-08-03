#pragma once

#ifndef _BSP_FILE_H_
#define _BSP_FILE_H_


#define	LUMP_ENTITIES	0
#define	LUMP_PLANES		1
#define	LUMP_TEXTURES	2
#define	LUMP_VERTEXES	3
#define	LUMP_VISIBILITY	4
#define	LUMP_NODES		5
#define	LUMP_TEXINFO	6
#define	LUMP_FACES		7
#define	LUMP_LIGHTING	8
#define	LUMP_CLIPNODES	9
#define	LUMP_LEAFS		10
#define	LUMP_MARKSURFACES 11
#define	LUMP_EDGES		12
#define	LUMP_SURFEDGES	13
#define	LUMP_MODELS		14

#define	HEADER_LUMPS	15
#define BSPVERSION		30
#define	MAX_MAP_HULLS	4
#define	MIPLEVELS		4
#define	NUM_AMBIENTS	4		// automatic ambient sounds

typedef unsigned char byte;

struct BSPMapLump_t
{
	int		fileofs, filelen; // смещение и размер блока в файле
};

struct BSPMapHeader_t
{
	int					version;
	BSPMapLump_t		lumps[HEADER_LUMPS];
};

struct BSPTexinfo_t
{
	float		vecs[2][4];		// [s/t][xyz offset]
	int			miptex;
	int			flags;
};

struct BSPMiptexlump_t
{
	int			nummiptex;
	int			dataofs[4];		// [nummiptex]
};

struct BSPMiptex_t
{
	char		name[16];
	unsigned	width, height;
	unsigned	offsets[MIPLEVELS];		// four mip maps stored
};

struct BSPVertex_t
{
	float	point[3];
};

struct BSPVec2D_t
{
	float	x;
	float	y;
};

struct BSPEdge_t
{
	unsigned short	v[2];		// vertex numbers
};

#define	MAXLIGHTMAPS	4

struct BSPFace_t
{
	short		planenum;
	short		side;

	int			firstedge;		// we must support > 64k edges
	short		numedges;
	short		texinfo;

	// lighting info
	byte		styles[MAXLIGHTMAPS];
	int			lightofs;		// start of [numstyles*surfsize] samples
};

struct BSPNode_t
{
	int			planenum;
	short		children[2];	// negative numbers are -(leafs+1), not nodes
	short		mins[3];		// for sphere culling
	short		maxs[3];
	unsigned short	firstface;
	unsigned short	numfaces;	// counting both sides
};

struct BSPModel_t
{
	float		mins[3], maxs[3];
	float		origin[3];
	int			headnode[MAX_MAP_HULLS];
	int			visleafs;		// not including the solid leaf 0
	int			firstface, numfaces;
};

struct  BSPLeaf_t
{
	int			contents;
	int			visofs;				// -1 = no visibility info

	short		mins[3];			// for frustum culling
	short		maxs[3];

	unsigned short		firstmarksurface;
	unsigned short		nummarksurfaces;

	byte		ambient_level[NUM_AMBIENTS];
};

struct  BSPClipnode_t
{
	int			planenum;
	short		children[2];	// negative numbers are contents
};

// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2

// 3-5 are non-axial planes snapped to the nearest
#define	PLANE_ANYX		3
#define	PLANE_ANYY		4
#define	PLANE_ANYZ		5

struct BSPPlane_t
{
	float	normal[3];
	float	dist;
	int		type;		// PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
};

int BSPCopyLump(BSPMapHeader_t* header, unsigned int lump, void* dest, unsigned int size);
int BSPGetLumpItemsNum(BSPMapHeader_t* header, unsigned int lump);

int BSPCompressVisRow( const byte* vis, byte* dest, int visrow);
int BSPDecompressVisRow(const byte* visCompr, byte* dest, int visrow);

#endif