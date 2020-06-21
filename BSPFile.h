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

#define BSPVERSION	30

#define	MAX_MAP_HULLS		4

typedef unsigned char byte;

typedef struct
{
	int		fileofs, filelen; // смещение и размер блока в файле
} BSPMapLump_t;


typedef struct
{
	int					version;
	BSPMapLump_t		lumps[HEADER_LUMPS];
} BSPMapHeader_t;

typedef struct texinfo_s
{
	float		vecs[2][4];		// [s/t][xyz offset]
	int			miptex;
	int			flags;
} BSPTexinfo_t;

typedef struct
{
	int			nummiptex;
	int			dataofs[4];		// [nummiptex]
} BSPMiptexlump_t;

#define	MIPLEVELS	4

typedef struct miptex_s
{
	char		name[16];
	unsigned	width, height;
	unsigned	offsets[MIPLEVELS];		// four mip maps stored
} BSPMiptex_t;

typedef struct
{
	float	point[3];
} BSPVertex_t;

typedef struct
{
	float	x;
	float	y;
} BSPVec2D_t;

typedef struct
{
	unsigned short	v[2];		// vertex numbers
} BSPEdge_t;

#define	MAXLIGHTMAPS	4

typedef struct
{
	short		planenum;
	short		side;

	int			firstedge;		// we must support > 64k edges
	short		numedges;
	short		texinfo;

	// lighting info
	byte		styles[MAXLIGHTMAPS];
	int			lightofs;		// start of [numstyles*surfsize] samples
} BSPFace_t;

typedef struct
{
	int			planenum;
	short		children[2];	// negative numbers are -(leafs+1), not nodes
	short		mins[3];		// for sphere culling
	short		maxs[3];
	unsigned short	firstface;
	unsigned short	numfaces;	// counting both sides
} BSPNode_t;

typedef struct
{
	float		mins[3], maxs[3];
	float		origin[3];
	int			headnode[MAX_MAP_HULLS];
	int			visleafs;		// not including the solid leaf 0
	int			firstface, numfaces;
} BSPModel_t;

#define	NUM_AMBIENTS			4		// automatic ambient sounds
typedef struct
{
	int			contents;
	int			visofs;				// -1 = no visibility info

	short		mins[3];			// for frustum culling
	short		maxs[3];

	unsigned short		firstmarksurface;
	unsigned short		nummarksurfaces;

	byte		ambient_level[NUM_AMBIENTS];
} BSPLeaf_t;

typedef struct
{
	int			planenum;
	short		children[2];	// negative numbers are contents
} BSPClipnode_t;

// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2

// 3-5 are non-axial planes snapped to the nearest
#define	PLANE_ANYX		3
#define	PLANE_ANYY		4
#define	PLANE_ANYZ		5

typedef struct
{
	float	normal[3];
	float	dist;
	int		type;		// PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
} BSPPlane_t;

int BSPCopyLump(BSPMapHeader_t* header, unsigned int lump, void* dest, unsigned int size);
int BSPGetLumpItemsNum(BSPMapHeader_t* header, unsigned int lump);

#endif