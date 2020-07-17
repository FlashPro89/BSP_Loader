#include "Scene.h"
#include "Resources.h"
#include "Mesh.h"

//-----------------------------------------------
//
//	CLASS: gEntity
//
//-----------------------------------------------

gEntity::gEntity( const char* name )
{
	m_name = name;
	m_pHoldingNode = 0;
	m_pRenderable = 0;

	deleteAnimators();
}

gEntity::~gEntity()
{
	setRenderable(0); //renderable->release();

	deleteAnimators();
	auto it = m_userMaterials.begin();
	while ( it != m_userMaterials.end() )
	{
		if (it->second)
			it->second->release();
		it++;
	}
	m_userMaterials.clear();

	if (m_pRenderable)
		m_pRenderable->release();
}

const char* gEntity::getName() const
{
	return m_name.c_str();
}

void gEntity::onAttachToNode( gSceneNode* node )
{
	m_pHoldingNode = node;
}

void gEntity::onDetachFromNode()
{
	m_pHoldingNode = 0;
}

gSceneNode* gEntity::getHoldingNode() const
{
	return m_pHoldingNode;
}

void gEntity::setRenderable( gRenderable* renderable )
{

	if (m_pRenderable)
	{
		deleteAnimators();

		m_pRenderable->release();

		auto it = m_userMaterials.begin();
		while (it != m_userMaterials.end())
		{
			if (it->second)
				it->second->release();
			it++;
		}
		m_userMaterials.clear();
	}

	m_pRenderable = renderable;
	if (m_pRenderable != 0)
	{
		m_pRenderable->addRef();

		if ( m_pRenderable->getGroup() == GRESGROUP_SKINNEDMESH )
			m_animators[GANIMATOR_SKINNED] = new gSkinnedMeshAnimator(this);
		//((gSkinnedMeshAnimator*)m_animators[GANIMATOR_SKINNED])->getWordBonesMatrixes();
	}
}

const gRenderable* gEntity::getRenderable() const
{
	return m_pRenderable;
}

void gEntity::onFrameMove( float delta )
{
	for (unsigned int i = 0; i < GANIMATOR_NUM; i++)
	{
		if (m_animators[i])
			m_animators[i]->tick(delta);
	}
}

void gEntity::onFrameRender( gRenderQueue& queue, const gCamera* camera ) const
{
	if ( m_pRenderable && m_pHoldingNode )
	{
		if (m_pRenderable->isRenderable())
		{		
			//applyWorldMatrixesToRenderSystem
			if (m_pRenderable->getGroup() == GRESGROUP_SKINNEDMESH)
			{
				gResourceSkinnedMesh* pMesh = (gResourceSkinnedMesh*)m_pRenderable;
				gSkinnedMeshAnimator* pAnimator = (gSkinnedMeshAnimator*)m_animators[GANIMATOR_SKINNED];
				
				m_pRenderable->onFrameRender( &queue, this, camera );

				D3DXMATRIX mId;
				D3DXMatrixIdentity(&mId);
				if (pAnimator->getMixedFrame())
					pMesh->drawSkeleton(pAnimator->getMixedFrame(), &mId, 0, 0xFFFF0000);
			}
			else
				m_pRenderable->onFrameRender( &queue, this, camera );
		}
	}
}

const gAABB& gEntity::getAABB()
{
	m_AABB.reset();
	if (m_pRenderable)
		m_AABB =  m_pRenderable->getAABB();
	return m_AABB;
}

gAnimator* gEntity::getAnimator( GANIMATOR_TYPE type ) const
{
	return m_animators[type];
}

