#pragma once

#ifndef _SCENE_H_
#define _SCENE_H_

#include <d3dx9.h>
#include <map>
#include <vector>
#include <string>
#include "Resources.h"
#include "Camera.h"
#include "Animator.h"

#define NAME_LENGHT 32

class gVertexAABB
{
public:
	gVertexAABB(const D3DXVECTOR3& v, DWORD c) { pos = v; color = c; }
	D3DXVECTOR3 pos;
	DWORD color;
};

class gSceneManager;
class gSceneNode;
class gSkinnedMeshAnimator;

class gEntity
{
public:
	gEntity( const char* name );
	~gEntity();

	void onAttachToNode( gSceneNode* node );
	void onDetachFromNode( );

	gSceneNode* getHoldingNode() const;
	const char* getName() const;

	void setRenderable( gRenderable* resource );
	const gRenderable* getRenderable() const;

	void onFrameMove( float delta );
	void onFrameRender() const; 

	const gAABB& getAABB();
	gAnimator* getAnimator( GANIMATOR_TYPE type ) const;

protected:
	gEntity() { }
	gEntity( gEntity& ) { }

	void applyWorldMatrixesToRenderSystem( const D3DXMATRIX& transform, LPDIRECT3DDEVICE9 pDevice ) const;
	void deleteAnimators();

	std::string m_name;
	gSceneNode* m_holdingNode;
	mutable gRenderable* m_renderable;
	gAnimator* m_animators[GANIMATOR_NUM];
	gAABB m_AABB;

};

class gSceneNode
{
public:
	gSceneNode( const char* name,  gSceneManager* mgr, gSceneNode* parent );
	~gSceneNode();

	const char* getName() const;
	const D3DXVECTOR3&		getScale() const;
	const D3DXVECTOR3&		getPosition() const;
	const D3DXQUATERNION&	getOrientation() const;
	const D3DXMATRIX&		getAbsoluteMatrix() const;
	const D3DXMATRIX&		getRelativeMatrix() const;
	const gAABB&			getAABB() const;

	gSceneNode* createChild(const char* name);
	bool destroyChild( const char* name);
	void destroyChildren();

	void attachEntity( gEntity* entity );
	void detachEntity( gEntity* entity );
	void detachAllEntities();

	void setScale( const D3DXVECTOR3 scale );
	void setPosition( const D3DXVECTOR3& position );
	void setOrientation( const D3DXQUATERNION& orientation );

	void needTransformParentAABB(); //set flag only
	void needTransformChildren();  //set flag only
	void computeTransform();

	void onFrameMove(float delta);
	void onFrameRender();

protected:

	void nodeAABBChanged(); 
	void drawEntityList();
	void updateEntityList( float delta );
	void drawAABB();

	gSceneNode() {}

	bool			m_isAABBVisible;
	bool			m_isTransformed;
	bool			m_isAABBChanged;
	D3DXMATRIX		m_absoluteMatrix;
	D3DXMATRIX		m_relativeMatrix;
	D3DXVECTOR3		m_scale;
	D3DXVECTOR3		m_position;
	D3DXQUATERNION	m_orientation;
	char			m_name[NAME_LENGHT];
	gAABB			m_AABB;

	gSceneNode* m_parent;
	std::map<std::string, gSceneNode*> m_children;
	std::map < std::string, gEntity* > m_entList;
	gSceneManager* m_sceneManager;
};

class gSceneManager
{
public:
	gSceneManager( gResourceManager* rmgr );
	~gSceneManager();

	const gResourceManager* getResourseManager() const;
	gSceneNode& getRootNode() const;
	gSceneNode* getNode( const char* name ) const;
	gEntity* getEntity( const char* name ) const;

	gSceneNode* createNode( const char* name, gSceneNode* parent );
	gEntity* createEntity( const char* name );

	bool destroyNode( gSceneNode *node );
	bool destroyNode( const char* name );
	bool destroyEntity( const char* name );
	void destroyAllEntities();

	void frameRender();
	void frameMove(float delta);

	void setActiveCamera( gCamera* cam );
	const gCamera* getActiveCamera( ) const;

	//stats
	void			__statEntDraw();
	void			__statNodeDraw();
	unsigned int	__statEntitiesInFrustum() const; // num drawed entities in activeCam
	unsigned int	__statNodesInFrustum() const; // num drawed entities in activeCam

protected:
	gCamera* m_activeCam;
	gResourceManager* m_rmgr;
	mutable gSceneNode m_rootNode;
	std::map < std::string, gSceneNode* > m_nodeList;
	std::map < std::string, gEntity* > m_entList;

	//stats
	unsigned int m_entsInFrustum;
	unsigned int m_nodesInFrustum;
};


#endif
