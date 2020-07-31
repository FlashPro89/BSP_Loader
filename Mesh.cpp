#include "Mesh.h"
#include "Scene.h"
#include "Terrain.h"
#include "gmath.h"
#include "FileSystem.h"
#include "Camera.h"

#define BUFSZ 2048

#define GSKIN_FVF ( D3DFVF_XYZB1 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX1 )
#define GSTATIC_FVF ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )

struct gSkinVertex
{
	float x, y, z;

	DWORD MatrixIndices;

	float nx, ny, nz; 
	float tu, tv; 
};

struct gStaticVertex
{
	float x, y, z;
	float nx, ny, nz;
	float tu, tv;
};

struct gBonePoint
{
	D3DXVECTOR3 v;
	unsigned long color;
};

FILE* fd = 0;

typedef unsigned __int16  gSkinIndex;

//-----------------------------------------------
//
//	CLASS: gSkinBone
//
//-----------------------------------------------

gSkinBone::gSkinBone( ) : m_position( 0.f, 0.f, 0.f )
{
	m_name = "";
	m_parent = 0;
	D3DXQuaternionIdentity( &m_orientation );
}

gSkinBone::~gSkinBone()
{
//	if (m_name)
//		delete[] m_name;
}

void gSkinBone::setParams( int parent, const char* name, const D3DXVECTOR3& position, const D3DXVECTOR3& orientation )
{
	setName( name );
	m_parent = parent;
	m_position = position;
	m_orientation = m_orientation;
}

void gSkinBone::setParentId( int parent )
{
	m_parent = parent;
}
void gSkinBone::setName( const char* name )
{
	/*
	if (m_name)
		delete[] m_name;

	unsigned int l = strlen(name);
	m_name = new char[l + 1];
	m_name[0] = 0;
	strcpy_s( m_name, l+1, name);
	m_name[l] = 0;
	*/

	m_name = name;
}
void gSkinBone::setPosition( const D3DXVECTOR3& position )
{
	m_position = position;
}
void gSkinBone::setOrientation( const D3DXQUATERNION& orientation )
{
	m_orientation = orientation;
}

void gSkinBone::move(const D3DXVECTOR3& v)
{
	m_position += v;
}

void gSkinBone::rotate(const D3DXQUATERNION& q)
{
	D3DXQuaternionMultiply( &m_orientation, &m_orientation, &q ); 
}

int gSkinBone::getParentId() const
{
	return m_parent;
}

const char* gSkinBone::getName() const
{
	return m_name.c_str();
}


const D3DXVECTOR3& gSkinBone::getPosition() const
{
	return m_position;
}

const D3DXQUATERNION& gSkinBone::getOrientation() const
{
	return m_orientation;
}

//-----------------------------------------------
//
//	CLASS: gResourceSkinAnimation
//
//-----------------------------------------------

gResourceSkinAnimation::gResourceSkinAnimation( gResourceManager* mgr, GRESOURCEGROUP group, 
	const char* filename, const char* name, gResourceSkinnedMesh* mesh)
	: gResource( mgr, group, filename, name )
{
	m_resName = name;
	m_fileName = filename;

	m_bonesNum = 0;
	m_framesNum = 0;
	m_fps = SKINNING_FPS_DEFAULT;

	m_frames = 0; //new gSkinBone[ m_bonesNum * m_framesNum ];
	m_skeletonBlockPos = 0;
	m_absMats = 0;
	//m_nullFrame = 0;
	//m_hierarchyTransformedCurrentFrame = 0;
	m_mesh = mesh;
}

gResourceSkinAnimation::~gResourceSkinAnimation()
{
	if (m_frames)
		delete[] m_frames;
}

bool gResourceSkinAnimation::preload()//загрузка статических данных
{
	char buffer[BUFSZ] = "";
	FILE* f = 0;
	errno_t err = fopen_s(&f, m_fileName.c_str(), "rt");

	if ((err != 0) || f == 0)
		return false;

	//find nodes block
	while (fgets(buffer, BUFSZ, f))
	{
		buffer[strlen(buffer) - 1] = 0;
		if (!strcmp("nodes", buffer))
			break;
	}

	//m_bonesNum = 0;
	while (fgets(buffer, BUFSZ, f))
	{
		buffer[strlen(buffer) - 1] = 0;
		if (!strcmp("end", buffer))
			break;
		m_bonesNum++;
	}

	while (fgets(buffer, BUFSZ, f))
	{
		buffer[strlen(buffer) - 1] = 0;
		if (!strcmp("skeleton", buffer))
			break;
	}

	m_skeletonBlockPos = ftell(f);

	// блок skeleton
	int itmp;
	char tstr[256] = "";

	m_framesNum = 0; /// разобраться тут!!! 

	while (fgets(buffer, BUFSZ, f))
	{
		buffer[strlen(buffer) - 1] = 0;
		if (!strcmp("end", buffer))
			break;


		sscanf_s(buffer, "%s %d", &tstr, 256, &itmp);
		if ( !strcmp(tstr, "time") )
			m_framesNum++;
	}

	fclose( f );

	return true;
}

bool gResourceSkinAnimation::load()
{
	unload();
		
	m_absMats = new D3DXMATRIX[ m_bonesNum ];

	char buffer[BUFSZ] = "";
	FILE* f = 0;
	errno_t err = fopen_s(&f, m_fileName.c_str(), "rt");

	if ((err != 0) || f == 0)
		return false;

	//выделим память
	m_frames = new gSkinBone[m_framesNum * m_bonesNum];
	m_nullFrame = new gSkinBone[m_bonesNum];
	//m_hierarchyTransformedCurrentFrame = new gSkinBone[m_bonesNum];

	//find nodes block
	while (fgets(buffer, BUFSZ, f))
	{
		buffer[strlen(buffer) - 1] = 0;
		if (!strcmp("nodes", buffer))
			break;
	}

	unsigned int unodes = 0;
	while (fgets(buffer, BUFSZ, f))
	{
		buffer[strlen(buffer) - 1] = 0;
		if (!strcmp("end", buffer))
			break;

		int itmp = 0, parent = 0;
		char tmpstr[256] = "";
		char fmt[] = "%i \"%[^\"]\" %i";

		sscanf_s(buffer, fmt, &itmp, tmpstr, 255, &parent);
		m_nullFrame[unodes].setName(tmpstr);
		m_nullFrame[unodes].setParentId(parent);
		unodes++;
	}

	//читаем кадры
	fseek(f, m_skeletonBlockPos, SEEK_SET);

	int itmp;
	char tstr[256];
	int ubones = 0;
	while (fgets(buffer, BUFSZ, f))
	{
		int sl = strlen(buffer);
		if (sl > 0)
			buffer[sl - 1] = 0;

		if (!strncmp(buffer, "end", 3))
			break;

		sscanf_s(buffer, "%s %d", &tstr, 256, &itmp);
		if (!strcmp(tstr, "time"))
			continue;

		D3DXVECTOR3 v, r;
		sscanf_s(buffer, "%d %f %f %f %f %f %f", &itmp, &v.x, &v.z, &v.y, &r.x, &r.y, &r.z);

		D3DXMATRIX mrot;
		D3DXMATRIX mrot_x;
		D3DXMATRIX mrot_y;
		D3DXMATRIX mrot_z;

		D3DXMatrixRotationX(&mrot_x, r.x);
		D3DXMatrixRotationY(&mrot_y, r.y);
		D3DXMatrixRotationZ(&mrot_z, r.z);

		D3DXMatrixMultiply(&mrot, &mrot_x, &mrot_y);
		D3DXMatrixMultiply(&mrot, &mrot, &mrot_z);

		D3DXQUATERNION q;
		D3DXQuaternionRotationMatrix(&q, &mrot);
		q = D3DXQUATERNION(-q.x, -q.z, -q.y, q.w); 	// меняем ось Z на ось Y!!!

		m_frames[ubones].setPosition( v );
		m_frames[ubones].setOrientation( q );

		ubones++;
	}

	fclose(f);
	m_isLoaded = true;
	return m_isLoaded;
}

void gResourceSkinAnimation::unload() //данные, загруженые preload() в этой функции не изменяются
{
	if (m_absMats)
		delete[] m_absMats;
	m_absMats = 0;

	if (m_frames)
		delete[] m_frames;
	m_frames = 0;

	if (m_nullFrame)
		delete[] m_nullFrame;
	m_nullFrame = 0;

	//if (m_hierarchyTransformedCurrentFrame)
	//	delete[] m_hierarchyTransformedCurrentFrame;
	//m_hierarchyTransformedCurrentFrame = 0;

	m_isLoaded = false;
}

void gResourceSkinAnimation::setFPS( float fps )
{
	m_fps = fps;
}

