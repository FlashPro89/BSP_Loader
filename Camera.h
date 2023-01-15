#pragma once

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "input.h"
#include "coldet.h"
#include <d3dx9.h>


class gCamera;

class gViewingFrustum
{
public:
	gViewingFrustum(gCamera* cam);

	bool testPoint( float x, float y, float z )  const;
	bool testPoint( const D3DXVECTOR3& point )  const;
	bool testAABB( const D3DXVECTOR3& bbMin, const D3DXVECTOR3& bbMax )  const;
	bool testAABB( const gAABB& bbox )  const;

	void updatePlanes();

protected:

	gViewingFrustum();

	union
	{
		struct {
			D3DXPLANE m_nearPlane, m_farPlane, m_leftPlane, m_rightPlane, m_topPlane, m_bottomPlane;
		};
		struct {
			D3DXPLANE m_planes[6];
		};
	};

	gCamera* m_pCam;
};

class gCamera
{
public:
	gCamera( gInput* input );
	gCamera();
	~gCamera();

	void tick(float dt);

	const D3DXMATRIX& getViewMatrix() const;
	const D3DXMATRIX& getProjMatrix() const;
	const D3DXMATRIX& getViewProjMatrix() const;

	const D3DXVECTOR3& getPosition( ) const;
	const D3DXQUATERNION& getOrientation( ) const;
	float getAspectRatio() const;
	float getFOV() const;

	//test
	float getYaw() const;
	float getPitch() const;

	const gViewingFrustum& getViewingFrustum() const;

	void setPosition( const D3DXVECTOR3& vec );
	void setOrientation( const D3DXQUATERNION& q );
	void setOrientation(const D3DXVECTOR3& dir);
	void setAspectRatio( float aspectRatio );
	void setFOV( float FOV );


	void lookAt( const D3DXVECTOR3& target );
	void lookAt( const D3DXVECTOR3& target, const D3DXVECTOR3& newCamPosition );

	void setMovementSpeed( float speed );
	void setRotationSpeed( float speed );
	void setDefaults();
	void setInput( gInput* input );

	void projPointToScreen( const D3DXVECTOR3& point, 
		D3DXVECTOR3& outPoint, const D3DVIEWPORT9& viewport ) const;

	float getDistanceToPointF(const D3DXVECTOR3& vec) const;
	unsigned short getDistanceToPointUS(const D3DXVECTOR3& vec) const;

protected:
	
	void recompMatrices();
	void _YPtoQuat();

	gInput* m_input;
	D3DXQUATERNION m_rot;
	float m_yaw; 
	float m_pitch;
	D3DXVECTOR3     m_pos;
	//D3DXVECTOR3     m_lookAt;
	float m_tspeed, m_rspeed, m_aspect, m_FOV, m_fPlane,m_nPlane;
	gViewingFrustum m_frustum;
	D3DXMATRIX m_mview;
	D3DXMATRIX m_mproj;
	D3DXMATRIX m_mviewproj;

	float prev_mouse_x = 0.f;
	float prev_mouse_y = 0.f;
	float prev_vel_f = 0.f;
	float prev_vel_s = 0.f;
	float prev_linear_speed = 0.f;
};


#endif
