#include "Camera.h"
#include "gmath.h"

#define DEFAULT_CAM_TSPEED 1200.f
#define DEFAULT_CAM_RSPEED 0.9f
#define DEFAULT_CAM_FOV    D3DX_PI / 4.0f
#define DEFAULT_CAM_FPLANE 15000.f
#define DEFAULT_CAM_NPLANE 1.f
#define DEFAULT_CAM_ASPECT 1.333f
#define MOUSE_MAX_MOVEMENT 15

gCamera::gCamera( gInput* input ) : m_frustum(this)
{
	m_input = input;
	setDefaults();
	m_pos = D3DXVECTOR3(0, 0, 0);
	D3DXQuaternionIdentity(&m_rot);
	recompMatrices();
}

gCamera::gCamera() : m_frustum(this)
{
	m_input = 0;
	setDefaults();
	m_pos = D3DXVECTOR3(0, 0, 0);
	D3DXQuaternionIdentity(&m_rot);
	recompMatrices();
}

gCamera::~gCamera()
{

}

const D3DXMATRIX& gCamera::getViewMatrix()  const
{
	return m_mview;
}
const D3DXMATRIX& gCamera::getProjMatrix()  const
{
	return m_mproj;
}

const D3DXMATRIX& gCamera::getViewProjMatrix()  const
{
	return m_mviewproj;
}

const gViewingFrustum& gCamera::getViewingFrustum()  const
{
	return m_frustum;
}

void gCamera::tick(float dt)
{
	bool changed = false;

	if (m_input)
	{
		
		if (m_input->isMousePressed(0))
		{
			changed = true;

			int x = m_input->getMouseX();
			int y = m_input->getMouseY();

			if (abs(x) > MOUSE_MAX_MOVEMENT)
			{
				if (x < 0)
					x = MOUSE_MAX_MOVEMENT;
				else
					x = -MOUSE_MAX_MOVEMENT;
			}

			if (abs(y) > MOUSE_MAX_MOVEMENT)
			{
				if (y < 0)
					y = MOUSE_MAX_MOVEMENT;
				else
					y = -MOUSE_MAX_MOVEMENT;
			}

			m_yaw += x * dt * m_rspeed;

			m_pitch += y * dt * m_rspeed;
			if (m_pitch > D3DX_PI / 2.5f)
				m_pitch = D3DX_PI / 2.5f;
			if (m_pitch < -D3DX_PI / 2.5f)
				m_pitch = -D3DX_PI / 2.5f;

			_YPtoQuat();
		}


		D3DXMATRIX mat_dir;
		D3DXMatrixRotationQuaternion(&mat_dir, &m_rot);

		D3DXVECTOR3 dir_forward(0, 0, 1);
		D3DXVECTOR3 dir_left(-1, 0, 0);
		D3DXVECTOR3 dir_up(0, 1, 0);
		
		D3DXVec3TransformCoord(&dir_forward, &dir_forward, &mat_dir);
		D3DXVec3Normalize(&dir_forward, &dir_forward);
		D3DXVec3Cross(&dir_left, &dir_forward, &dir_up);
		D3DXVec3Normalize(&dir_left, &dir_left);

		
		if (m_input->isKeyPressed(DIK_W))
		{
			m_pos += dir_forward * dt * m_tspeed;
			changed = true;
		}
		if (m_input->isKeyPressed(DIK_S))
		{
			m_pos -= dir_forward * dt * m_tspeed;
			changed = true;
		}
		if (m_input->isKeyPressed(DIK_A))
		{
			m_pos += dir_left * dt * m_tspeed;
			changed = true;
		}
		if (m_input->isKeyPressed(DIK_D))
		{
			m_pos -= dir_left * dt * m_tspeed;
			changed = true;
		}

	}

	if (changed)
	{
		recompMatrices();
	}
}