float gResourceSkinAnimation::getFPS() const
{
	return m_fps;
}

unsigned char gResourceSkinAnimation::getBonesNum() const
{
	return m_bonesNum;
}

unsigned char gResourceSkinAnimation::getFramesNum() const
{
	return m_framesNum;
}

gSkinBone* gResourceSkinAnimation::getFrame(unsigned char frame)
{
	return &m_frames[frame * m_framesNum];
}

gSkinBone* gResourceSkinAnimation::getBone(unsigned char frame, unsigned char bone)
{
	return &m_frames[frame * m_framesNum + bone];
}

gSkinBone* gResourceSkinAnimation::getFrameInTimePos( float time, gSkinBone* frame )
{
	if (!frame)
		return 0;

	time = time - (int)(time / (m_framesNum - 1)) * (m_framesNum - 1);

	unsigned int currentFrame = (int)time * m_bonesNum;
	if (currentFrame > (m_framesNum - (unsigned int)1)* m_bonesNum)
		currentFrame = (m_framesNum - (unsigned int)1) * m_bonesNum;


	float i1, i2;
	i2 = time - (int)time;
	i1 = 1.0f - i2;

	D3DXMATRIX mRot;
	D3DXMATRIX mTr;
	D3DXMATRIX mAbs;
	D3DXVECTOR3 v, v1, v2;
	D3DXQUATERNION q, q1, q2;

	for (int i = 0; i < m_bonesNum; i++) // интерполятор
	{
		//if (m_nullFrame[i].getParentId() != -1) //TODO: optimize it
		//{
		v1 = m_frames[currentFrame + i].getPosition();
		v2 = m_frames[currentFrame + i + m_bonesNum].getPosition();
		v = v1 * i1 + v2 * i2;

		q1 = m_frames[currentFrame + i].getOrientation();
		q2 = m_frames[currentFrame + i + m_bonesNum].getOrientation();

		D3DXQuaternionSlerp(&q, &q1, &q2, i2); // можно упростить линейной интерполяцией

		frame[i].setPosition(v);
		frame[i].setOrientation(q);
	}
	return frame;
}

/*
D3DXMATRIX* gResourceSkinAnimation::getAbsoluteMatrixes(float time)
{

	time = time - (int)(time / (m_framesNum-1)) * (m_framesNum-1);

	unsigned int currentFrame = (int)time * m_bonesNum;
	if (currentFrame > (m_framesNum- (unsigned int)1)*m_bonesNum)
		currentFrame = (m_framesNum- (unsigned int)1)*m_bonesNum;


	float i1, i2;
	i2 = time - (int)time;
	i1 = 1.0f - i2;

	D3DXMATRIX mRot;
	D3DXMATRIX mTr;
	D3DXMATRIX mAbs;
	D3DXVECTOR3 v, v1, v2;
	D3DXQUATERNION q, q1, q2;

	for (int i = 0; i < m_bonesNum; i++) // интерполятор
	{
		//if (m_nullFrame[i].getParentId() != -1) //TODO: optimize it
		//{
			v1 = m_frames[currentFrame + i].getPosition();
			v2 = m_frames[currentFrame + i + m_bonesNum].getPosition();
			v = v1 * i1 + v2 * i2;

			q1 = m_frames[currentFrame + i].getOrientation();
			q2 = m_frames[currentFrame + i + m_bonesNum].getOrientation();

			D3DXQuaternionSlerp(&q, &q1, &q2, i2);

			m_nullFrame[i].setPosition(v);
			m_nullFrame[i].setOrientation(q);

			m_hierarchyTransformedCurrentFrame[i].setPosition(v);
			m_hierarchyTransformedCurrentFrame[i].setOrientation(q);
		//}
		//else
		//{
			//m_nullFrame[i].setPosition(D3DXVECTOR3(0.f, 0.f, 0.f));
			//D3DXQuaternionIdentity(&q);
			//m_nullFrame[i].setOrientation(q);

			//v = m_nullFrame[i].getPosition();
			//v.x = m_root._41;
			//v.y = m_root._42;
			//v.z = m_root._43;
			//m_nullFrame[i].setPosition();
			//D3DXQuaternionRotationMatrix(&m_transformed[i].m_rotation, &m_root);
		//}
	}

	//TEST: use null frame as interpolated
	//if (m_mesh != 0)
	//{
	//	for (unsigned char i = 0; i < m_bonesNum; i++)
	//	{
	//		m_nullFrame[i] = m_mesh->getNullFrame()[i];
	//	}
	//}
	
	transformToWorldCurrentFrame( m_hierarchyTransformedCurrentFrame, -1 );

	for (unsigned char i = 0; i < m_bonesNum; i++)
	{
		v = m_hierarchyTransformedCurrentFrame[i].getPosition();
		q = m_hierarchyTransformedCurrentFrame[i].getOrientation();

		D3DXMatrixTranslation( &mTr, v.x, v.y, v.z );
		D3DXMatrixRotationQuaternion( &mRot, &q );
		D3DXMatrixMultiply( &m_absMats[i], &mRot, &mTr );
	}

	return m_absMats;
}
*/

gSkinBone* gResourceSkinAnimation::getNullFrame() const
{
	return m_nullFrame;
}

//gSkinBone* gResourceSkinAnimation::getHierarchyTransformedCurrentFrame() const
//{
//	return m_hierarchyTransformedCurrentFrame;
//}

gResourceSkinnedMesh* gResourceSkinAnimation::getSkinnedMesh() const
{
	return m_mesh;
}

void gResourceSkinAnimation::transformToWorldCurrentFrame(gSkinBone* frames, int bone)
{
	/*
	for (int i = 0; i < m_bonesNum; i++)
	{
		unsigned char parentId = frame[i].getParentId();
		if ( parentId == bone)
		{
			if( parentId != -1 )
			{
				D3DXMATRIX mTmp;
				D3DXMATRIX mTr;
				D3DXMATRIX mRot;

				D3DXVECTOR3 v;
				D3DXQUATERNION q;

				v = frame[parentId].getPosition();
				q = frame[parentId].getOrientation();

				D3DXMatrixTranslation( &mTr, v.x, v.y, v.z );
				D3DXMatrixRotationQuaternion( &mRot, &q );
				D3DXMatrixMultiply(&mTmp, &mRot, &mTr);

				// local
				D3DXMATRIX mAbsTr;
				D3DXMATRIX mTmp2;
				D3DXMATRIX mTrChild;
				D3DXMATRIX mRotChild;

				v = frame[i].getPosition();
				q = frame[i].getOrientation();

				D3DXMatrixTranslation( &mTrChild, v.x, v.y, v.z );
				D3DXMatrixRotationQuaternion(&mRotChild, &q);
				D3DXMatrixMultiply(&mTmp2, &mRotChild, &mTrChild);

				D3DXMatrixMultiply(&mAbsTr, &mTmp2, &mTmp);
				frame[i].setPosition( D3DXVECTOR3( mAbsTr._41, mAbsTr._42, mAbsTr._43 ) );

				D3DXQuaternionRotationMatrix( &q, &mAbsTr );
				frame[i].setOrientation( q );
			}
			transformToWorldCurrentFrame( frame, i);
		}
	}
	*/

	//=========================================================================
	//	OPTIMIAZATION: no matrix transforms for skinning
	//=========================================================================

	for (int i = 0; i < m_bonesNum; i++)
	{
		unsigned int parentId = frames[i].getParentId();

		if (parentId == bone)
		{
			if (parentId != -1)
			{
				D3DXVECTOR3 v_parent = frames[parentId].getPosition();
				D3DXQUATERNION q_parent = frames[parentId].getOrientation();
				D3DXVECTOR3 v_child = frames[i].getPosition();
				D3DXQUATERNION q_child = frames[i].getOrientation();

				D3DXQUATERNION q_transformedChild = q_child * q_parent;
				D3DXVECTOR3 v_transformedChild;
				_transformHierarchyVec3( q_parent, v_parent, v_child, v_transformedChild );

				//=========================================================================

				frames[i].setPosition(v_transformedChild);
				frames[i].setOrientation(q_transformedChild);
			}
			transformToWorldCurrentFrame(frames, i);
		}
	}
}

//-----------------------------------------------
//
//	CLASS: gResourceSkinnedMesh
//
//-----------------------------------------------

