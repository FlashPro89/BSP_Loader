#include "Animator.h"
#include "gmath.h"
#include "Scene.h"


//-----------------------------------------------
//
//	CLASS: gAnimator
//
//-----------------------------------------------

GANIMATOR_TYPE gAnimator::getType() const
{
	return m_type;
}

//-----------------------------------------------
//
//	CLASS: gSkinnedMeshAnimator
//
//-----------------------------------------------

gSkinnedMeshAnimator::gSkinnedMeshAnimator( gEntity* entity )
{
	m_pEntity = entity;

	gResourceSkinnedMesh* pSMesh = (gResourceSkinnedMesh*)m_pEntity->getRenderable();
	m_bonesNum = pSMesh->getBonesNum();
	pSMesh->release();

	m_mixedFrame = new gSkinBone[m_bonesNum];
	m_worldBonesMatrixes = new D3DXMATRIX[m_bonesNum];
	m_type = GANIMATOR_SKINNED;
}

gSkinnedMeshAnimator::~gSkinnedMeshAnimator()
{
	if (m_mixedFrame)
		delete[] m_mixedFrame;
	if (m_worldBonesMatrixes)
		delete m_worldBonesMatrixes;
	clear();
}

void gSkinnedMeshAnimator::tick(float delta)
{
	//удаляем проигранные до конца анимации
	auto it = m_tracks.begin();
	while (it != m_tracks.end())
	{
		if (it->second->isNeedToDelete())
		{
			delete it->second;
			it = m_tracks.erase(it);
		}
		else
		{
			it->second->tick(delta);
			it++;
		}
	}

	//вычисление смешанного ADD кадра
	it = m_tracks.begin();

	for (int i = 0; i < m_bonesNum; i++)
	{
		m_mixedFrame[i].setPosition( D3DXVECTOR3(0.f, 0.f, 0.f) );
		m_mixedFrame[i].setOrientation(D3DXQUATERNION(0.f, 0.f, 0.f, 1.f)); // Id
	}

	while (it != m_tracks.end())
	{
		for (int i = 0; i < m_bonesNum; i++)
		{
			m_mixedFrame[i].move( it->second->getCurrentFrame()[i].getPosition() );
			m_mixedFrame[i].rotate(it->second->getCurrentFrame()[i].getOrientation());
		}
		it++;
	}

	//переводим в мировую систему координат ( трансформация по иерархии костей )
	_transformHierarchyFrameBones( m_mixedFrame, -1, m_bonesNum );

	D3DXMATRIX mTr, mRot, mAbs;
	D3DXVECTOR3 v; 
	D3DXQUATERNION q;

	gResourceSkinnedMesh* pSMesh = (gResourceSkinnedMesh*)m_pEntity->getRenderable();
	const D3DXMATRIX* mInverted = pSMesh->getInvertedMatrixes();

	for (int i = 0; i < m_bonesNum; i++ )
	{
		v = m_mixedFrame[i].getPosition();
		q = m_mixedFrame[i].getOrientation();

		//D3DXMatrixTranslation(&mTr, v.x, v.y, v.z);
		//D3DXMatrixRotationQuaternion(&mRot, &q);
		//D3DXMatrixMultiply(&mAbs, &mRot, &mTr);

		D3DXMatrixRotationQuaternion(&mAbs, &q);
		mAbs._41 = v.x; mAbs._42 = v.y; mAbs._43 = v.z;
		D3DXMatrixMultiply( &m_worldBonesMatrixes[i], &mInverted[i], &mAbs );
		
	}

	pSMesh->release();
}

void gSkinnedMeshAnimator::clear()
{
	auto it = m_tracks.begin();
	while (it != m_tracks.end())
	{
		delete it->second;
		it++;
	}
	m_tracks.clear();
}

void gSkinnedMeshAnimator::stop()
{
	auto it = m_tracks.begin();
	while (it != m_tracks.end())
	{
		it->second->stop();
	}
}

void gSkinnedMeshAnimator::play()
{
	auto it = m_tracks.begin();
	while (it != m_tracks.end())
	{
		it->second->play();
	}
}

unsigned char gSkinnedMeshAnimator::getTracksNum() const
{
	return (unsigned char)m_tracks.size();
}

bool gSkinnedMeshAnimator::removeTrack(const char* name)
{
	auto it = m_tracks.find(name);
	if (it != m_tracks.end())
	{
		delete it->second;
		m_tracks.erase(it);
		return true;
	}
	else
		return false;
}

const gSkinnedMeshAnimationTrack* gSkinnedMeshAnimator::getTrack(const char* name) const
{
	auto it = m_tracks.find(name);
	if (it != m_tracks.end())
	{
		return it->second;
	}
	else
		return (gSkinnedMeshAnimationTrack*)0;
}