void gCamera::setMovementSpeed(float speed)
{
	m_tspeed = speed;
}
void gCamera::setRotationSpeed(float speed)
{
	m_rspeed = speed;
}

void gCamera::setDefaults()
{
	m_rspeed = DEFAULT_CAM_RSPEED;
	m_tspeed = DEFAULT_CAM_TSPEED;
	m_FOV = DEFAULT_CAM_FOV;
	m_fPlane = DEFAULT_CAM_FPLANE;
	m_nPlane = DEFAULT_CAM_NPLANE;
	m_aspect = DEFAULT_CAM_ASPECT;
	m_yaw = 0;
	m_pitch = 0;

	D3DXMatrixIdentity(&m_mview);
	D3DXMatrixIdentity(&m_mproj);
	D3DXMatrixIdentity(&m_mviewproj);
}

void gCamera::setInput(gInput* input)
{
	m_input = input;

	m_yaw = D3DX_PI / 2;
	_YPtoQuat();

	m_yaw = D3DX_PI;
	_YPtoQuat();

	m_yaw = 0;
	m_pitch = D3DX_PI / 2;
	_YPtoQuat();

	m_pitch = D3DX_PI;
	_YPtoQuat();

	m_pitch = 0;
	_YPtoQuat();
}

void gCamera::projPointToScreen(const D3DXVECTOR3& point, D3DXVECTOR3& outPoint, const D3DVIEWPORT9& viewport ) const
{
	//D3DXVec3Transform( &outPoint, &point, &m_mviewproj );
	D3DXMATRIX mId;
	D3DXMatrixIdentity(&mId);
	D3DXVec3Project(&outPoint, &point, &viewport, &m_mviewproj, &mId, &mId);

	D3DXVECTOR4 out;
	D3DXVec3Transform(&out, &point, &m_mviewproj);
	out.x = out.x / out.w;
	out.y = out.y / out.w;

	DWORD halfW = (viewport.Width - viewport.X) / 2;
	DWORD halfH = (viewport.Height - viewport.Y) / 2;

	out.x *= halfW; 
	out.y *= halfH;

	out.x += halfW;
	out.y = halfH - out.y;

	out.x += viewport.X;
	out.y += viewport.Y;

	out.z = out.z / out.w - 1.f;

	outPoint = D3DXVECTOR3( out.x, out.y, out.z );
}

const D3DXVECTOR3& gCamera::getPosition() const
{
	return m_pos;
}

const D3DXQUATERNION& gCamera::getOrientation()  const
{
	return m_rot;
}

float gCamera::getAspectRatio() const
{
	return m_aspect;
}

float gCamera::getFOV() const
{
	return m_FOV;
}


float gCamera::getYaw() const
{
	return m_yaw;
}

float gCamera::getPitch() const
{
	return m_pitch;
}

void gCamera::setPosition(const D3DXVECTOR3& vec)
{
	m_pos = vec;
	recompMatrices();
}

void gCamera::setOrientation( const D3DXQUATERNION& q )
{
	m_rot = q;
	recompMatrices();
}

void gCamera::setOrientation(const D3DXVECTOR3& dir)
{
	D3DXQUATERNION q;
	float yaw, pitch, roll;
	_quatShortestArc(q, D3DXVECTOR3(0, 0, 1.f), dir);
	_quatToYawPitchRoll(q, &yaw, &pitch, &roll );

	m_yaw = yaw;
	m_pitch = pitch;
	_YPtoQuat();

	recompMatrices();
}

void gCamera::setAspectRatio(float aspectRatio)
{
	m_aspect = aspectRatio;

	recompMatrices();
}

void gCamera::setFOV(float FOV)
{
	m_FOV = FOV;
	recompMatrices();
}