gResourceSkinnedMesh::gResourceSkinnedMesh(gResourceManager* mgr, GRESOURCEGROUP group,
	const char* filename, const char* name) : gRenderable(mgr, group, filename, name)
{
	m_pVB = 0;
	m_pIB = 0;

	m_vertexesNum = 0;
	m_indexesNum = 0;
	m_bonesNum = 0;
	m_pMaterialsNum = 0;
	m_trisNum = 0;

	//m_pMaterial = 0;

	m_nodes_blockpos = 0;
	m_time0_blockpos = 0;
	m_tris_blockpos = 0;

	m_pBones = 0;
	m_pMatInverted = 0;
	m_pMatPalete = 0;

	m_pTransformedBones = 0;
	//m_pTransformedBonesByQuat = 0;
	_time = 0.f; // TODO: delete

	m_pAtlasTexture = 0;
}

gResourceSkinnedMesh::~gResourceSkinnedMesh()
{
	auto it = m_animMap.begin();
	while (it != m_animMap.end())
	{
		if (it->second)
			m_pResMgr->destroyResource(it->second->getResourceName(), it->second->getGroup());
		it++;
	}
	m_animMap.clear();

	if (m_pMatInverted)
		delete[]  m_pMatInverted;
	m_pMatInverted = 0;

	if (m_pBones)
		delete[] m_pBones;
	m_pBones = 0;

	if (m_pTransformedBones)
		delete[] m_pTransformedBones;
	m_pTransformedBones = 0;

	m_trisCacher.clear(); // ??

	unload();
}

bool gResourceSkinnedMesh::preload() //загрузка статических данных
{
	m_AABB.reset();
	HRESULT hr = S_OK;
	char buffer[BUFSZ];
	FILE* f = 0;
	errno_t err = fopen_s( &f, m_fileName.c_str(), "rt" );
	if ( err !=0 )
		return false;

	//find nodes block
	while ( fgets(buffer, BUFSZ, f) )
	{
		if (!strncmp(buffer, "nodes", 5))
			break;
	}

	m_nodes_blockpos = ftell(f);

	while (fgets(buffer, BUFSZ, f))
	{
		if (!strncmp("end", buffer, 3))
			break;
		m_bonesNum++;
	}

	//----------------------------------------
	//	загрузим данные о костях
	//----------------------------------------

	//загрузим иерархию костей специально для Skinned аниматора

	unsigned int unodes = 0;
	m_pBones = new gSkinBone[m_bonesNum];
	fseek(f, m_nodes_blockpos, SEEK_SET);
	while ((fgets(buffer, BUFSZ, f)) && (unodes < m_bonesNum))
	{
		if (!strncmp(buffer, "end", 3))
			break;

		int sl = strlen(buffer); // ???
		if (sl > 0)
			buffer[sl - 1] = 0;

		int itmp = 0, parent = 0;
		char tmpstr[256] = "";
		char fmt[] = "%i \"%[^\"]\" %i";

		sscanf_s(buffer, fmt, &itmp, tmpstr, 255, &parent);
		m_pBones[unodes].setName(tmpstr);
		m_pBones[unodes].setParentId(parent);
		unodes++;
	}

	//find time 0 block
	while (fgets(buffer, BUFSZ, f))
	{
		if (!strncmp(buffer, "time 0", 6))
		{
			m_time0_blockpos = ftell(f);
			while (fgets(buffer, BUFSZ, f))
			{
				if (!strncmp("end", buffer, 3))
					break;
			}
			break;
		}
	}

	//загружаем позицию и ориентацию костей
	unodes = 0;
	fseek(f, m_time0_blockpos, SEEK_SET);
	while ((fgets(buffer, BUFSZ, f)) && (unodes < m_bonesNum))
	{
		if (unodes > m_bonesNum)
			throw("Invalid skeleton bones num!"); // ??

		if (!strncmp(buffer, "end", 3))
			break;

		int sl = strlen(buffer);
		if (sl > 0)
			buffer[sl - 1] = 0;

		D3DXVECTOR3 pos, rot;
		int itmp;
		sscanf_s(buffer, "%d %f %f %f %f %f %f", &itmp,
			&pos.x, &pos.z, &pos.y, 						// меняем ось Z на ось Y!!!
			&rot.x, &rot.y, &rot.z);

		// TODO: optimize: use func euler_to_quat
		D3DXMATRIX mrot;
		D3DXMATRIX mrot_x;
		D3DXMATRIX mrot_y;
		D3DXMATRIX mrot_z;

		D3DXMatrixRotationX(&mrot_x, rot.x);
		D3DXMatrixRotationY(&mrot_y, rot.y);
		D3DXMatrixRotationZ(&mrot_z, rot.z);

		D3DXMatrixMultiply(&mrot, &mrot_x, &mrot_y);
		D3DXMatrixMultiply(&mrot, &mrot, &mrot_z);

		mrot = mrot_x * mrot_y * mrot_z;

		D3DXQUATERNION q;
		D3DXQuaternionRotationMatrix(&q, &mrot);
		q = D3DXQUATERNION(-q.x, -q.z, -q.y, q.w); 	// меняем ось Z на ось Y!!!

		m_pBones[unodes].setOrientation(q);
		m_pBones[unodes].setPosition(pos);

		unodes++;
	}

	m_pTransformedBones = new gSkinBone[m_bonesNum];
	for (unsigned int i = 0; i < m_bonesNum; i++)
	{
		m_pTransformedBones[i].setName( m_pBones[i].getName() );
		m_pTransformedBones[i].setParentId( m_pBones[i].getParentId() );
		m_pTransformedBones[i].setPosition( m_pBones[i].getPosition() );
		m_pTransformedBones[i].setOrientation( m_pBones[i].getOrientation() );
	}
	_transform_to_world(m_pTransformedBones, -1);

	//приготовим инверсные матрицы начального положения
	m_pMatInverted = new D3DXMATRIX[m_bonesNum];
	for (unsigned int i = 0; i < m_bonesNum; i++)
	{
		D3DXMATRIX mRot;
		D3DXMATRIX mTrans;
		D3DXMATRIX mAbs;

		D3DXVECTOR3 v = m_pTransformedBones[i].getPosition();
		D3DXQUATERNION q = m_pTransformedBones[i].getOrientation();

		D3DXMatrixTranslation(&mTrans, v.x, v.y, v.z);
		D3DXMatrixRotationQuaternion(&mRot, &q);
		D3DXMatrixMultiply(&mAbs, &mRot, &mTrans);
		D3DXMatrixInverse(&m_pMatInverted[i], 0, &mAbs);
	}
	//-----------------------------------------------------------------------

	gTrisGroupCacherIterator cit;
	
	//extract dirName
	char dirName[BUFSZ] = "";
	strcpy_s(dirName, BUFSZ - 1, m_fileName.c_str());
	unsigned int l = strlen(dirName) - 1;

	while ((dirName[l] != '/') && (l > 0)) l--;
	dirName[l + 1] = 0;								//UNSAFE !?!?

	char fullFileName[BUFSZ];

	//find triangles block
	while (fgets(buffer, BUFSZ, f))
	{
		if (!strncmp(buffer, "triangles", 9))
		{
			m_tris_blockpos = ftell(f);

			//count triangles
			while (fgets(buffer, BUFSZ, f))
			{
				if (!strncmp("end", buffer, 3))
					break;

				buffer[strlen(buffer) - 1] = 0; ///////
				cit = m_trisCacher.find(buffer);

				if (cit == m_trisCacher.end())
				{
					gTrisGroup tg;
					tg.textureIndex = m_pMaterialsNum++;
					tg.trisNum = 1; // прибавляем сдесь 1!
					tg.__used_tris = 0;
					tg.__before = m_trisNum;
					tg.trisOffsetInBuff = m_trisNum * 3;
					
					sprintf_s( fullFileName, BUFSZ, "%s%s", dirName, buffer );

					gFileImpl* file = new gFileImpl( fullFileName, false, true );
					tg.bitmap = new gBMPFile(); // TODO: free mem of bitmap after load!!!!
					tg.bitmap->loadFromFile(file); 
					delete file;


					//gResource2DTexture* pTex = (gResource2DTexture*)m_pResMgr->loadTexture2D(fullFileName);
					//tg.pTex = pTex;
					tg.pTex = 0; //вдальнейшем убрать !

					m_trisCacher[buffer] = tg;
				}
				else
				{
					cit->second.trisNum++;
					//cit->second.trisNum++;
				}

				//3 вершины, пока пропускаем их
				//unsigned int tuint; 
				//float x, y, z;

				fgets(buffer, BUFSZ, f);
				//sscanf_s(buffer, "%u %f %f %f", &tuint, &x, &z, &y);
				//m_AABB.addPoint(D3DXVECTOR3(x, y, z));
				
				fgets(buffer, BUFSZ, f);
				//sscanf_s(buffer, "%u %f %f %f", &tuint, &x, &z, &y);
				//m_AABB.addPoint(D3DXVECTOR3(x, y, z));
				
				fgets(buffer, BUFSZ, f);
				//sscanf_s(buffer, "%u %f %f %f", &tuint, &x, &z, &y);
				//m_AABB.addPoint(D3DXVECTOR3(x, y, z));

				m_trisNum++;
			}
			break;
		}
	}
	fclose(f);


	if (m_trisCacher.size() > 1)
	{

		//optimize textures to atlas
		m_atlas.beginAtlas(m_trisCacher.size());

		auto it = m_trisCacher.begin();
		while (it != m_trisCacher.end())
		{
			m_atlas.pushTexture(it->second.bitmap->getWidth(), it->second.bitmap->getHeight(), &it->second);
			it++;
		}
		m_atlas.mergeTexturesToAtlas(4096, 4096, 0);

		char atlasFileName[MAX_PATH];

		memcpy(atlasFileName, m_fileName.c_str(), m_fileName.length() - 4);
		atlasFileName[m_fileName.length() - 4] = 0;

		sprintf_s(atlasFileName, MAX_PATH, "%s%s", atlasFileName, "_atlas.bmp");

		gBMPFile outAtlas;
		outAtlas.createBitMap(m_atlas.getAtlasWidth(), m_atlas.getAtlasHeight());
		gFileImpl* file = new gFileImpl(atlasFileName, true, true);

		unsigned short baseIndex = 0; // ??

		for (unsigned int i = 0; i < m_trisCacher.size(); i++)
		{
			gTrisGroup* tg = (gTrisGroup*)m_atlas.getUserDataBySortedIndex(i);

			tg->remappedX = m_atlas.getTextureRemapedXPosBySortedOrder(i);
			tg->remappedY = m_atlas.getTextureRemapedYPosBySortedOrder(i);
			tg->texWidth = m_atlas.getTextureWidthBySortedOrder(i);
			tg->texHeight = m_atlas.getTextureHeightBySortedOrder(i);

			if (tg->bitmap != 0)
				outAtlas.overlapOther(*(tg->bitmap), tg->remappedX, tg->remappedY);

			//free bitmap memory
			delete tg->bitmap;
			tg->bitmap = 0;
		}

		//outAtlas.verticalFlip();
		outAtlas.saveToFile(file);
		delete file;

		m_pAtlasTexture = (gResource2DTexture*)m_pResMgr->loadTexture2D(atlasFileName);
	}
	else
	{
		m_pAtlasTexture = (gResource2DTexture*)m_pResMgr->loadTexture2D( fullFileName ); //load texture in first trisGroup
	}

	gMaterial* pMaterial = m_pResMgr->getMaterialFactory()->getMaterial( getResourceName() );
	if (!pMaterial)
		pMaterial = m_pResMgr->getMaterialFactory()->createMaterial( getResourceName() );
	pMaterial->setTexture( 0, m_pAtlasTexture );

	m_defaultMatMap[m_resName.c_str()] = pMaterial;

	m_vertexesNum = m_trisNum * 3;

	return true;
}