bool gEntity::setMaterial( gMaterial* pMaterial, short matIndex )
{
	if ( !m_pRenderable )
		return false;

	if ((matIndex >= 0) && (matIndex < m_userMaterials.size()))
	{
		auto it = m_userMaterials.begin();
		std::advance( it, matIndex );
		it->second = pMaterial;
		pMaterial->addRef();
	}
	else if (matIndex < 0)
	{
		for (unsigned short i = 0; i < m_pRenderable->getDefaultMaterialsNum(); i++)
		{
			gMaterial* pMaterial = m_pRenderable->getDefaultMaterialByIndex(i);
			m_userMaterials[pMaterial->getName()] = pMaterial;
			pMaterial->addRef();
		}
	}
	return true;
}

bool gEntity::setMaterialByName( gMaterial* pMaterial, const char* name )
{
	if ( (!m_pRenderable) || (name != 0) )
		return false;

	auto it = m_userMaterials.find(name);
	if (it != m_userMaterials.end())
	{
		it->second = pMaterial;
	}
	else
	{
		m_userMaterials[name] = pMaterial;
	}

	pMaterial->addRef();
	return true;
}

gMaterial* gEntity::getMaterial( short matIndex ) const
{
	if (!m_pRenderable)
		return 0;

	if ((matIndex >= 0) && (matIndex < m_userMaterials.size()))
	{
		auto it = m_userMaterials.begin();
		std::advance(it, matIndex);
		return it->second;
	}
	else
		return 0;
}

gMaterial* gEntity::getMaterialByName( const char* name ) const
{
	if ((!m_pRenderable) || (name != 0))
		return 0;

	auto it = m_userMaterials.find(name);
	if (it != m_userMaterials.end())
	{
		return it->second;
	}
	else
		return 0;
}

void gEntity::deleteAnimators()
{
	for (unsigned int i = 0; i < GANIMATOR_NUM; i++)
	{
		if (m_animators[i])
		{
			delete m_animators[i];
			m_animators[i] = 0;
		}
	}
}

//-----------------------------------------------
//
//	CLASS: gSceneNode
//
//-----------------------------------------------

gSceneNode::gSceneNode( const char* name, gSceneManager* mgr, gSceneNode* parent ) : 
	m_relPosition( 0.f, 0.f, 0.f ), m_relScale( 1.f, 1.f, 1.f ), 
	m_absPosition( 0.f, 0.f, 0.f ), m_absScale( 1.f, 1.f, 1.f )
{
	strcpy_s(m_name, NAME_LENGHT-1, name);
	m_name[NAME_LENGHT - 1] = 0;
	m_parent = parent;
	m_isTransformed = true;
	m_isAABBVisible = false; // false need
	m_isAABBChanged = false; // при создании узел не измен€ет AABB предка

	D3DXQuaternionIdentity( &m_relOrientation );
	D3DXQuaternionIdentity(&m_absOrientation);
	D3DXMatrixIdentity( &m_absoluteMatrix );
	D3DXMatrixIdentity(&m_relativeMatrix);
	m_sceneManager = mgr;
}

gSceneNode::~gSceneNode()
{
	detachAllEntities();
	destroyChildren();
	m_sceneManager->destroyNode(m_name);
}

const char* gSceneNode::getName() const
{
	return m_name;
}

const D3DXVECTOR3& gSceneNode::getRelativeScale() const
{
	return m_relScale;
}

const D3DXVECTOR3& gSceneNode::getRelativePosition() const
{
	return m_relPosition;
}

const D3DXQUATERNION& gSceneNode::getRelativeOrientation() const
{
	return m_relOrientation;
}

const D3DXVECTOR3& gSceneNode::getAbsoluteScale() const
{
	return m_absScale;
}

const D3DXVECTOR3& gSceneNode::getAbsolutePosition() const
{
	return m_absPosition;
}

const D3DXQUATERNION& gSceneNode::getAbsoluteOrientation() const
{
	return m_absOrientation;
}

const D3DXMATRIX& gSceneNode::getAbsoluteMatrix() const
{
	return m_absoluteMatrix;
}
const D3DXMATRIX& gSceneNode::getRelativeMatrix() const
{
	return m_relativeMatrix;
}