gSkinnedMeshAnimationTrack* gSkinnedMeshAnimator::addTrack(const char* name,
	GSKINANIM_TYPE type)
{
	gResourceSkinnedMesh* pSMesh = (gResourceSkinnedMesh*)m_pEntity->getRenderable();
	gSkinnedMeshAnimationTrack* track = 0;

	gResourceSkinAnimation* anim = pSMesh->getAnimation(name);
	if (!anim)
	{
		pSMesh->release();
		return track;
	}

	auto it = m_tracks.find(name);
	if (it == m_tracks.end())
	{
		track = new gSkinnedMeshAnimationTrack(anim);
		track->setAnimationType(type);
		m_tracks[name] = track;
	}

	//else //пока что возвращаем 0 чтобы исключить влияние на уже используемые анимации

	pSMesh->release();
	return track;
}

const gSkinBone* gSkinnedMeshAnimator::getMixedFrame() const
{
	return m_mixedFrame;
}

const D3DXMATRIX* gSkinnedMeshAnimator::getWordBonesMatrixes()
{
	return m_worldBonesMatrixes;
}

//-----------------------------------------------
//
//	CLASS: gSkinnedMeshAnimationTrack
//
//-----------------------------------------------

gSkinnedMeshAnimationTrack::gSkinnedMeshAnimationTrack(gResourceSkinAnimation* animation)
{
	m_pAnimation = animation; 
	m_FPS = SKINNING_FPS_DEFAULT;
	m_timePosition = 0.f;
	m_isStoped = true;
	m_animType = GSKINANIM_LOOP;
	m_isNeedDelete = false;
	m_currentFrame = new gSkinBone[ animation->getBonesNum() ]; //zeroMem?
}

gSkinnedMeshAnimationTrack::~gSkinnedMeshAnimationTrack()
{
	if (m_currentFrame)
		delete[] m_currentFrame;
}

const gResourceSkinAnimation* gSkinnedMeshAnimationTrack::getAnimation() const
{
	return m_pAnimation;
}

void gSkinnedMeshAnimationTrack::setAnimationType(GSKINANIM_TYPE type)
{
	m_animType = type;
	if ((type == GSKINANIM_LOOP) || (type == GSKINANIM_PLAY_ONE_TIME))
		m_timePosition = 0;
	if ((type == GSKINANIM_REVERSE_LOOP) || (type == GSKINANIM_REVERSE_PLAY_ONE_TIME))
		m_timePosition = (float)m_pAnimation->getFramesNum() - 1;
}

GSKINANIM_TYPE gSkinnedMeshAnimationTrack::getAnimationType() const
{
	return m_animType;
}

void gSkinnedMeshAnimationTrack::setFPS(float fps)
{
	m_FPS = fps;
}

float gSkinnedMeshAnimationTrack::getFPS() const
{
	return m_FPS;
}

void gSkinnedMeshAnimationTrack::setTimePosition(float time)
{
	m_timePosition = time;
}

float gSkinnedMeshAnimationTrack::getTimePosition() const
{
	return m_timePosition;
}

void gSkinnedMeshAnimationTrack::tick(float delta)
{
	if (m_isStoped)
		return;

	switch (m_animType)
	{
	case GSKINANIM_LOOP:
		if ((unsigned char)(m_timePosition + delta * m_FPS) < (m_pAnimation->getFramesNum() - 1))
			m_timePosition += delta * m_FPS;
		else
			m_timePosition = 0.f;
		break;

	case GSKINANIM_PLAY_ONE_TIME:
		if ((unsigned char)(m_timePosition + delta * m_FPS) < (m_pAnimation->getFramesNum() - 1))
			m_timePosition += delta * m_FPS;
		else
			m_isNeedDelete = true;
		break;

	case GSKINANIM_REVERSE_LOOP:
		if ((unsigned char)(m_timePosition - delta * m_FPS) > 0)
			m_timePosition -= delta * m_FPS;
		else
			m_timePosition = (float)m_pAnimation->getFramesNum() - 1;
		break;

	case GSKINANIM_REVERSE_PLAY_ONE_TIME:
		if ((unsigned char)(m_timePosition - delta * m_FPS) > 0)
			m_timePosition -= delta * m_FPS;
		else
			m_isNeedDelete = true;
		break;
	}

	//update current frame
	if (!m_isNeedDelete)
	{
		m_pAnimation->getFrameInTimePos( m_timePosition, m_currentFrame );
	}
}

bool gSkinnedMeshAnimationTrack::isNeedToDelete()
{
	return m_isNeedDelete;
}

gSkinBone* gSkinnedMeshAnimationTrack::getCurrentFrame() const
{
	return m_currentFrame;
}

bool gSkinnedMeshAnimationTrack::isStoped() const
{
	return m_isStoped;
}

void gSkinnedMeshAnimationTrack::stop()
{
	m_isStoped = true;
}

void gSkinnedMeshAnimationTrack::play()
{
	m_isStoped = false;
}