int compareVData( const void* i, const void* j )
{
	const gSkinVertex* tr0 = (gSkinVertex*)i; //triangle sz = vert size * 3
	const gSkinVertex* tr1 = (gSkinVertex*)j;

	DWORD d0 = tr0[2].MatrixIndices | (tr0[1].MatrixIndices << 8) | (tr0[0].MatrixIndices << 16 );
	DWORD d1 = tr1[2].MatrixIndices | (tr1[1].MatrixIndices << 8) | (tr1[0].MatrixIndices << 16);

	return (int)d0 - (int)d1;
/*
	if (tr0[0].MatrixIndices > tr1[0].MatrixIndices)
	{
		if (tr0[1].MatrixIndices > tr1[1].MatrixIndices)
		{
			if (tr0[2].MatrixIndices > tr1[2].MatrixIndices)
				return 1;
			else
				return -1;
		}
		else return -1;
	}
	else return -1;
*/
}

bool gResourceSkinnedMesh::load()
{
	unload();

	_time = 0.f;

	char buffer[BUFSZ] = "";

	FILE* f = 0;
	errno_t err = fopen_s( &f, m_fileName.c_str(), "rt" );
	if ( (err != 0) || f == 0 )
		return false;

	HRESULT hr1 = S_OK, hr2 = S_OK;
	LPDIRECT3DDEVICE9 pD3DDev9 = m_pResMgr->getDevice();

	//---------------------------------
	// Create and lock DXbuffers
	//---------------------------------

	//create vertex buffer
	hr1 = pD3DDev9->CreateVertexBuffer( sizeof(gSkinVertex) * m_trisNum * 3, 
		D3DUSAGE_WRITEONLY, GSKIN_FVF, D3DPOOL_DEFAULT, &m_pVB, 0 );

	//create index buffer
	hr2 = pD3DDev9->CreateIndexBuffer( m_trisNum * 6, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, 0 );
	
	if (FAILED(hr1) || FAILED(hr2))
	{
		unload();
		if( f )fclose(f);
		return false;
	}

	m_vertexesNum = m_trisNum * 3;
	m_indexesNum = m_trisNum * 6;

	gSkinVertex* p_vData = 0;
	gSkinIndex* p_iData = 0;

	hr1 = m_pVB->Lock( 0, sizeof(gSkinVertex) * m_trisNum * 3, (void**)& p_vData, 0);
	hr2 = m_pIB->Lock( 0, m_trisNum * 6, (void**)& p_iData, 0);

	if (FAILED(hr1) || FAILED(hr2))
	{
		unload();
		if (f) fclose(f);
		return false;
	}

	//--------------------------------------------------------
	// read tris data from SMD
	//--------------------------------------------------------
	
	m_AABB.reset();
	fseek(f, m_tris_blockpos, SEEK_SET);

	/*
	if (m_pAtlasTexture == 0)
	{
		while (fgets(buffer, BUFSZ, f))
		{
			if (!strncmp("end", buffer, 3))
				break;

			buffer[strlen(buffer) - 1] = 0; //////
			auto cit = m_trisCacher.find(buffer);

			// смещение для записи в вершинный буффер для данного треугольника
			int vdi = cit->second.__before * 3 + cit->second.__used_tris * 3;

			if (cit == m_trisCacher.end())
			{
				throw("Тут надо подумать!");
			}
			else
			{	//находим в вершинном буффере данную группу треугольников и добавляем к ней 
				unsigned char indexex[3];

				for (int i = 2; i >= 0; i--) // меняем CW на CCW
				{
					if (!fgets(buffer, BUFSZ, f))
						throw("Ошибка при загрузке данных из файла!");

					int sl = strlen(buffer);
					if (sl > 0)
						buffer[sl - 1] = 0;

					unsigned int tuint;
					sscanf_s(buffer, "%u %f %f %f %f %f %f %f %f",  //FIX IT!!!
						&tuint,
						&p_vData[vdi + i].x,
						&p_vData[vdi + i].z,
						&p_vData[vdi + i].y,
						&p_vData[vdi + i].nx,
						&p_vData[vdi + i].nz,   // меняем оси z и y
						&p_vData[vdi + i].ny,
						&p_vData[vdi + i].tu,
						&p_vData[vdi + i].tv);

					m_AABB.addPoint(D3DXVECTOR3(p_vData[vdi + i].x, p_vData[vdi + i].y, p_vData[vdi + i].z));

					indexex[i] = tuint;

					//p_vData[vdi + i].nx = -p_vData[vdi + i].nx;
					//p_vData[vdi + i].ny = -p_vData[vdi + i].ny;
					//p_vData[vdi + i].nz = -p_vData[vdi + i].nz;

					p_vData[vdi + i].tv = -p_vData[vdi + i].tv;
					p_vData[vdi + i].tu = p_vData[vdi + i].tu;
					p_iData[cit->second.__before * 3 + cit->second.__used_tris * 3 + i] = vdi + i;
				}

				//remap bones matrix pallete indexes
				if (cit->second.remapedSubsets.empty())
				{
					cit->second.remapedSubsets.push_back(gSkinIndexRemapedSubset(vdi));
				}
				//итератор на последний элемент subset'a
				auto rsit = --cit->second.remapedSubsets.end();

				//если ожидается переполнение индексов в subset'e то создаем новый subset
				if ((rsit->remapedIndexes.size() > 8 - 1) &&
					((rsit->remapedIndexes.end() == rsit->remapedIndexes.find(indexex[0])) ||
						(rsit->remapedIndexes.end() == rsit->remapedIndexes.find(indexex[1])) ||
						(rsit->remapedIndexes.end() == rsit->remapedIndexes.find(indexex[2]))))
				{
					cit->second.remapedSubsets.push_back(gSkinIndexRemapedSubset(vdi));
					rsit = --cit->second.remapedSubsets.end();
				}

				rsit->primitivesNum++;

				//проверяем индексы в subset'e на повторяемость
				for (unsigned int i = 0; i < 3; i++)
				{
					auto riit = rsit->remapedIndexes.find(indexex[i]);

					//если такого индекса нет в наборе, то добавляем его
					if (riit == rsit->remapedIndexes.end())
					{
						unsigned int sz = rsit->remapedIndexes.size();
						p_vData[vdi + i].MatrixIndices = sz;
						rsit->remapedIndexes[indexex[i]] = sz;
					}
					else
					{
						p_vData[vdi + i].MatrixIndices = riit->second;
						rsit->remapedIndexes[indexex[i]] = riit->second;
					}

					//TEST:
					p_vData[vdi + i].MatrixIndices = 0;
				}
			}
			cit->second.__used_tris++;
		}
	}
	else //вариант с текстурным атласом
	{

	*/
		float atlasW = m_pAtlasTexture->getTextureWidth();
		float atlasH = m_pAtlasTexture->getTextureHeight();

		float pxU = 1.f / atlasW;
		float pxV = 1.f / atlasH;

		int vdi = 0;

		while (fgets(buffer, BUFSZ, f))
		{
			if (!strncmp("end", buffer, 3))
				break;

			buffer[strlen(buffer) - 1] = 0; //////
			auto cit = m_trisCacher.find(buffer);

			// смещение для записи в вершинный буффер для данного треугольника
			//int vdi = cit->second.__before * 3 + cit->second.__used_tris * 3;

			if (cit == m_trisCacher.end())
			{
				throw("Тут надо подумать!");
			}
			else
			{	//находим в вершинном буффере данную группу треугольников и добавляем к ней 
				unsigned char indexex[3];

				unsigned short w = cit->second.texWidth;
				unsigned short h = cit->second.texHeight;
				unsigned short remappedX = cit->second.remappedX;
				unsigned short remappedY = cit->second.remappedY;

				for (int i = 2; i >= 0; i--) // меняем CW на CCW
				{
					if (!fgets(buffer, BUFSZ, f))
						throw("Ошибка при загрузке данных из файла!");

					int sl = strlen(buffer);
					if (sl > 0)
						buffer[sl - 1] = 0;

					unsigned int tuint;
					sscanf_s(buffer, "%u %f %f %f %f %f %f %f %f",  //FIX IT!!!
						&tuint,
						&p_vData[vdi + i].x,
						&p_vData[vdi + i].z,
						&p_vData[vdi + i].y,
						&p_vData[vdi + i].nx,
						&p_vData[vdi + i].nz,   // меняем оси z и y
						&p_vData[vdi + i].ny,
						&p_vData[vdi + i].tu,
						&p_vData[vdi + i].tv);

					m_AABB.addPoint(D3DXVECTOR3(p_vData[vdi + i].x, p_vData[vdi + i].y, p_vData[vdi + i].z));

					indexex[i] = tuint;

					p_vData[vdi + i].tv = 1 - p_vData[vdi + i].tv;

					p_vData[vdi + i].tu = pxU * remappedX + p_vData[vdi + i].tu * (float)w / atlasW;
					p_vData[vdi + i].tv = pxV * remappedY + p_vData[vdi + i].tv * (float)h / atlasH;

					p_vData[vdi + i].MatrixIndices = tuint;

					p_iData[vdi + i] = vdi + i;
				}

				vdi += 3;
			}
		}
	//}

	qsort( p_vData, m_trisNum, sizeof( gSkinVertex ) * 3, compareVData );
	
	/*
	f = 0;
	err = fopen_s(&f, "out_sorted_vertexes.txt", "wt");
	for( unsigned int i = 0; i<m_trisNum*3; i+=3 )
	{
		fprintf(f, "%u %f %f %f %f %f %f %f %f\n",  //FIX IT!!!
			p_vData[i].MatrixIndices,
			p_vData[i].x,
			p_vData[i].y,
			p_vData[i].z,
			p_vData[i].nx,
			p_vData[i].ny,   // меняем оси z и y
			p_vData[i].nz,
			p_vData[i].tu,
			p_vData[i].tv);

		fprintf(f, "%u %f %f %f %f %f %f %f %f\n",  //FIX IT!!!
			p_vData[i+1].MatrixIndices,
			p_vData[i+1].x,
			p_vData[i+1].y,
			p_vData[i+1].z,
			p_vData[i+1].nx,
			p_vData[i+1].ny,   // меняем оси z и y
			p_vData[i+1].nz,
			p_vData[i+1].tu,
			p_vData[i+1].tv);

		fprintf(f, "%u %f %f %f %f %f %f %f %f\n",  //FIX IT!!!
			p_vData[i + 2].MatrixIndices,
			p_vData[i + 2].x,
			p_vData[i + 2].z,
			p_vData[i + 2].y,
			p_vData[i + 2].nx,
			p_vData[i + 2].ny,   // меняем оси z и y
			p_vData[i + 2].nz,
			p_vData[i + 2].tu,
			p_vData[i + 2].tv);

		fprintf(f,"================================================================\n");
	}
	fclose(f);
	*/

	hr1 = m_pVB->Unlock();
	hr2 = m_pIB->Unlock();
	if (FAILED(hr1) || FAILED(hr2))
	{
		unload();
		if (f) fclose(f);
		return false;
	}

	m_skinBoneGroups.push_back(gSkinBoneGroup());

	auto git = m_skinBoneGroups.begin();

	git->firstTriangle = 0;
	git->TrianglesNum = 0;
	
	const unsigned char maxBonesInGroup = 8;
	//unsigned char usedIndexes = 0;
	unsigned int trisCounter = 0;

	for (unsigned int i = 0; i < m_trisNum * 3; i += 3)
	{
		//проверяем не будет ли переполнения в BonesGroup
		unsigned char bonesToAdd;  bool b[3];
		
testIndexes:
		b[0] = b[1] = b[2] = false;
		bonesToAdd = 0;

		auto it = git->remappedBones.find( (unsigned char)p_vData[i].MatrixIndices );
		if (it == git->remappedBones.end())
		{
			bonesToAdd++; b[0] = true;
		}
		it = git->remappedBones.find( (unsigned char)p_vData[i + 1].MatrixIndices);
		{
			if (it == git->remappedBones.end())
			{
				if (p_vData[i].MatrixIndices != p_vData[i + 1].MatrixIndices)
				{
					bonesToAdd++; b[1] = true;
				}
			}
		}
		it = git->remappedBones.find( (unsigned char)p_vData[i + 2].MatrixIndices);
		{
			if (it == git->remappedBones.end())
			{
				if ((p_vData[i].MatrixIndices != p_vData[i + 2].MatrixIndices) &&
					(p_vData[i + 1].MatrixIndices != p_vData[i + 2].MatrixIndices))
				{
					bonesToAdd++; b[2] = true;
				}
			}
		}

		if (git->remappedBones.size() + bonesToAdd > maxBonesInGroup )
		{
			//создаем новый BonesGroup
			m_skinBoneGroups.push_back(gSkinBoneGroup());
			git = --m_skinBoneGroups.end();
			git->firstTriangle = trisCounter;
			git->TrianglesNum = 0;
			//usedIndexes = 0;
			goto testIndexes;
		}
	
		//индексируем кости для ремапа
		if (b[0])
			git->remappedBones[(unsigned char)p_vData[i].MatrixIndices] = (unsigned char)git->remappedBones.size();

		if (b[1])
			git->remappedBones[(unsigned char)p_vData[i+1].MatrixIndices] = (unsigned char)git->remappedBones.size();
				
		if (b[2])
			git->remappedBones[(unsigned char)p_vData[i+2].MatrixIndices] = (unsigned char)git->remappedBones.size();

		git->TrianglesNum++;

		p_vData[i].MatrixIndices = (git->remappedBones.find((unsigned char)p_vData[i].MatrixIndices))->second;
		p_vData[i+1].MatrixIndices = (git->remappedBones.find((unsigned char)p_vData[i+1].MatrixIndices))->second;
		p_vData[i+2].MatrixIndices = (git->remappedBones.find((unsigned char)p_vData[i+2].MatrixIndices))->second;


		trisCounter++;
	}

	if (f) fclose(f);

	m_isLoaded = true;
	return m_isLoaded;
}