const gAABB& gSceneNode::getAABB() const
{
	return m_AABB;
}

gSceneNode* gSceneNode::createChild( const char* name )
{
	gSceneNode* pNode = m_sceneManager->createNode(name, this);
	m_children[name] = pNode;
	return pNode;
}

bool gSceneNode::destroyChild( const char* name )
{
	auto it = m_children.find(name);
	if (it == m_children.end())
		return false;
	return m_sceneManager->destroyNode( name );
}

void gSceneNode::destroyChildren()
{
	auto it = m_children.begin();
	while (it != m_children.end())
	{
		if (it->second)
		{
			//it->second->destroyChildren();
			m_sceneManager->destroyNode( it->second->getName());
		}
		it++;
	}
	m_children.clear();
}

void gSceneNode::attachEntity( gEntity* entity )
{
	m_entList[ entity->getName() ] = entity;
	entity->onAttachToNode( this );
	m_AABB.addAABB( entity->getAABB() );
	needTransformParentAABB();
}
void gSceneNode::detachEntity( gEntity* entity )
{
	auto it = m_entList.find(entity->getName());
	if ( it == m_entList.end() )
		return;
	m_entList.erase( it );
	entity->onDetachFromNode();
	needTransformParentAABB();
}

void gSceneNode::detachAllEntities()
{
	auto it = m_entList.begin();
	while (it != m_entList.end())
	{
		if (it->second)
			it->second->onDetachFromNode();
		it++;
	}
	m_entList.clear();
	needTransformParentAABB();
}

void gSceneNode::setRelativeScale(const D3DXVECTOR3 scale)
{	
	m_relScale = scale;
	needTransformChildren();
	needTransformParentAABB();
}

void gSceneNode::setRelativePosition(const D3DXVECTOR3& position)
{
	m_relPosition = position;
	needTransformChildren();
	needTransformParentAABB();
}

void gSceneNode::setRelativeOrientation(const D3DXQUATERNION& orientation)
{
	m_relOrientation = orientation;
	needTransformChildren();
	needTransformParentAABB();
}


void gSceneNode::setAbsoluteScale(const D3DXVECTOR3 scale)
{
	throw("not impl");
}

void gSceneNode::setAbsolutePosition(const D3DXVECTOR3& position)
{
	throw("not impl");
}

void gSceneNode::setAbsoluteOrientation(const D3DXQUATERNION& orientation)
{
	throw("not impl");
}


void gSceneNode::needTransformParentAABB()
{
	m_isAABBChanged = true;
	if (m_parent)
		m_parent->needTransformParentAABB();
}

void gSceneNode::needTransformChildren() //??
{
	m_isTransformed = true;
	auto it = m_children.begin();
	while (it != m_children.end() )
	{
		if (it->second)
			it->second->needTransformChildren();
		it++;
	}
}