void gCamera::lookAt( const D3DXVECTOR3& target )
{
	D3DXVECTOR3 dir = target - m_pos;
	D3DXVec3Normalize( &dir, &dir );

	D3DXQUATERNION q;
	float yaw, pitch, roll;
	_quatShortestArc( q, D3DXVECTOR3(0, 0, 1.f), dir );
	_quatToYawPitchRoll( q, &yaw, &pitch, &roll );

	_YawPitchRolltoQuat(q, yaw, pitch, roll);

	m_pitch = pitch;
	m_yaw = yaw;

	_YPtoQuat();
	recompMatrices();
}

void gCamera::lookAt( const D3DXVECTOR3& target, const D3DXVECTOR3& newCamPosition )
{
	m_pos = newCamPosition;
	lookAt( target );
}

void gCamera::_YPtoQuat()
{
	D3DXQuaternionRotationYawPitchRoll( &m_rot, m_yaw, m_pitch, 0.f);
}

void gCamera::recompMatrices()
{
	D3DXMATRIX rot;
	D3DXVECTOR3 dir(0, 0, 1),at;
	D3DXMatrixRotationQuaternion(&rot, &m_rot);
	D3DXVec3TransformCoord(&dir, &dir, &rot);
	dir += m_pos;
	D3DXMatrixLookAtLH( &m_mview, &m_pos, &dir, &D3DXVECTOR3(0, 1.0f, 0) );

	D3DXMatrixPerspectiveFovLH( &m_mproj, m_FOV, m_aspect, m_nPlane, m_fPlane );

	D3DXMatrixMultiply( &m_mviewproj, &m_mview, &m_mproj );

	//������������� �������� ���������
	m_frustum.updatePlanes();
}


gViewingFrustum::gViewingFrustum(gCamera* cam)
{
	if (!cam)
		throw("Error: null gcamera pointer in frustum");
	m_pCam = cam;
}

bool gViewingFrustum::testPoint(float x, float y, float z)  const
{
	float dot = 0;

	// ����������, ��� ����� ������ ��������
	for( short i = 0; i < 6; i++ ) {
		dot = D3DXPlaneDotCoord(&m_planes[i], &D3DXVECTOR3(x, y, z));
		if( dot < 0.0f )
			return false;
	}
	return true;
}

bool gViewingFrustum::testPoint(const D3DXVECTOR3& point)  const
{
	// ����������, ��� ����� ������ ��������
	for (short i = 0; i < 6; i++) {
		if (D3DXPlaneDotCoord(&m_planes[i], &point) < 0.0f)
			return false;
	}
	return true;
}
bool gViewingFrustum::testAABB(const D3DXVECTOR3& bbMin, const D3DXVECTOR3& bbMax)  const
{
	// ������������ ���������� ����� ������ ��������
	for (short i = 0; i < 6; i++)
	{
		short Count = 8;
		bool PointIn = true;

		// ��������� ��� ������ ����� ������������ ���������
		if (D3DXPlaneDotCoord(&m_planes[i],
			&D3DXVECTOR3( bbMin.x, bbMin.y, bbMin.z )) < 0.0f) {
			PointIn = false;
			Count--;
		}

		if (D3DXPlaneDotCoord(&m_planes[i],
			&D3DXVECTOR3(bbMax.x, bbMin.y, bbMin.z)) < 0.0f) {
			PointIn = false;
			Count--;
		}

		if (D3DXPlaneDotCoord(&m_planes[i],
			&D3DXVECTOR3(bbMin.x, bbMax.y, bbMin.z)) < 0.0f) {
			PointIn = false;
			Count--;
		}

		if (D3DXPlaneDotCoord(&m_planes[i],
			&D3DXVECTOR3(bbMax.x, bbMax.y, bbMin.z)) < 0.0f) {
			PointIn = false;
			Count--;
		}

		// ��������� ��� ������ ����� ������������ ���������
		if (D3DXPlaneDotCoord(&m_planes[i],
			&D3DXVECTOR3(bbMin.x, bbMin.y, bbMax.z)) < 0.0f) {
			PointIn = false;
			Count--;
		}

		// ��������� ��� ������ ����� ������������ ���������
		if (D3DXPlaneDotCoord(&m_planes[i],
			&D3DXVECTOR3(bbMax.x, bbMin.y, bbMax.z)) < 0.0f) {
			PointIn = false;
			Count--;
		}

		// ��������� ��� ������ ����� ������������ ���������
		if (D3DXPlaneDotCoord(&m_planes[i],
			&D3DXVECTOR3(bbMin.x, bbMax.y, bbMax.z)) < 0.0f) {
			PointIn = false;
			Count--;
		}

		// ��������� ��� ������ ����� ������������ ���������
		if (D3DXPlaneDotCoord(&m_planes[i],
			&D3DXVECTOR3(bbMax.x, bbMax.y, bbMax.z)) < 0.0f) {
			PointIn = false;
			Count--;
		}

		// ���� ������ �������� ������ ���, ���������� FALSE
		if (Count == 0)
			return false;
	}
	return true;
}