void gResourceSkinnedMesh::unload() //данные, загруженые preload() в этой функции не изменяются
{
	m_AABB.reset();
	
	if (m_pIB)
		m_pIB->Release();
	m_pIB = 0;

	if (m_pVB)
		m_pVB->Release();
	m_pVB = 0;

	m_isLoaded = false;
}

void gResourceSkinnedMesh::onFrameRender( gRenderQueue* queue, const gEntity* entity, const gCamera* cam ) const
{
	LPDIRECT3DDEVICE9 pD3DDev9 = m_pResMgr->getDevice();
	if (!pD3DDev9) 
		return;
	HRESULT hr;

	//DWORD oldLightingState;
	//pD3DDev9->GetRenderState(D3DRS_LIGHTING, &oldLightingState);
	
	//pD3DDev9->SetRenderState(D3DRS_LIGHTING, true);
	//pD3DDev9->SetTransform( D3DTS_WORLD, &matrixes[0] );
	//pD3DDev9->SetFVF( GSKIN_FVF );
	//pD3DDev9->SetStreamSource( 0, m_pVB, 0, sizeof(gSkinVertex) );
	//pD3DDev9->SetIndices( m_pIB );

	//pD3DDev9->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE );
	//pD3DDev9->SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS );

	gSkinnedMeshAnimator* animator = (gSkinnedMeshAnimator*)entity->getAnimator(GANIMATOR_SKINNED);
	const D3DXMATRIX* matrixes = animator->getWordBonesMatrixes();

/*
	auto ait = m_animMap.find("idle1");
	gResourceSkinAnimation* anim = ait->second;

	pD3DDev9->SetFVF(GSKIN_FVF);

	D3DXMATRIX* mAbs = anim->getAbsoluteMatrixes(_time);

	auto it = m_trisCacher.begin();
	while ( it != m_trisCacher.end() )
	{
		if( it->second.pTex )
			pD3DDev9->SetTexture( 0, it->second.pTex->getTexture() );

		if (anim->isLoaded())
		{
			D3DXMATRIX mOut;
			unsigned int pos_in_remap_buffer = 0;

			auto remapSit = it->second.remapedSubsets.begin();

			while ( remapSit != it->second.remapedSubsets.end())
			{
				auto remapIt = remapSit->remapedIndexes.begin();
				while (remapIt != remapSit->remapedIndexes.end())
				{

					D3DXMatrixMultiply(&mOut, &m_pMatInverted[remapIt->first], &mAbs[remapIt->first]);
					D3DXMatrixMultiply(&mOut, &mOut, &transform);
					hr = pD3DDev9->SetTransform(D3DTS_WORLDMATRIX(remapIt->second), &mOut);

					remapIt++;
				}
				
				pD3DDev9->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0,
					m_trisNum * 3, remapSit->indexBufferOffset, remapSit->primitivesNum );

				remapSit++;
			}
		}


		//pD3DDev9->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0,
		//	m_trisNum * 3, it->second.trisOffsetInBuff, it->second.trisNum );
	
		it++;
	}
*/

	D3DXMATRIX mId;
	D3DXMatrixIdentity(&mId);
	if (m_pAtlasTexture)
	{
		//pD3DDev9->SetTexture(0, m_pAtlasTexture->getTexture());

		auto it = m_skinBoneGroups.begin();
		while (it != m_skinBoneGroups.end())
		{
			auto rit = it->remappedBones.begin();
		
			while (rit != it->remappedBones.end())
			{
				//if( !matrixes )
				//	pD3DDev9->SetTransform(D3DTS_WORLDMATRIX(rit->second), &mId);
				//else
				//	pD3DDev9->SetTransform(D3DTS_WORLDMATRIX(rit->second), &matrixes[rit->first]);
				rit++;
			}

			gMaterial* entMat = entity->getMaterial(0);
			if (entMat == 0)
				entMat = m_defaultMatMap.begin()->second;

			unsigned short distance = cam->getDistanceToPointUS( D3DXVECTOR3( matrixes[0]._41, matrixes[0]._42, matrixes[0]._43) );
			gRenderElement re( this, entMat, distance, m_bonesNum, matrixes, it->firstTriangle * 3, it->TrianglesNum, m_vertexesNum, &(*it) );
			queue->pushBack(re);

			//позже удалить
			//pD3DDev9->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_vertexesNum, 
			//	it->firstTriangle*3, it->TrianglesNum );

			it++;
		}
	}
	else
	{
		auto it = m_trisCacher.begin();
		while (it != m_trisCacher.end())
		{
			//if (it->second.pTex)
			//pD3DDev9->SetTexture(0, it->second.pTex->getTexture());
			
			//pD3DDev9->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_vertexesNum, 
			//	it->second.trisOffsetInBuff, it->second.trisNum );
			it++;
		}
	}

	//pD3DDev9->SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE );
	//pD3DDev9->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, false );
	 
	//if (m_pTransformedBones)
	//	drawSkeleton(m_pTransformedBones, &matrixes[0], 0, 0xFF00FFFF );
	
	//pD3DDev9->SetRenderState(D3DRS_LIGHTING, oldLightingState);
}