void gSceneNode::computeTransform()
{
	D3DXMATRIX scale, pos, tmp, rot;
	if (m_isTransformed)
	{
		D3DXMatrixScaling(&scale, m_relScale.x, m_relScale.y, m_relScale.z);
		D3DXMatrixRotationQuaternion(&rot, &m_relOrientation);
		D3DXMatrixTranslation(&pos, m_relPosition.x, m_relPosition.y, m_relPosition.z);

		D3DXMatrixMultiply(&tmp, &scale, &rot);
		D3DXMatrixMultiply(&m_relativeMatrix, &tmp, &pos);

		if (m_parent == 0) // ROOT node
		{
			m_absoluteMatrix = m_relativeMatrix;
			m_absPosition = m_relPosition;
			m_absOrientation = m_relOrientation;
			m_absScale = m_relScale;
		}
		else
		{
			D3DXMatrixMultiply( &m_absoluteMatrix, &m_relativeMatrix, &m_parent->getAbsoluteMatrix() );
			m_absPosition = D3DXVECTOR3( m_absoluteMatrix._41, m_absoluteMatrix._42, m_absoluteMatrix._43 );
			
			float scaleX = powf((m_absoluteMatrix._11 * m_absoluteMatrix._11 + m_absoluteMatrix._12 * m_absoluteMatrix._12 + m_absoluteMatrix._13 * m_absoluteMatrix._13), 0.5f);
			float scaleY = powf((m_absoluteMatrix._21 * m_absoluteMatrix._21 + m_absoluteMatrix._22 * m_absoluteMatrix._22 + m_absoluteMatrix._23 * m_absoluteMatrix._23), 0.5f);
			float scaleZ = powf((m_absoluteMatrix._31 * m_absoluteMatrix._31 + m_absoluteMatrix._32 * m_absoluteMatrix._32 + m_absoluteMatrix._33 * m_absoluteMatrix._33), 0.5f);

			m_absScale = D3DXVECTOR3( scaleX, scaleY, scaleZ );
			D3DXQuaternionRotationMatrix( &m_absOrientation, &m_absoluteMatrix );
		}
		m_isTransformed = false;
	}


	auto it = m_children.begin();
	while( it != m_children.end() )
	{
		if (it->second)  it->second->computeTransform();
		it++;
	}

	if (m_isAABBChanged)
	{
		nodeAABBChanged();
	}
}

void gSceneNode::onFrameMove(float delta)
{
	updateEntityList( delta );
	auto it = m_children.begin();
	while (it != m_children.end())
	{
		it->second->onFrameMove( delta );
		it++;
	}
}

void gSceneNode::onFrameRender( gRenderQueue& queue, const gCamera* camera )
{	
	drawEntityList( queue );

	if (m_isAABBVisible)
		drawAABB();

	auto it = m_children.begin();
	while (it != m_children.end())
	{
		if (it->second)
		{
			//const gCamera* cam = m_sceneManager->getActiveCamera();
			if ( (!it->second->getAABB().isEmpty()) && (camera!=0) )
			{
	
				if( camera->getViewingFrustum().testAABB( it->second->getAABB() ) )
					it->second->onFrameRender( queue, camera );
			}
			else
				it->second->onFrameRender( queue, camera );
		}
		it++;
	}
	m_sceneManager->__statNodeDraw();
}

void gSceneNode::nodeAABBChanged()
{
	m_AABB.reset();

	gAABB tmp;

	auto it_ent = m_entList.begin();
	while (it_ent != m_entList.end())
	{
		if (it_ent->second)
		{
			if (!it_ent->second->getAABB().isEmpty())
			{
				it_ent->second->getAABB().getTransformedByMatrix( &tmp, m_absoluteMatrix );
				m_AABB.addAABB( tmp );
			}
		}
		it_ent++;
	}

	auto it = m_children.begin();
	while (it != m_children.end())
	{
		if (it->second)
		{
			if ( !it->second->getAABB().isEmpty() )
			{
				m_AABB.addAABB( it->second->getAABB() ); // узлы потомков уже содержат трансформированные AABB 
			}
		}
		it++;
	}

//	if (m_parent)
//		m_parent->nodeAABBChanged();
}

void gSceneNode::drawEntityList( gRenderQueue& queue )
{
	gAABB tmp;
	const gCamera* cam = 0;
	auto it = m_entList.begin();
	while (it != m_entList.end())
	{
		if (it->second)
		{
			cam = m_sceneManager->getActiveCamera();
			if (cam)
			{
				it->second->getAABB().getTransformedByMatrix(&tmp, m_absoluteMatrix);
				if (cam->getViewingFrustum().testAABB(tmp))
				{
					m_sceneManager->__statEntDraw();
					it->second->onFrameRender( queue, cam );
				}
			}
		}
		it++;
	}
}

