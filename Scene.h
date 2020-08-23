#pragma once

#ifndef _SCENE_H_
#define _SCENE_H_

#include <d3dx9.h>
#include <map>
#include <vector>
#include <string>
#include "RenderQueue.h"
#include "Resources.h"
#include "Materials.h"
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
	void onDetachFromNode();
	void onHoldingNodeTransformed();

	gSceneNode* getHoldingNode() const;
	const D3DXMATRIX& getAbsoluteMatrix() const; // use entity offset transform if needed
	const char* getName() const;

	void setRenderable( gRenderable* resource );
	const gRenderable* getRenderable() const;

	void onFrameMove( float delta );
	void onFrameRender( gRenderQueue& queue, const gCamera* camera ) const;

	const gAABB& getAABB();
	void drawAABB( IDirect3DDevice9* pDev );

	gAnimator* getAnimator( GANIMATOR_TYPE type ) const;
	
	//index -1 set All subMesh Materials to this
	bool setMaterial( gMaterial* material, short matIndex = -1 );
	bool setMaterialByName( gMaterial* material, const char* name );
	gMaterial* getMaterial( short matIndex ) const;
	gMaterial* getMaterialByName( const char* name ) const;

	void setRenderableOffsetPosition( const D3DXVECTOR3& offsetPosition );
	void setRenderableOffsetScale( const D3DXVECTOR3& offsetScale );
	void setRenderableOffsetOrientaion( const D3DXQUATERNION& offsetOrientation );

	const D3DXVECTOR3& getRenderableOffsetPosition( ) const;
	const D3DXVECTOR3& getRenderableOffsetScale( ) const;
	const D3DXQUATERNION& getRenderableOffsetOrientaion( ) const;

	bool isNeedOffsetTransform() const;

protected:
	gEntity() { }
	gEntity( gEntity& ) { }

	void _deleteAnimators();
	bool _checkIsNeedOffsetTransform();
	void _rebuildOffsetTransformMatrix();
	void _rebuildAbsoluteTransformMatrix();

	std::map< std::string, gMaterial* > m_userMaterials;

	std::string m_name;
	gSceneNode* m_pHoldingNode;
	mutable gRenderable* m_pRenderable;
	gAnimator* m_animators[GANIMATOR_NUM];
	gAABB m_AABB;
	bool m_isAABBVisible;


	D3DXVECTOR3 m_offsetScale;
	D3DXVECTOR3 m_offsetPosition;
	D3DXQUATERNION m_offsetOrientation;
	bool m_isNeedOffsetTransform;

	D3DXMATRIX m_asboluteTransformMatrix;
	D3DXMATRIX m_offsetMatrix;
};

class gSceneNode
{
public:
	gSceneNode( const char* name,  gSceneManager* mgr, gSceneNode* parent );
	~gSceneNode();

	const char* getName() const;
	const D3DXVECTOR3&		getRelativeScale() const;
	const D3DXVECTOR3&		getRelativePosition() const;
	const D3DXQUATERNION&	getRelativeOrientation() const;

	const D3DXVECTOR3&		getAbsoluteScale() const;
	const D3DXVECTOR3&		getAbsolutePosition() const;
	const D3DXQUATERNION&	getAbsoluteOrientation() const;

	const D3DXMATRIX&		getAbsoluteMatrix() const;
	const D3DXMATRIX&		getRelativeMatrix() const;
	const gAABB&			getAABB() const;

	gSceneNode* createChild(const char* name);
	bool destroyChild( const char* name);
	void destroyChildren();

	void attachEntity( gEntity* entity );
	void detachEntity( gEntity* entity );
	void detachAllEntities();

	bool onDestroyChild( const char* name );

	void setRelativeScale( const D3DXVECTOR3 scale );
	void setRelativePosition( const D3DXVECTOR3& position );
	void setRelativeOrientation( const D3DXQUATERNION& orientation );

	void setAbsoluteScale(const D3DXVECTOR3 scale);
	void setAbsolutePosition(const D3DXVECTOR3& position);
	void setAbsoluteOrientation(const D3DXQUATERNION& orientation);

	void needTransformParentAABB(); //set flag only
	void needTransformChildren();  //set flag only
	void computeTransform();

	void onFrameMove(float delta);
	void onFrameRender( gRenderQueue& queue, const gCamera* camera );

protected:

	void nodeAABBChanged(); 
	void drawEntityList( gRenderQueue& queue );
	void updateEntityList( float delta );
	void drawAABB();

	gSceneNode() {}

	bool			m_isAABBVisible;
	bool			m_isTransformed;
	bool			m_isAABBChanged;

	D3DXMATRIX		m_absoluteMatrix;
	D3DXMATRIX		m_relativeMatrix;

	D3DXVECTOR3		m_relScale;
	D3DXVECTOR3		m_relPosition;
	D3DXQUATERNION	m_relOrientation;

	D3DXVECTOR3		m_absScale;
	D3DXVECTOR3		m_absPosition;
	D3DXQUATERNION	m_absOrientation;

	char			m_name[NAME_LENGHT];
	gAABB			m_AABB;

	gSceneNode* m_parent;
	std::map<std::string, gSceneNode*> m_children;
	std::map < std::string, gEntity* > m_entList;
	gSceneManager* m_sceneManager;
};

class gRenderQueue;

class gSceneManager
{
public:
	gSceneManager( gResourceManager* rmgr, gMaterialFactory* mfactory );
	~gSceneManager();

	gMaterialFactory* getMaterialFactory() const;
	gResourceManager* getResourseManager() const;

	gSceneNode& getRootNode() const;
	gSceneNode* getNode( const char* name ) const;
	gEntity* getEntity( const char* name ) const;

	gSceneNode* createNode( const char* name, gSceneNode* parent );
	gEntity* createEntity( const char* name );

	bool destroyNode( gSceneNode *node );
	bool destroyNode( const char* name );
	bool destroyEntity( const char* name );
	void destroyAllEntities();

	void frameRender( gRenderQueue& queue );
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
	gResourceManager* m_pResMgr;
	gMaterialFactory* m_pMatFactory;
	gSceneNode* m_pRootNode;
	std::map < std::string, gSceneNode* > m_nodeList;
	std::map < std::string, gEntity* > m_entList;

	//stats
	unsigned int m_entsInFrustum;
	unsigned int m_nodesInFrustum;
};


#endif