bool gResourceSkinnedMesh::addAnimation(const char* filename, const char* name)
{
	auto it = m_animMap.find( filename );
	if( it != m_animMap.end() )
		return false;

	char animResName[256];
	sprintf_s( animResName, "%s.%s", m_resName.c_str(), name );
	
	gResourceSkinAnimation* anim = (gResourceSkinAnimation*)m_pResMgr->loadSkinnedAnimationSMD( filename, animResName, this );
	if ( !anim )
	{
		return false;
	}

	anim->addRef();
	m_animMap[name] = anim;
	return true;
}

gResourceSkinAnimation* gResourceSkinnedMesh::getAnimation(const char* name) const
{
	auto it = m_animMap.find( name );
	if (it != m_animMap.end())
	{
		return it->second;
	}
	else
		return (gResourceSkinAnimation*)0;
}

const D3DXMATRIX* gResourceSkinnedMesh::getInvertedMatrixes() const
{
	return m_pMatInverted;
}

//set before frame render by render queue
void gResourceSkinnedMesh::setMatrixPalete( const D3DXMATRIX* palete )
{
	m_pMatPalete = palete;
}

const D3DXMATRIX* gResourceSkinnedMesh::getMatrixPalete() const
{
	return m_pMatPalete;
}

gSkinBone* gResourceSkinnedMesh::getNullFrame() const
{
	return m_pBones;
}

unsigned int gResourceSkinnedMesh::getBonesNum() const
{
	return m_bonesNum;
}

GVERTEXFORMAT gResourceSkinnedMesh::getVertexFormat() const
{
	return GVF_SKINNED0WEIGHTS;
}

const char* gResourceSkinnedMesh::getDefaultMaterialName()
{
	return m_resName.c_str();
}

void* gResourceSkinnedMesh::getVBuffer() const
{
	return m_pVB;
}

void* gResourceSkinnedMesh::getIBuffer() const
{
	return m_pIB;
}

unsigned int gResourceSkinnedMesh::getIBufferSize() const
{
	return m_trisNum * 6;
}

unsigned int gResourceSkinnedMesh::getVBufferSize() const
{
	return sizeof(gSkinVertex) * m_trisNum * 3;
}

GPRIMITIVETYPE gResourceSkinnedMesh::getPrimitiveType() const
{
	return GPRIMITIVETYPE::GPT_TRIANGLELIST;
}
unsigned int gResourceSkinnedMesh::getVertexStride() const
{
	return sizeof(gSkinVertex);
}


bool gResourceSkinnedMesh::isUseUserMemoryPointer()
{
	return false;
}