bool gViewingFrustum::testAABB( const gAABB& bbox ) const
{
	return testAABB( bbox.getMaxBounds(), bbox.getMinBounds() );
}
	
void gViewingFrustum::updatePlanes()
{
	const D3DXMATRIX& Matrix = m_pCam->getViewProjMatrix();
	
	//// ��������� ���������
	m_nearPlane.a = Matrix._14 + Matrix._13;
	m_nearPlane.b = Matrix._24 + Matrix._23;
	m_nearPlane.c = Matrix._34 + Matrix._33;
	m_nearPlane.d = Matrix._44 + Matrix._43;
	D3DXPlaneNormalize(&m_nearPlane, &m_nearPlane);

	m_farPlane.a = Matrix._14 - Matrix._13;
	m_farPlane.b = Matrix._24 - Matrix._23;
	m_farPlane.c = Matrix._34 - Matrix._33;
	m_farPlane.d = Matrix._44 - Matrix._43;
	D3DXPlaneNormalize(&m_farPlane, &m_farPlane);

	m_leftPlane.a = Matrix._14 + Matrix._11;
	m_leftPlane.b = Matrix._24 + Matrix._21;
	m_leftPlane.c = Matrix._34 + Matrix._31;
	m_leftPlane.d = Matrix._44 + Matrix._41;
	D3DXPlaneNormalize(&m_leftPlane, &m_leftPlane);

	m_rightPlane.a = Matrix._14 - Matrix._11;
	m_rightPlane.b = Matrix._24 - Matrix._21;
	m_rightPlane.c = Matrix._34 - Matrix._31;
	m_rightPlane.d = Matrix._44 - Matrix._41;
	D3DXPlaneNormalize(&m_rightPlane, &m_rightPlane);

	m_topPlane.a = Matrix._14 - Matrix._12;
	m_topPlane.b = Matrix._24 - Matrix._22;
	m_topPlane.c = Matrix._34 - Matrix._32;
	m_topPlane.d = Matrix._44 - Matrix._42;
	D3DXPlaneNormalize(&m_topPlane, &m_topPlane);

	m_bottomPlane.a = Matrix._14 + Matrix._12;
	m_bottomPlane.b = Matrix._24 + Matrix._22;
	m_bottomPlane.c = Matrix._34 + Matrix._32;
	m_bottomPlane.d = Matrix._44 + Matrix._42;
	D3DXPlaneNormalize(&m_bottomPlane, &m_bottomPlane);
}

float gCamera::getDistanceToPointF( const D3DXVECTOR3& vec) const
{
	D3DXVECTOR3 vDist = m_pos - vec;
	return D3DXVec3Length(&vDist);
}

unsigned short gCamera::getDistanceToPointUS(const D3DXVECTOR3& vec) const
{
	float dist = getDistanceToPointF(vec);
	if (dist >= m_fPlane)
		return 0xFFFF;
	else
		return unsigned short((dist / m_fPlane) * 0xFFFF);
}
