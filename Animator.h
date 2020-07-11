#pragma once

#ifndef _ANIMATOR_H_
#define _ANIMATOR_H_

#include <map>
#include <string>
#include "Mesh.h"

enum GSKINANIM_TYPE
{
	GSKINANIM_LOOP,
	GSKINANIM_REVERSE_LOOP,
	GSKINANIM_PLAY_ONE_TIME,		// delete after play
	GSKINANIM_REVERSE_PLAY_ONE_TIME // delete after play
};

enum GANIMPLAY_TYPE
{
	GANIMPLAY_START,
	GANIMPLAY_CURRENTPOS,
	GANIMPLAY_END
};

enum GANIMATOR_TYPE
{
	GANIMATOR_SKINNED,
	GANIMATOR_CHARACTER, //управление с устройств ввода или сеть
	GANIMATOR_PATH,
	GANIMATOR_CLOSEDPATH,
	GANIMATOR_ROTATOR,
	GANIMATOR_NUM
};

class gAnimator
{
public:
	virtual ~gAnimator() {}

	virtual void tick( float delta ) = 0;
	virtual void stop() = 0;
	virtual void play() = 0;

	GANIMATOR_TYPE getType() const;

protected:
	gAnimator() {}
	gAnimator(gAnimator&) {}

	GANIMATOR_TYPE m_type;
};

class gSkinnedMeshAnimationTrack;
typedef std::map<std::string, gSkinnedMeshAnimationTrack*> gSkinnedMeshAnimationTracks;

class gSkinnedMeshAnimator : public gAnimator
{
public:
	gSkinnedMeshAnimator( gEntity* entity );
	~gSkinnedMeshAnimator();

	void tick(float delta);
	void clear();

	void stop();
	void play();

	unsigned char getTracksNum() const;

	bool removeTrack( const char* name );
	gSkinnedMeshAnimationTrack* getTrack( const char* name ) const;
	gSkinnedMeshAnimationTrack* addTrack( const char* name,
		GSKINANIM_TYPE type = GSKINANIM_LOOP);

	const gSkinBone* getMixedFrame() const; //получаем набор трансформированных костей дл€ рендера в мировой системе отсчета
	const D3DXMATRIX* getWordBonesMatrixes();

protected:
	gSkinnedMeshAnimationTracks m_tracks;
	gEntity* m_pEntity;  // ????
	//gResourceSkinnedMesh* m_pMesh;
	gSkinBone* m_mixedFrame;
	D3DXMATRIX* m_worldBonesMatrixes;
	unsigned char m_bonesNum;
};

class gSkinnedMeshAnimationTrack
{
public:
	gSkinnedMeshAnimationTrack(gResourceSkinAnimation* animation);
	~gSkinnedMeshAnimationTrack();

	const gResourceSkinAnimation* getAnimation() const; //дл€ интерпол€тора

	void setAnimationType(GSKINANIM_TYPE type = GSKINANIM_LOOP);
	GSKINANIM_TYPE getAnimationType() const;

	void setFPS(float fps);
	float getFPS() const;

	void setTimePosition(float time);
	float getTimePosition() const;

	void tick(float delta);
	bool isNeedToDelete();

	gSkinBone* getCurrentFrame() const;

	bool isStoped() const;
	void stop();
	void play();

protected:
	gResourceSkinAnimation* m_pAnimation; 
	bool m_isNeedDelete;
	bool m_isStoped;
	GSKINANIM_TYPE m_animType;
	float m_FPS;
	float m_timePosition;
	gSkinBone* m_currentFrame;
};



#endif