void gResourceSkinnedMesh::_transform_to_world( gSkinBone* frames, int bone )
{
	for (unsigned int i = 0; i < m_bonesNum; i++)
	{
		unsigned int parentId = frames[i].getParentId();

		if (parentId == bone)
		{
			if ( parentId != -1 )
			{
				/*
				D3DXMATRIX m_parent;
				D3DXMATRIX m_tr;
				D3DXMATRIX m_rot;

				D3DXVECTOR3 v = frames[parentId].getPosition();
				D3DXQUATERNION q = frames[parentId].getOrientation();

				D3DXMatrixTranslation(&m_tr, v.x, v.y, v.z);
				D3DXMatrixRotationQuaternion(&m_rot, &q);
				D3DXMatrixMultiply(&m_parent, &m_rot, &m_tr);

				// local
				D3DXMATRIX abs_tr;
				D3DXMATRIX m_child;
				D3DXMATRIX m_tr_child;
				D3DXMATRIX m_rot_child;

				v = frames[i].getPosition();
				q = frames[i].getOrientation();

				D3DXMatrixTranslation(&m_tr_child, v.x, v.y, v.z);
				//D3DXMatrixRotationQuaternion(&m_rot_child, &q);
				//D3DXMatrixMultiply(&m_child, &m_rot_child, &m_tr_child);

				D3DXMatrixMultiply(&abs_tr, &m_tr_child, &m_parent);

				//v.x = m_parent._41 + v.x; v.y = m_parent._42 + v.y; v.z = m_parent._43 + v.z;
				v.x = abs_tr._41; v.y = abs_tr._42; v.z = abs_tr._43;
				
				*/

				//=========================================================================
				//OPTIMIAZATION: no matrix transforms for skinning
				//=========================================================================
				D3DXVECTOR3 v_parent = frames[parentId].getPosition();
				D3DXQUATERNION q_parent = frames[parentId].getOrientation();
				D3DXVECTOR3 v_child = frames[i].getPosition();
				D3DXQUATERNION q_child = frames[i].getOrientation();

				//Rotate vec
				D3DXQUATERNION q_transformedChild = q_child * q_parent;
				D3DXVECTOR3 v_transformedChild;
				D3DXQuaternionNormalize(&q_parent, &q_parent);
				
				_transformHierarchyVec3( q_parent, v_parent, v_child, v_transformedChild );

				//=========================================================================

/*
				//DEBUG
				fprintf_s(fd, "----------------------------------------------------------------------------\n");
				fprintf_s(fd, "bone %i (parent %i\n", i, parentId);
				fprintf_s(fd, "v_parent xyzw: %f %f %f\n", v_parent.x, v_parent.y, v_parent.z);
				fprintf_s(fd, "v_child  xyzw: %f %f %f\n", v_child.x, v_child.y, v_child.z);
				fprintf_s(fd, "parent quat xyzw: %f %f %f %f\n", q_parent.x, q_parent.y, q_parent.z, q_parent.w);

				fprintf_s(fd, "by matrix: %f %f %f\n", v.x, v.y, v.z);
				fprintf_s(fd, "by	quat: %f %f %f\n", v_transformedChild.x, v_transformedChild.y, v_transformedChild.z);
*/
				frames[i].setPosition(v_transformedChild);
				//D3DXQuaternionRotationMatrix(&q, &abs_tr);
				frames[i].setOrientation(q_transformedChild);

				//m_pTransformedBonesByQuat[i].setPosition(v_transformedChild);
				//m_pTransformedBonesByQuat[i].setOrientation(q_transformedChild);
			}
			_transform_to_world(frames, i);
		}
	}

}

void gResourceSkinnedMesh::drawSkeleton(const gSkinBone* frame, const D3DXMATRIX* mWorld, 
	int b1, DWORD color) const
{
	LPDIRECT3DDEVICE9 pD3DDev9 = m_pResMgr->getDevice();
	if (!pD3DDev9)
		return;

	DWORD oldLightingState;
	pD3DDev9->GetRenderState(D3DRS_LIGHTING, &oldLightingState);
	pD3DDev9->SetRenderState(D3DRS_LIGHTING, false);
	pD3DDev9->SetTransform(D3DTS_WORLD, mWorld);
	pD3DDev9->SetRenderState(D3DRS_ZENABLE, false);

	DWORD lastFVF;
	pD3DDev9->GetFVF(&lastFVF);
	pD3DDev9->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pD3DDev9->SetTexture(0, 0);

	_skeleton(frame, b1, color);

	pD3DDev9->SetFVF(lastFVF);
	pD3DDev9->SetRenderState(D3DRS_ZENABLE, true);
	pD3DDev9->SetRenderState(D3DRS_LIGHTING, oldLightingState);
}

void gResourceSkinnedMesh::_skeleton( const gSkinBone* frame, int b1, DWORD color ) const
{
	if (frame == 0)
		return;

	LPDIRECT3DDEVICE9 pD3DDev9 = m_pResMgr->getDevice();
	if (!pD3DDev9)
		return;

	int parent;

	for( unsigned int i = 0; i < m_bonesNum; i++ )
	{
		parent = frame[i].getParentId();

		if ( parent == b1)
		{	
			gBonePoint p[2];

			p[0].v = frame[i].getPosition();
			p[1].v = frame[parent].getPosition();
			p[0].color = p[1].color = color;

			pD3DDev9->DrawPrimitiveUP(D3DPT_LINELIST, 1, (void*)&p, sizeof(gBonePoint) );
			_skeleton( frame, i, color );
		}
	}
}

//-----------------------------------------------
//
//	CLASS: gResourceStaticMesh
//
//-----------------------------------------------

gResourceStaticMesh::gResourceStaticMesh( gResourceManager* mgr, GRESOURCEGROUP group, 
	const char* filename, const char* name): gRenderable(mgr, group, filename, name)
{
	m_pVB = 0;
	m_pIB = 0;

	m_vertexesNum = 0;
	m_indexesNum = 0;
	m_pMaterialsNum = 0;
	m_trisNum = 0;

	m_tris_blockpos = 0;

	//m_pMaterial = 0;

	//debug
	m_normals = 0;
}

gResourceStaticMesh::~gResourceStaticMesh()
{
	//освобождение материала в деструкторе gRenderable
	/*
	auto it = m_trisCacher.begin();
	while (it != m_trisCacher.end())
	{
		if (it->second.pMat)
			it->second.pMat->release();
		it++;
	}
	*/
	unload();
}

