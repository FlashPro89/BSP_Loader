
# ifdef _BSP_FILE_
#define _BSP_FILE_


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

typedef struct
{
	int		fileofs, filelen; // �������� � ������ ����� � �����
} BSPMapLump_t;


typedef struct
{
	int			version;
	lump_t		lumps[HEADER_LUMPS];
} BSPMapHeader_t;


#endif