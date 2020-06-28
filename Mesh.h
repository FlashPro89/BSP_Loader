#pragma once

#ifndef _MESH_H_
#define _MESH_H_

#include <d3d9.h>
#include <d3dx9.h>
#include <map>
#include <list>
#include <vector>
#include <string>
#include "Resources.h"
#include "BMPFile.h"

#define SKINNING_FPS_DEFAULT 20.f

//for FFP HAL Indexed Vertex Blending
typedef std::map< unsigned char, unsigned char > gSkinRemapedIndexes;
struct gSkinIndexRemapedSubset
{
	gSkinIndexRemapedSubset(unsigned int offset) { indexBufferOffset = offset; primitivesNum = 0;}

	gSkinRemapedIndexes remapedIndexes;
	unsigned int indexBufferOffset;
	unsigned int primitivesNum;
};
typedef std::vector< gSkinIndexRemapedSubset > gSkinIndexRemapedSubsets;

struct gTrisGroup //дл€ отрисовки
{
	unsigned int trisNum;
	unsigned int textureIndex;
	unsigned int trisOffsetInBuff;
	gResource2DTexture* pTex;
	gSkinIndexRemapedSubsets remapedSubsets;

	gBMPFile* bitmap;

	unsigned int __used_tris; // !!! используетс€ только при загрузке данных
	unsigned int __before; // !!! offset количесво байт в буффере _перед_ данной группой треугольников 
};

///				  texture    | tris group
typedef std::map< std::string, gTrisGroup > gTrisGroupCacher;
typedef gTrisGroupCacher::iterator gTrisGroupCacherIterator;

class gSkinBone
{
public:
	gSkinBone();
	~gSkinBone();

	void setParams( int parent, const char* name, const D3DXVECTOR3& position, const D3DXVECTOR3& orientation );

	void setParentId( int parent );
	void setName( const char* name );
	void setPosition( const D3DXVECTOR3& position );
	void setOrientation( const D3DXQUATERNION& orientation );

	void move( const D3DXVECTOR3& v );
	void rotate( const D3DXQUATERNION& q );

	int getParentId() const;
	const char* getName() const;
	const D3DXVECTOR3& getPosition() const;
	const D3DXQUATERNION& getOrientation() const;

protected:
	D3DXVECTOR3 m_position;
	D3DXQUATERNION m_orientation;

	int m_parent;
	std::string m_name;
};

class gResourceSkinAnimation : public gResource
{
public:  
	gResourceSkinAnimation(gResourceManager* mgr, GRESOURCEGROUP group,
		const char* name, const char* filename, gResourceSkinnedMesh* mesh);
	~gResourceSkinAnimation();

	bool preload();//загрузка статических данных
	bool load();
	void unload(); //данные, загруженые preload() в этой функции не измен€ютс€

	void setFPS( float fps );

	float getFPS() const;
	unsigned char getBonesNum() const;
	unsigned char getFramesNum() const;

	gSkinBone* getFrame( unsigned char frame );
	gSkinBone* getBone( unsigned char frame, unsigned char bone );

	//SLERP
	gSkinBone* getFrameInTimePos( float time, gSkinBone* frame ); //frame - in/out

	//D3DXMATRIX* getAbsoluteMatrixes( float time );
	gSkinBone* getNullFrame() const;
	//gSkinBone* getHierarchyTransformedCurrentFrame() const; 
	gResourceSkinnedMesh* getSkinnedMesh() const;

protected:
	gResourceSkinAnimation();
	gResourceSkinAnimation(gResourceSkinAnimation&);
	void transformToWorldCurrentFrame( gSkinBone* frame, int bone );

	gResourceSkinnedMesh* m_mesh;
	gSkinBone* m_frames;      // num bones * num frames
	unsigned char m_framesNum;
	unsigned char m_bonesNum;
	float m_fps;
	unsigned int m_skeletonBlockPos;
	D3DXMATRIX* m_absMats;
	gSkinBone* m_nullFrame;
	//gSkinBone* m_hierarchyTransformedCurrentFrame;
};

typedef std::map< std::string, gResourceSkinAnimation* > gSkinAnimMap;
typedef gSkinAnimMap::iterator gSkinAnimMapIterator;

struct gSkinnedSubset
{
public:
	int m_texture;
	int m_trisnum;
	int m_offset;
};

class gResourceSkinnedMesh : public gRenderable
{
public:
	gResourceSkinnedMesh(gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name);
	~gResourceSkinnedMesh();

	bool preload(); //загрузка статических данных
	bool load(); //загрузка видеоданных POOL_DEFAULT
	void unload(); //данные, загруженые preload() в этой функции не измен€ютс€

	void onFrameRender(const D3DXMATRIX& transform) const;

	bool addAnimation( const char* filename, const char* name );
	gResourceSkinAnimation* getAnimation( const char* name ) const; 

	D3DXMATRIX* getInvertedMatrixes() const;
	gSkinBone* getNullFrame() const;

	unsigned int getBonesNum() const;

	//for debug anim
	float _time;

protected:
	void _skeleton( const gSkinBone* frame, int b1 ) const;
	void _transform_to_world( gSkinBone* bones, int bone );

	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DINDEXBUFFER9 m_pIB;

	unsigned int m_vertexesNum;
	unsigned int m_indexesNum;
	unsigned int m_bonesNum;
	unsigned int m_materialsNum;
	unsigned int m_trisNum;

	int m_nodes_blockpos;
	int m_time0_blockpos;
	int m_tris_blockpos;

	gTrisGroupCacher m_trisCacher;
	gSkinBone* m_pBones;
	D3DXMATRIX* m_pMatInverted;

	gSkinAnimMap m_animMap;

	gSkinBone* m_pTransformedBones; // for debug ?
	//gSkinBone* m_pTransformedBonesByQuat; // for debug ?
};

//debug
struct gDebugNormal;

class gResourceStaticMesh : public gRenderable
{
public:
	gResourceStaticMesh( gResourceManager* mgr, GRESOURCEGROUP group, const char* filename, const char* name );
	~gResourceStaticMesh();

	bool preload(); //загрузка статических данных
	bool load(); //загрузка видеоданных POOL_DEFAULT
	void unload(); //данные, загруженые preload() в этой функции не измен€ютс€

	void onFrameRender(const D3DXMATRIX& transform) const;

protected:

	//debug
	gDebugNormal* m_normals;
	void drawNormals() const;

	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DINDEXBUFFER9 m_pIB;

	unsigned int m_vertexesNum;
	unsigned int m_indexesNum;
	unsigned int m_materialsNum;
	unsigned int m_trisNum;

	int m_tris_blockpos;
	gTrisGroupCacher m_trisCacher;
};

#endif