bool gResourceStaticMesh::preload()
{
	m_AABB.reset();
	HRESULT hr = S_OK;
	char buffer[BUFSZ];
	FILE* f = 0;
	errno_t err = fopen_s(&f, m_fileName.c_str(), "rt");
	if (err != 0)
		return false;

	gTrisGroupCacherIterator cit;

	//extract dirName
	char dirName[BUFSZ] = "";
	strcpy_s(dirName, BUFSZ - 1, m_fileName.c_str());
	unsigned int l = strlen(dirName) - 1;

	while ((dirName[l] != '/') && (l > 0)) l--;
	dirName[l + 1] = 0;								//UNSAFE !?!?

	char fullFileName[BUFSZ];
	char matName[BUFSZ];

	//find triangles block
	while (fgets(buffer, BUFSZ, f))
	{
		if (!strncmp(buffer, "triangles", 9))
		{
			m_tris_blockpos = ftell(f);

			//count triangles
			while (fgets(buffer, BUFSZ, f))
			{
				if (!strncmp("end", buffer, 3))
					break;

				buffer[strlen(buffer) - 1] = 0; ///////
				cit = m_trisCacher.find(buffer);

				if (cit == m_trisCacher.end())
				{
					gTrisGroup tg;
					tg.textureIndex = m_pMaterialsNum++;
					tg.trisNum = 1; // прибавляем сдесь 1!
					tg.__used_tris = 0;
					tg.__before = m_trisNum;
					tg.trisOffsetInBuff = m_trisNum * 3;

					sprintf_s( fullFileName, BUFSZ, "%s%s", dirName, buffer );
					sprintf_s( matName, BUFSZ, "%s.%s", m_resName.c_str(), buffer);
					matName[strlen(matName) - 4] = 0;

					gMaterial* pMaterial = m_pResMgr->getMaterialFactory()->getMaterial(matName);
					if (!pMaterial)
					{
						gResource2DTexture* pTex = (gResource2DTexture*)m_pResMgr->getResource(fullFileName, GRESGROUP_2DTEXTURE);
						if (!pTex)
							pTex = (gResource2DTexture*)m_pResMgr->loadTexture2D(fullFileName);

						pMaterial = m_pResMgr->getMaterialFactory()->createMaterial(matName);
						pMaterial->setTexture(0, pTex);
					}
					else
						pMaterial->addRef();

					m_defaultMatMap[matName] = pMaterial;

					tg.pMat = pMaterial;
					tg.pTex = pMaterial->getTexture(0);

					m_trisCacher[buffer] = tg;
				}
				else
					cit->second.trisNum++;

				//3 вершины, пока пропускаем их
				fgets(buffer, BUFSZ, f);
				fgets(buffer, BUFSZ, f);
				fgets(buffer, BUFSZ, f);
				m_trisNum++;
			}
			break;
		}
	}
	fclose(f);

	m_vertexesNum = m_trisNum * 3;

	return true;
}
bool gResourceStaticMesh::load() //загрузка видеоданных POOL_DEFAULT
{
	unload();

	char buffer[BUFSZ] = "";
	
	FILE* f = 0;
	errno_t err = fopen_s(&f, m_fileName.c_str(), "rt");
	if ((err != 0) || f == 0)
		return false;

	HRESULT hr1 = S_OK, hr2 = S_OK;
	LPDIRECT3DDEVICE9 pD3DDev9 = m_pResMgr->getDevice();

	//---------------------------------
	// Create and lock DXbuffers
	//---------------------------------

	//create vertex buffer
	hr1 = pD3DDev9->CreateVertexBuffer(sizeof(gStaticVertex) * m_trisNum * 3,
		D3DUSAGE_WRITEONLY, GSTATIC_FVF, D3DPOOL_DEFAULT, &m_pVB, 0);

	//create index buffer
	hr2 = pD3DDev9->CreateIndexBuffer(m_trisNum * 6, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, 0);

	if (FAILED(hr1) || FAILED(hr2))
	{
		unload();
		if (f)fclose(f);
		return false;
	}

	gStaticVertex* p_vData = 0;
	gSkinIndex* p_iData = 0;

	hr1 = m_pVB->Lock(0, sizeof(gStaticVertex) * m_trisNum * 3, (void**)& p_vData, 0);
	hr2 = m_pIB->Lock(0, m_trisNum * 6, (void**)& p_iData, 0);

	if (FAILED(hr1) || FAILED(hr2))
	{
		unload();
		if (f) fclose(f);
		return false;
	}

	//--------------------------------------------------------
	// read tris data from SMD
	//--------------------------------------------------------

	//debug
	//m_normals = new gDebugNormal[m_trisNum * 3 * 2];

	m_AABB.reset();

	fseek(f, m_tris_blockpos, SEEK_SET);
	while (fgets(buffer, BUFSZ, f))
	{
		if (!strncmp("end", buffer, 3))
			break;

		buffer[strlen(buffer) - 1] = 0; //////
		auto cit = m_trisCacher.find(buffer);

		// смещение для записи в вершинный буффер для данного треугольника
		int vdi = cit->second.__before * 3 + cit->second.__used_tris * 3;

		if (cit == m_trisCacher.end())
		{
			throw("Тут надо подумать!");
		}
		else
		{	//находим в вершинном буффере данную группу треугольников и добавляем к ней 
			for (int i = 2; i >= 0; i--) // меняем CW на CCW
			{
				if (!fgets(buffer, BUFSZ, f))
					throw("Ошибка при загрузке данных из файла!");

				int sl = strlen(buffer);
				if (sl > 0)
					buffer[sl - 1] = 0;

				unsigned int tuint;
				sscanf_s(buffer, "%u %f %f %f %f %f %f %f %f",  //FIX IT!!!
					&tuint,
					&p_vData[vdi + i].x,
					&p_vData[vdi + i].z,
					&p_vData[vdi + i].y,
					&p_vData[vdi + i].nx,
					&p_vData[vdi + i].nz,   // меняем оси z и y
					&p_vData[vdi + i].ny,
					&p_vData[vdi + i].tu,
					&p_vData[vdi + i].tv);

				m_AABB.addPoint(D3DXVECTOR3(p_vData[vdi + i].x, p_vData[vdi + i].y, p_vData[vdi + i].z));

				//p_vData[vdi + i].nx = -p_vData[vdi + i].nx;
				//p_vData[vdi + i].ny = -p_vData[vdi + i].ny;
				//p_vData[vdi + i].nz = -p_vData[vdi + i].nz;

				/*
				//debug
				m_normals[(vdi + i) * 2].x = p_vData[vdi + i].x;
				m_normals[(vdi + i) * 2].y = p_vData[vdi + i].y;
				m_normals[(vdi + i) * 2].z = p_vData[vdi + i].z;
				m_normals[(vdi + i) * 2].color = 0xFF00FF00;

				m_normals[(vdi + i) * 2 + 1].x = p_vData[vdi + i].x + p_vData[vdi + i].nx * 10.f;
				m_normals[(vdi + i) * 2 + 1].y = p_vData[vdi + i].y + p_vData[vdi + i].ny * 10.f;
				m_normals[(vdi + i) * 2 + 1].z = p_vData[vdi + i].z + p_vData[vdi + i].nz * 10.f;
				m_normals[(vdi + i) * 2+1].color = 0xFFFF0000;
				*/

				p_vData[vdi + i].tv = -p_vData[vdi + i].tv;
				p_vData[vdi + i].tu = p_vData[vdi + i].tu;
				p_iData[cit->second.__before * 3 + cit->second.__used_tris * 3 + i] = vdi + i;
			}
		}
		cit->second.__used_tris++;
	}

	hr1 = m_pVB->Unlock();
	hr2 = m_pIB->Unlock();
	if (FAILED(hr1) || FAILED(hr2))
	{
		unload();
		if (f) fclose(f);
		return false;
	}

	m_isLoaded = true;
	return m_isLoaded;
}

void gResourceStaticMesh::unload()
{
	m_AABB.reset();

	if (m_pIB)
		m_pIB->Release();
	m_pIB = 0;

	if (m_pVB)
		m_pVB->Release();
	m_pVB = 0;

	if (m_normals)
		delete[] m_normals;
	m_normals = 0;

	m_isLoaded = false;
}

void gResourceStaticMesh::drawNormals() const
{
	if (!m_normals)
		return;

	LPDIRECT3DDEVICE9 pD3DDev9 = m_pResMgr->getDevice();
	if (!pD3DDev9)
		return;

	DWORD lastFVF;
	pD3DDev9->GetFVF(&lastFVF);
	pD3DDev9->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pD3DDev9->SetTexture(0, 0);
	//pD3DDev9->SetRenderState(D3DRS_ZENABLE, false);

	DWORD oldLightingState;
	pD3DDev9->GetRenderState(D3DRS_LIGHTING, &oldLightingState);
	pD3DDev9->SetRenderState(D3DRS_LIGHTING, false);

	pD3DDev9->DrawPrimitiveUP(D3DPT_LINELIST, m_trisNum * 3, (void*)m_normals, sizeof(gDebugNormal));

	pD3DDev9->SetRenderState(D3DRS_LIGHTING, oldLightingState);
	pD3DDev9->SetFVF(lastFVF);
	//pD3DDev9->SetRenderState(D3DRS_ZENABLE, true);
}

GVERTEXFORMAT gResourceStaticMesh::getVertexFormat() const
{
	return GVF_STATICMESH;
}

void* gResourceStaticMesh::getVBuffer() const
{
	return m_pVB;
}

void* gResourceStaticMesh::getIBuffer() const
{
	return m_pIB;
}

unsigned int gResourceStaticMesh::getIBufferSize() const
{
	return m_trisNum * 6;
}

unsigned int gResourceStaticMesh::getVBufferSize() const
{
	return sizeof(gSkinVertex) * m_trisNum * 3;
}

GPRIMITIVETYPE gResourceStaticMesh::getPrimitiveType() const
{
	return GPRIMITIVETYPE::GPT_TRIANGLELIST;
}
unsigned int gResourceStaticMesh::getVertexStride() const
{
	return sizeof(gStaticVertex);
}

bool gResourceStaticMesh::isUseUserMemoryPointer()
{
	return false;
}

void gResourceStaticMesh::onFrameRender(gRenderQueue* queue, const gEntity* entity, const gCamera* cam) const
{
	LPDIRECT3DDEVICE9 pD3DDev9 = m_pResMgr->getDevice();
	if (!pD3DDev9)
		return;

//	pD3DDev9->SetTransform( D3DTS_WORLD, &entity->getHoldingNode()->getAbsoluteMatrix() );
//	pD3DDev9->SetFVF(GSTATIC_FVF);
//	pD3DDev9->SetStreamSource(0, m_pVB, 0, sizeof(gStaticVertex));
//	pD3DDev9->SetIndices(m_pIB);

	const D3DXMATRIX& matrix = entity->getHoldingNode()->getAbsoluteMatrix();
	unsigned short distance = cam->getDistanceToPointUS(D3DXVECTOR3(matrix._41, matrix._42, matrix._43));

//	pD3DDev9->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	//pD3DDev9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
//	pD3DDev9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
//	pD3DDev9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
//	pD3DDev9->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
//	pD3DDev9->SetRenderState(D3DRS_BLENDFACTOR, 0xFF5F5F5F);

//	pD3DDev9->SetRenderState(D3DRS_ZWRITEENABLE, false);

	auto it = m_trisCacher.begin();
	while (it != m_trisCacher.end())
	{
		gMaterial* entMat = entity->getMaterialByName(it->second.pMat->getName());
		if (entMat == 0)
			entMat = ( m_defaultMatMap.find( it->second.pMat->getName() ) )->second; //very unsafe??

		gRenderElement re( this, entMat, distance, 1, &matrix, it->second.trisOffsetInBuff * 3, it->second.trisNum, m_vertexesNum );
		queue->pushBack(re);

		if (it->second.pTex)
			//pD3DDev9->SetTexture(0, it->second.pTex->getTexture());
		
		//pD3DDev9->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0,
		//	m_trisNum * 3, it->second.trisOffsetInBuff, it->second.trisNum );

		it++;
	}
	//pD3DDev9->SetRenderState(D3DRS_ZWRITEENABLE, true);
	//pD3DDev9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	//drawNormals();
}