void gSceneNode::updateEntityList(float delta)
{
	auto it = m_entList.begin();
	while (it != m_entList.end())
	{
		if (it->second)
		{
			it->second->onFrameMove(delta);
			//gRenderable* renderable = it->second->getRenderable();
			//if (renderable)
			//{
			//	if (renderable->isRenderable())
			//		renderable->onFrameMove(delta);
			//}
		}
		it++;
	}
}

void gSceneNode::drawAABB()
{

	D3DXVECTOR3 bmin = m_AABB.getMinBounds();
	D3DXVECTOR3 bmax = m_AABB.getMaxBounds();

	gVertexAABB points[8] =
	{
		gVertexAABB(	D3DXVECTOR3(bmin.x, bmin.y, bmin.z), 0xFF00FF00 ),
		gVertexAABB(	D3DXVECTOR3(bmax.x, bmin.y, bmin.z), 0xFF00FF00 ),
		gVertexAABB(	D3DXVECTOR3(bmax.x, bmax.y, bmin.z), 0xFF00FF00 ),
		gVertexAABB(	D3DXVECTOR3(bmin.x, bmax.y, bmin.z), 0xFF00FF00 ),

		gVertexAABB(	D3DXVECTOR3(bmin.x, bmin.y, bmax.z), 0xFF00FF00 ),
		gVertexAABB(	D3DXVECTOR3(bmax.x, bmin.y, bmax.z), 0xFF00FF00 ),
		gVertexAABB(	D3DXVECTOR3(bmax.x, bmax.y, bmax.z), 0xFF00FF00 ),
		gVertexAABB(	D3DXVECTOR3(bmin.x, bmax.y, bmax.z), 0xFF00FF00 ),
	};

	unsigned short ind[24] =
	{
		0,1,  1,2, 2,3, 3,0,

		4,5, 5,6, 6,7, 7,4,

		0,4, 1,5, 2,6, 3,7
	};

	D3DXMATRIX m;
	D3DXMatrixIdentity(&m); 

	LPDIRECT3DDEVICE9 pDev = m_sceneManager->getResourseManager()->getDevice();
	if (!pDev)return;

	DWORD oldLightingState;
	pDev->GetRenderState(D3DRS_LIGHTING, &oldLightingState);
	pDev->SetRenderState(D3DRS_LIGHTING, false);

	pDev->SetTransform( D3DTS_WORLD, &m );
	pDev->SetTexture(0, 0);

	DWORD fvf;
	pDev->GetFVF(&fvf);
	pDev->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	pDev->DrawIndexedPrimitiveUP( D3DPT_LINELIST, 0, 8, 12, &ind, D3DFMT_INDEX16, &points, sizeof(gVertexAABB) );
	pDev->SetFVF(fvf);
	pDev->SetRenderState(D3DRS_LIGHTING, oldLightingState);

	//pDev->SetSamplerState( 0, D3DSA)
}

//-----------------------------------------------
//
//	CLASS: gSceneManager
//
//-----------------------------------------------

gSceneManager::gSceneManager( gResourceManager* rmgr, gMaterialFactory* mfactory ) : m_rootNode( "root", this, 0 )
{
	m_pResMgr = rmgr;
	m_activeCam = 0;
	m_entsInFrustum = 0;
	m_nodesInFrustum = 0;

	m_pMatFactory = mfactory;

	//test
	//m_pMatFactory->createMaterial("default");
	//m_pMatFactory->createMaterial("default1");
	//m_pMatFactory->createMaterial("default2");
	//m_pMatFactory->createMaterial("default3");
	//m_pMatFactory->createMaterial("default4");
}

gSceneManager::~gSceneManager()
{

}

gMaterialFactory* gSceneManager::getMaterialFactory() const
{
	return m_pMatFactory;
}

gResourceManager* gSceneManager::getResourseManager() const
{
	return m_pResMgr;
}

gSceneNode& gSceneManager::getRootNode() const
{
	return m_rootNode;
}

gSceneNode* gSceneManager::getNode( const char* name ) const
{
	auto it = m_nodeList.find(name);

	if (it != m_nodeList.end())
		return it->second;
	else
		return (gSceneNode*)0;
}

gEntity* gSceneManager::getEntity( const char* name ) const
{
	auto it = m_entList.find(name);

	if (it != m_entList.end())
		return it->second;
	else
		return (gEntity*)0;
}


gSceneNode* gSceneManager::createNode( const char* name, gSceneNode* parent )
{
	auto it = m_nodeList.find(name);
	if (it != m_nodeList.end())			//уже существует
		return (gSceneNode*)0;
	gSceneNode* pNode = new gSceneNode( name, this, parent );
	m_nodeList[name] = pNode;
	return pNode;
}

gEntity* gSceneManager::createEntity( const char* name )
{
	if (!name) 
		return 0;

	auto it = m_entList.find( name );
	if (it != m_entList.end())			//уже существует
		return (gEntity*)0;
	gEntity* pEnt = new gEntity(name);

	//test
	//pEnt->setMaterial(m_pMatFactory->getMaterial("default"));

	m_entList[name] = pEnt;
	return pEnt;
}

// TODO?::
bool gSceneManager::destroyNode( gSceneNode* node )
{
	return false; // TODO?::
}

bool gSceneManager::destroyNode( const char* name )
{
	auto it = m_nodeList.find(name);
	
	if( it == m_nodeList.end() )
		return false;

	auto ptr = it->second;
	m_nodeList.erase(it);
	delete ptr;

	return true;
}

bool gSceneManager::destroyEntity( const char* name )
{
	auto it = m_entList.find(name);

	if (it == m_entList.end())
		return false;

	if (it->second != 0)
		delete it->second;

	m_entList.erase( it );
	return true;
}

// !!!! в функции не удал€ютс€ указатели на Entity в узлах сцены
void gSceneManager::destroyAllEntities( ) 
{
	auto it = m_entList.begin();
	while (it != m_entList.end())
	{
		if (it->second)
		{
			
			delete it->second;
		}
		it++;
	}
	m_entList.clear();
	return;
}

void gSceneManager::frameRender( gRenderQueue& queue )
{
	m_entsInFrustum = 0;
	m_nodesInFrustum = 0;

	if (!m_pResMgr) return;

	LPDIRECT3DDEVICE9 pD3DDev = m_pResMgr->getDevice();

	queue.clear();

	if ( m_activeCam && pD3DDev )
	{
		pD3DDev->SetTransform(D3DTS_VIEW, &m_activeCam->getViewMatrix());
		pD3DDev->SetTransform(D3DTS_PROJECTION, &m_activeCam->getProjMatrix());

		if (m_activeCam->getViewingFrustum().testAABB(m_rootNode.getAABB()))
			m_rootNode.onFrameRender( queue, m_activeCam );
	}

	queue._debugOut("out_queue_not_sorted.txt");
	queue.sort();
	queue._debugOut("out_queue_sorted.txt");


	gRenderElement* pElement = 0;
	int counter = 0;
	while (queue.popBack(&pElement))
	{
		counter++;
	}
}

void gSceneManager::frameMove(float delta)
{
	if( m_activeCam )
		m_activeCam->tick( delta );

	m_rootNode.onFrameMove(delta);
	m_rootNode.computeTransform();
}

void gSceneManager::setActiveCamera(gCamera* cam)
{
	m_activeCam = cam;
}

const gCamera* gSceneManager::getActiveCamera() const
{
	return m_activeCam;
}

void gSceneManager::__statEntDraw()
{
	m_entsInFrustum++;
}

void gSceneManager::__statNodeDraw()
{
	m_nodesInFrustum++;
}

unsigned int	gSceneManager::__statEntitiesInFrustum() const // num drawed entities in activeCam
{
	return m_entsInFrustum;
}

unsigned int	gSceneManager::__statNodesInFrustum() const // num drawed entities in activeCam
{
	return m_nodesInFrustum;
}