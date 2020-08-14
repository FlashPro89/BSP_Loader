#include "util.h"
#include "input.h"
#include "timer.h"
#include "BSPFile.h"
#include "Camera.h"
#include "Terrain.h"
#include "Resources.h"
#include "Scene.h"
#include "FileSystem.h"
#include "TextureAtlas.h"
#include "BMPFile.h"
#include "RenderQueue.h"
#include <cstdio>
#include <map>
#include <string>
#include <stdlib.h>
#include <stdio.h>

LPDIRECT3DVERTEXBUFFER9 m_VB = 0;
LPDIRECT3DINDEXBUFFER9 m_IB = 0;
LPDIRECT3DTEXTURE9 pTexLightsAtlas = 0;

gTextureAtlas atlas;

bool bFullscreen = false;

//всякое всячино:
gInput* input = 0;
gTimer* timer = 0;

bool wf = false;
bool rbboxes = false;
bool useLightmaps = true;
bool useFrustum = true;
bool rmodels = true;
int drawedFaces = 0;
int drawedLeafs = 0;
int numSetTexCalls = 0;

int num_verts = 0;
int num_edges = 0;
int num_surfedges = 0;
int num_faces = 0;
int num_texinfs = 0;
int num_planes = 0;
int num_miptexs = 0;
int num_models = 0;
int num_nodes = 0;
int num_leafs = 0;
int num_marksurfaces = 0;
int num_clipnodes = 0;

BSPVertex_t* bsp_verts = 0;
BSPEdge_t* bsp_edges = 0;
int* bsp_surfedges = 0;
BSPFace_t* bsp_faces = 0;
BSPTexinfo_t* bsp_texinfs = 0;
BSPPlane_t* bsp_planes = 0;
BSPModel_t* bsp_models = 0;
BSPNode_t* bsp_nodes = 0;
BSPLeaf_t* bsp_leafs = 0;
unsigned short* bsp_marksurfaces = 0;
BSPClipnode_t* bsp_clipnodes = 0;

byte* bsp_lightdata = 0;
byte* bsp_texdata = 0;
byte* bsp_visdata = 0;
int bsp_texdatasize = 0;
int bsp_lightdatasize = 0;
int bsp_visdatasize = 0;

gFileSystem				fSys;
gMaterialFactory        matFactory;
gCamera					cam;
gResourceManager		rmgr( &pD3DDev9, &matFactory, &fSys );
gSceneManager			smgr( &rmgr, &matFactory );
gRenderQueue			rqueue;

int currentFace = 0;
int currentLeaf = 0;
int currentNode = 0;
float ti = 0;

int r_num_i = 0;
int r_num_v = 0;

float offset_lmap_x = 0.5f;
float offset_lmap_y = 0.5f;
float scale = 1.0f;

std::map< unsigned int, std::string> filesMapList;
unsigned int currentMap = 0;

struct RFace
{
	int start_indx;
	int num_prim;
	int miptex;
	bool isDrawed;
};

RFace rfaces[64000];

int leafsCounter = 0;
int nodesCounter = 0;
int facesCounter = 0;
int modelsCounter = 0;

//for PVS decompression
int visLeafs = 0;
int visrow = 0;

int decompressRow(byte* visCompr, byte* dest);
int compressRow(byte* vis, byte* dest);
inline bool isLeafVisible(byte* decomprPVS, int leafBit);

void drawLeaf(int leaf);

#define MAX_TEXS_NUM 512

gResource* tmap[MAX_TEXS_NUM]; // texures
unsigned int numLightedFaces = 0; // ??

typedef struct
{
	float x, y, z;
	float nx, ny, nz;
	float tu, tv;
	float tu2, tv2;
	//	DWORD color;
}D3DVertex;

typedef unsigned __int16  D3DIndex;


bool isLeafInFrustum(int leaf)
{
	D3DXVECTOR3 bmin(bsp_leafs[leaf].mins[0], bsp_leafs[leaf].mins[2], bsp_leafs[leaf].mins[1]);
	D3DXVECTOR3 bmax(bsp_leafs[leaf].maxs[0], bsp_leafs[leaf].maxs[2], bsp_leafs[leaf].maxs[1]);

	return 	cam.getViewingFrustum().testAABB(gAABB(bmin, bmax));
}

bool isNodeInFrustum(int node)
{
	D3DXVECTOR3 bmin(bsp_nodes[node].mins[0], bsp_nodes[node].mins[2], bsp_nodes[node].mins[1]);
	D3DXVECTOR3 bmax(bsp_nodes[node].maxs[0], bsp_nodes[node].maxs[2], bsp_nodes[node].maxs[1]);

	return 	cam.getViewingFrustum().testAABB(gAABB(bmin, bmax));
}

bool isModelInFrustum(int model)
{
	D3DXVECTOR3 bmin(bsp_models[model].mins[0], bsp_models[model].mins[2], bsp_models[model].mins[1]);
	D3DXVECTOR3 bmax(bsp_models[model].maxs[0], bsp_models[model].maxs[2], bsp_models[model].maxs[1]);

	return 	cam.getViewingFrustum().testAABB(gAABB(bmin, bmax));
}

void unLoadWAD()
{
	for (int i = 0; i < MAX_TEXS_NUM; i++)
	{
		if ( tmap[i] )
		{
			//rmgr.destroyResource(tmap[i]->getResourceName(), tmap[i]->getGroup());
			tmap[i]->release();
			tmap[i] = 0;
		}
	}
}

void loadWAD()
{
	printf_s("Loading Textures...\n");

	int counter = 0;
	gResource* res = 0;

	//загружаем тестуры из WAD используемые на карте
	for (int i = 0; i < num_miptexs; i++)
	{
		//find index in wad
		int idx = 0;
		int* offs = (int*)(bsp_texdata + sizeof(int));

		BSPMiptex_t* miptex = (BSPMiptex_t*)(bsp_texdata + offs[i]);
		std::string d = miptex->name;
		toUpper(miptex->name);

		res = rmgr.loadTexture2DFromWADList(miptex->name);
		if (res != 0)
			counter++;
		else
			printf("warning: texture %s not found\n", miptex->name);
		tmap[i] = res;
	}
	printf( "%i of %i textures loaded from WAD files\n", counter, num_miptexs );
}

void unLoadLightmaps()
{
	if (pTexLightsAtlas)
		pTexLightsAtlas->Release();
	pTexLightsAtlas = 0;
}


void createLightmapsAtlas(unsigned int width, unsigned int height, unsigned char border)
{
	LPDIRECT3DTEXTURE9 pTexTmp = 0;
	// TODO: проверить устройство на поддержку размера текстуры не соотв 2 в степени N
	HRESULT hr = pD3DDev9->CreateTexture(width, height, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pTexLightsAtlas, 0);
	if (FAILED(hr))
		throw("Cannot create lightmap atlas texture in sysmem!");

	hr = pD3DDev9->CreateTexture(width, height, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pTexTmp, 0);
	if (FAILED(hr))
		throw("Cannot create lightmap atlas texture in vidmem!");

	RECT rc;
	D3DLOCKED_RECT dlr;

	rc.left = 0; rc.top = 0; rc.bottom = height - 1; rc.right = width - 1;
	hr = pTexLightsAtlas->LockRect(0, &dlr, &rc, D3DLOCK_DISCARD);

	if (FAILED(hr))
		throw("Cannot lock rect of lightmap atlas texture!");

	memset(dlr.pBits, 0xFF, dlr.Pitch * height);

	unsigned int w = 0, h = 0, remappedX = 0, remappedY = 0;

	//debug
	//FILE* f = 0;
	//errno_t err = fopen_s(&f, "out_ep_bsp_order.txt", "wt");

	for (unsigned int i = 0; i < numLightedFaces; i++)
	{
		w = atlas.getTextureWidthBySortedOrder(i);
		h = atlas.getTextureHeightBySortedOrder(i);
		remappedX = atlas.getTextureRemapedXPosBySortedOrder(i);
		remappedY = atlas.getTextureRemapedYPosBySortedOrder(i);

		if (w * h == 0)
			continue;

		//load lightmap to DX texture
		icolor* ptr_dx = 0;
		iwadcolor* ptr_ld = (iwadcolor*)(bsp_lightdata + bsp_faces[atlas.getTextureBaseIndexInSortedOrder(i)].lightofs);

		//fprintf(f, "atlas ind: %i   face index: %i   lightofs: %i\n", i, atlas.getTextureBaseIndexInSortedOrder(i), bsp_faces[atlas.getTextureBaseIndexInSortedOrder(i)].lightofs);


		for (unsigned int y = remappedY; y < h + remappedY; y++)
		{
			for (unsigned int x = remappedX; x < w + remappedX; x++)
			{
				ptr_dx = (icolor*)(((byte*)dlr.pBits) + dlr.Pitch * y + x * 4);

				ptr_dx->a = 0xFF;
				ptr_dx->r = ptr_ld->r;
				ptr_dx->g = ptr_ld->g;
				ptr_dx->b = ptr_ld->b;
				ptr_ld++;
			}
		}
	}
	//fclose(f);

	hr = pTexLightsAtlas->UnlockRect(0);
	if (FAILED(hr))
		throw("Cannot unlock rect of lightmap atlas texture!");

	//DEBUG LMAP:
	D3DXSaveTextureToFile("atlas_fill_lights.bmp", D3DXIFF_BMP, pTexLightsAtlas, 0);
	hr = pD3DDev9->UpdateTexture(pTexLightsAtlas, pTexTmp);

	pTexLightsAtlas->Release();
	pTexLightsAtlas = pTexTmp;

}

void loadLighmaps()
{
	printf_s( "Loading Lightmaps...\n" );

	float mins[2], maxs[2];
	int texsize[2];
	float val; int e;
	BSPTexinfo_t* tex;
	BSPVertex_t* vert;
	BSPFace_t* s;

	numLightedFaces = 0;
	atlas.beginAtlas( num_faces );

	for (int i = 0; i < num_faces; i++)
	{
		// compute lightmap texture size
		mins[0] = 99999.f; mins[1] = 99999.f;
		maxs[0] = -99999.f; maxs[1] = -99999.f;

		tex = &bsp_texinfs[bsp_faces[i].texinfo];
		s = &bsp_faces[i];

		if (s->styles[0] == 0)
		{
			numLightedFaces++;
			texsize[0] = 0; texsize[1] = 0;

			for (int j = s->firstedge; j < s->numedges + s->firstedge; j++)
			{
				e = bsp_surfedges[j];
				if (e >= 0)
					vert = bsp_verts + bsp_edges[e].v[0];
				else
					vert = bsp_verts + bsp_edges[-e].v[1];

				for (int k = 0; k < 2; k++)
				{
					val = vert->point[0] * tex->vecs[k][0] +
						vert->point[1] * tex->vecs[k][1] +
						vert->point[2] * tex->vecs[k][2] + tex->vecs[k][3];

					if (val < mins[k])
						mins[k] = val;
					if (val > maxs[k])
						maxs[k] = val;
				}
			}

			for (int l = 0; l < 2; l++)
			{
				mins[l] = (float)floor(mins[l] / 16);
				maxs[l] = (float)ceil(maxs[l] / 16);

				texsize[l] = (int)(maxs[l] - mins[l] + 1);
				if (texsize[l] > 17) //17+1
					throw("Bad surface extents");
			}

			atlas.pushTexture(texsize[0], texsize[1]);
		}
		else
		{
			atlas.pushTexture(0, 0);
		}
	}

	atlas.mergeTexturesToAtlas( 4096, 4096 );
	createLightmapsAtlas( atlas.getAtlasWidth(), atlas.getAtlasHeight(), 0 );
}

#define ONPLANE		0
#define FRONT		1
#define BACK		2
#define EPSILON		0.01f

// returned dist point % plane
inline float testPointOnPlane(const D3DXVECTOR3& point, int plane)
{
	return (point.x * bsp_planes[plane].normal[0] +
		point.y * bsp_planes[plane].normal[2] +
		point.z * bsp_planes[plane].normal[1] - bsp_planes[plane].dist);
}

//returned clipnode content
int testClipnode(const D3DXVECTOR3& point, int clipnode )
{
	if (clipnode < 0)
	{
		return clipnode;
	}
	else
	{
		if (testPointOnPlane(point, bsp_clipnodes[clipnode].planenum) > EPSILON)
			return testClipnode(point, bsp_clipnodes[clipnode].children[0]);
		else
			return testClipnode(point, bsp_clipnodes[clipnode].children[1]);
	}
}

//returned clipnode content
int testClipnode(const D3DXVECTOR3& point, int clipnode, float radius)
{
	if (clipnode < 0)
	{
		return clipnode;
	}
	else
	{
		if (testPointOnPlane(point, bsp_clipnodes[clipnode].planenum) > radius)
			return testClipnode(point, bsp_clipnodes[clipnode].children[0]);
		else
			return testClipnode(point, bsp_clipnodes[clipnode].children[1]);
	}
}

int isCollideWithWorld( const D3DXVECTOR3& point )
{
	return testClipnode( point, 0 );
}

int isCollideWithWorld( const D3DXVECTOR3& point, float radius )
{
	return testClipnode(point, 0, radius);
}

int getLeafInCamPosition( int* steps = 0 )
{
	int node = 0;
	do
	{
		if (testPointOnPlane(cam.getPosition(), bsp_nodes[node].planenum) >= 0)
			node = bsp_nodes[node].children[0];
		else
			node = bsp_nodes[node].children[1];

		if (steps != 0) (*steps)++;

	} while (node > 0);

	return  -(node + 1);
}

int decompressRow( byte* visCompr, byte* dest );
int compressRow( byte* vis, byte* dest );
inline bool isLeafVisible( byte* decomprPVS, int leafBit );
void drawLeaf(int leaf);

void unLoadScene();

void testFileSystem()
{
	gFile* f = 0;
	/*
	gFileSystem sys;
	char path[MAX_PATH];
	sys.OpenFileDialogBox( path, MAX_PATH, "BMP files(*.bmp)\0*.bmp\0", 23, "Открыть файл:", "hollow.bmp" );

	gBMPFile bmp;
	f = new gFileImpl(path, false, true);
	bmp.loadFromFile(f);
	delete f;

	gBMPFile tmpBuffer;
	f = new gFileImpl("out.bmp", true, true);
	tmpBuffer.createBitMap(256, 256);
	tmpBuffer.overlapOther(bmp, 0, 0);
	tmpBuffer.overlapOther(bmp, bmp.getWidth(), bmp.getHeight());
	tmpBuffer.overlapOther(bmp, bmp.getWidth()*2, bmp.getHeight()*2);
	tmpBuffer.overlapOther(bmp, bmp.getWidth()*3, bmp.getHeight()*3);
	tmpBuffer.overlapOther(bmp, bmp.getWidth() * 4, bmp.getHeight() * 4);

	tmpBuffer.saveToFile(f);
	delete f;

	*/

	f = new gFileImpl( "test_fsystem.txt", true );

	f->puts( "--- TEST LINE 1 ---\n" );
	f->puts( "--- TEST LINE 2 ---\n" );
	f->puts( "--- TEST LINE 3 ---\n" );

	delete f;

	byte buffer[0x200];
	FILE* fd = 0;
	
	fopen_s(&fd, "test_fsystem.txt", "rb");
	fseek(fd, 0, SEEK_END);
	unsigned fsz = ftell(fd);
	fseek( fd, 0, SEEK_SET );
	fread( buffer, fsz, 1, fd );
	fclose(fd);

	// load file from memory
	f = new gFileImpl( 0, true, false, buffer, 0x200);
	char tmp[256] = "";
	while (f->gets(tmp, 256))
	{
		int i = 0;
		i++;
	}

	int _int = 0, r_int = 0;
	float _float = 0, r_float = 0;
	char _str[128] = "", r_str[128] = "";

	f->seek( GFS_SET, 0 );
	f->printf("test printf: %i %f %s\n", 10, 15.f, "str1");
	f->printf("test printf: %i %f %s\n", 35, 1.1f, "str2");
	f->seek(GFS_SET, 0);
	f->scanf("test printf: %i %f %s\n", &_int, &_float, _str, 128);
	f->scanf("test printf: %i %f %s\n", &r_int, &r_float, r_str, 128);

	delete f;
}

void loadFonts()
{
	gResource* res = rmgr.createTextDrawer("font1", gFontParameters(10, 16, 5, false, "Arial") );
	res = rmgr.createTextDrawer("font2", gFontParameters(14, 20, 7, false, "Colibri"));
}

void loadScene( const char* mapname )
{
	unLoadScene();

	rqueue.initialize(0xFFFF);

	currentFace = 0;

	printf( "===================================================================\n" );
	printf( "Loading map: %s\n", mapname );
	printf( "===================================================================\n");

	loadFonts();
	//testFileSystem();
	rmgr.setWADFolder( "../data/wad/" );

	char fname[1024] = "";
	//sprintf_s(fname, 1024, "../data/maps/%s", mapname);
	sprintf_s(fname, 1024, "../data/maps/%s", mapname);


	char tmp[1024];
	sprintf_s( tmp, 1024, "Map: %s", mapname);
	wnd_setTitle(tmp);

	printf_s("Loading Scene...\n");

	input = new gInput(hwnd);
	input->init();
	timer = new gTimer();
	cam.setInput(input);
	//cam.setRelativePosition(D3DXVECTOR3(0, 0, -3000));
	cam.lookAt( D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, -3000) );
	smgr.setActiveCamera(&cam);

	
		//////////////////////////////////////////////////////////////////
		//// scene node graph test
		gSceneNode* node_centr = smgr.getRootNode().createChild("new_central");
		gSceneNode* node_joint1 = node_centr->createChild("new_joint1");
		gSceneNode* node_joint2 = node_joint1->createChild("new_joint2");
		gSceneNode* node_joint3 = node_joint2->createChild("new_joint3");
		gSceneNode* node_joint4 = node_joint3->createChild("new_joint4");
		gSceneNode* node_joint5 = node_joint4->createChild("new_joint5");
		gSceneNode* node_joint51 = node_joint5->createChild("new_joint51");
		gSceneNode* node_joint52 = node_joint5->createChild("new_joint52");
		gSceneNode* node_joint53 = node_joint5->createChild("new_joint53");
		gSceneNode* node_skin1 = node_centr->createChild("new_skin1");
		gSceneNode* node_skin2 = node_centr->createChild("new_skin2");
		gSceneNode* node_skin3 = node_centr->createChild("new_skin3");
		gSceneNode* node_crystal = node_centr->createChild("new_crystal");
		gSceneNode* node_terrain = smgr.getRootNode().createChild("new_terrain");

		/*
		gResource2DTexture* t = (gResource2DTexture*)rmgr.loadTexture2D( "../data/textures/XCRATE5B.BMP", "box" );
		gMaterial* mat = matFactory.createMaterial("default");
		if (mat)
		{
			mat->setTexture(0, t);
		}
		*/


		gResourceStaticMesh* pStaticMesh = (gResourceStaticMesh*)rmgr.loadStaticMeshSMD("../data/models/crystal/crystal_reference.smd", "crystal");

		//gResourceShape* shape = (gResourceShape*)rmgr.createShape("box_1", GSHAPE_BOX);
		//shape->setSizes(40, 40, 40, 10, 10);

		gRenderable* shape = pStaticMesh;

		shape->getDefaultMaterialByIndex(0)->setTransparency(0xC0);

		gEntity* ent = smgr.createEntity("box__root");
		ent->setRenderable(shape);
		//ent->setMaterial(mat);

		smgr.getRootNode().attachEntity(ent);

		ent = smgr.createEntity("box__center");
		ent->setRenderable(shape);
		node_centr->attachEntity( ent );

		ent = smgr.createEntity("box__1");
		ent->setRenderable(shape);
		node_joint1->attachEntity(ent);

		ent = smgr.createEntity("box__2");
		ent->setRenderable(shape);
		node_joint2->attachEntity(ent);

		ent = smgr.createEntity("box__3");
		ent->setRenderable(shape);
		node_joint3->attachEntity(ent);

		ent = smgr.createEntity("box__4");
		ent->setRenderable(shape);
		node_joint4->attachEntity(ent);

		ent = smgr.createEntity("box__5");
		ent->setRenderable(shape);
		node_joint5->attachEntity(ent);

		ent = smgr.createEntity("box__51");
		ent->setRenderable(shape);
		node_joint51->attachEntity(ent);

		ent = smgr.createEntity("box__52");
		ent->setRenderable(shape);
		node_joint52->attachEntity(ent);

		ent = smgr.createEntity("box__53");
		ent->setRenderable(shape);
		node_joint53->attachEntity(ent);

		D3DXQUATERNION q;
		D3DXQuaternionRotationAxis( &q, &D3DXVECTOR3(0, 1.f, 0), D3DX_PI / 16.f );
		node_centr->setRelativePosition( D3DXVECTOR3(0, 0, 0.f) );
		node_joint1->setRelativePosition( D3DXVECTOR3(100.f, 0.f, 0) );
		node_joint2->setRelativePosition( D3DXVECTOR3(100.f, 0.f, 0) );
		node_joint3->setRelativePosition( D3DXVECTOR3(100.f, 0.f, 0) );
		node_joint4->setRelativePosition( D3DXVECTOR3(100.f, 0.f, 0) );
		node_joint5->setRelativePosition(D3DXVECTOR3(100.f, 0.f, 0));
		node_joint51->setRelativePosition(D3DXVECTOR3(50.f, -50.f, 0));
		node_joint52->setRelativePosition(D3DXVECTOR3(50.f, 0.f, 0));
		node_joint53->setRelativePosition(D3DXVECTOR3(50.f, 50.f, 0));
		node_skin1->setRelativePosition(D3DXVECTOR3(50.f, 0.f, 0.f));
		node_skin2->setRelativePosition(D3DXVECTOR3(-50.f, 0.f, 0.f));
		node_skin3->setRelativePosition(D3DXVECTOR3(-80.f, 0.f, 0.f));
		node_crystal->setRelativePosition(D3DXVECTOR3(0.f, 100.f, 0));
		node_crystal->setRelativeScale(D3DXVECTOR3(0.25f, 0.25f, 0.25f));
		node_terrain->setRelativePosition(D3DXVECTOR3(0.f, -500.f, 0.f));
		////////////////////////////////////////////////////////////////////

		//BSPLevel renderable
		gEntity* pEnt = smgr.createEntity("ent_world");
		gRenderable* pBSPLevel = (gRenderable * )rmgr.loadBSPLevel(fname, "bspLevel");
		pEnt->setRenderable(pBSPLevel);
		smgr.getRootNode().attachEntity(pEnt);

		//Load skinned mesh
		gResourceSkinnedMesh* pSMesh = (gResourceSkinnedMesh*)rmgr.loadSkinnedMeshSMD("../data/models/barney/BARNEY-X_Template_Biped1.smd", "barney");
		pSMesh->addAnimation("../data/models/barney/idle1.smd", "idle1" );
		pSMesh->addAnimation("../data/models/barney/idle2.smd", "idle2");
		pSMesh->addAnimation("../data/models/barney/idle3.smd", "idle3");
		pSMesh->addAnimation("../data/models/barney/idle4.smd", "idle4");
		pSMesh->addAnimation("../data/models/barney/run.smd", "run" );
		pSMesh->addAnimation("../data/models/barney/walk.smd", "walk" );
		pSMesh->addAnimation("../data/models/barney/sit1.smd", "sit1"); 
		pSMesh->addAnimation("../data/models/barney/fall_loop.smd", "fall_loop");
		
		ent = smgr.createEntity("ent__skinning1");
		ent->setRenderable(pSMesh);
		//ent->setMaterial( matFactory.getMaterial(pSMesh->getDefaultMaterialName() ) );
		node_skin1->attachEntity( ent );

		gSkinnedMeshAnimator* ctrl = (gSkinnedMeshAnimator * )ent->getAnimator(GANIMATOR_SKINNED);
		ctrl->addTrack("run",GSKINANIM_LOOP)->play();
		//ctrl->getTrack("run")->setFPS(30.f);

		gResourceSkinnedMesh* pSMesh2 =
			(gResourceSkinnedMesh*)rmgr.loadSkinnedMeshSMD("../data/models/zombie/Zom3_Template_Biped(White_Suit)1.smd", "zombie");
		pSMesh2->addAnimation( "../data/models/zombie/idle1.smd", "idle1" ); 
		pSMesh2->addAnimation("../data/models/zombie/eatbody.smd", "eatbody"); 

		ent = smgr.createEntity("ent__skinning2");
		ent->setRenderable(pSMesh2);
		gMaterial* opaqMat = pSMesh2->getDefaultMaterialByIndex(0)->cloneMaterial("zom_opaq");
		opaqMat->setTransparency(0x40);
		ent->setMaterial(opaqMat);
		node_skin2->attachEntity(ent);
		opaqMat->release(); //free mat pointer

		ctrl = (gSkinnedMeshAnimator*)ent->getAnimator(GANIMATOR_SKINNED);
		ctrl->addTrack("idle1", GSKINANIM_LOOP)->play();

		ent = smgr.createEntity("ent__skinning3");
		ent->setRenderable(pSMesh2);
		//ent->setMaterial(matFactory.getMaterial(pSMesh2->getDefaultMaterialName()));
		node_skin3->attachEntity(ent);
		ctrl = (gSkinnedMeshAnimator*)ent->getAnimator(GANIMATOR_SKINNED);
		ctrl->addTrack("eatbody", GSKINANIM_LOOP)->play();
		ctrl->getTrack("eatbody")->setFPS(40.f);
		
		///////////////////////////////////////////////////////////////////
		// Static mesh test
		ent = smgr.createEntity("ent__crystal1");
		ent->setRenderable(pStaticMesh);
		node_crystal->attachEntity(ent);
		////////////////////////////////////////////////////////////////////
		//// Terrain
		ent = smgr.createEntity("ent__terrain");
		gResourceTerrain* pTerr = (gResourceTerrain*)rmgr.loadTerrain( "../data/terrain/terrain1.ter", "Terrain1" );
		//pTerr->setSizes( 32, 32, -100, 600, 10.f, 10.f );
		ent->setRenderable(pTerr);
		node_terrain->attachEntity(ent);
		////////////////////////////////////////////////////////////////////

	//skybox
	ent = smgr.createEntity("ent_skybox");
	gRenderable* pSkyBoxRenderable = (gRenderable*)rmgr.loadSkyBox( "../data/env/skybox.dds", "skybox" );
	ent->setRenderable( pSkyBoxRenderable );
	smgr.getRootNode().attachEntity(ent);


	FILE* f = 0;

	errno_t err = fopen_s( &f, fname, "rb" );
	if ( !f || err ) throw( "BSP File Opening Error" );

	int fl = filelength(f);
	BSPMapHeader_t* bsp_header = (BSPMapHeader_t*)( new byte[fl + 1] ); //? +1?
	( (byte*)bsp_header )[ fl ] = 0;
	//memset( bsp_header, 0, fl );

	if ( fread_s(bsp_header, fl, 1, fl, f) != fl )
		throw("Cannot read BSP file!");

	fclose(f);
	f = 0;

	/*
	if (bsp_header->version != BSPVERSION) throw("Invalid BSP File Version");

	//swap bytes ?!?!? or no?

	//load verts
	num_verts = BSPGetLumpItemsNum(bsp_header, LUMP_VERTEXES);
	bsp_verts = new BSPVertex_t[num_verts];
	BSPCopyLump(bsp_header, LUMP_VERTEXES, bsp_verts, sizeof(BSPVertex_t));

	//load eges
	num_edges = BSPGetLumpItemsNum(bsp_header, LUMP_EDGES);
	bsp_edges = new BSPEdge_t[num_edges];
	BSPCopyLump(bsp_header, LUMP_EDGES, bsp_edges, sizeof(BSPEdge_t));

	//load surfedges
	num_surfedges = BSPGetLumpItemsNum(bsp_header, LUMP_SURFEDGES);
	bsp_surfedges = new int[num_surfedges];
	BSPCopyLump(bsp_header, LUMP_SURFEDGES, bsp_surfedges, sizeof(int));

	//load faces
	num_faces = BSPGetLumpItemsNum(bsp_header, LUMP_FACES);
	bsp_faces = new BSPFace_t[num_faces];
	BSPCopyLump(bsp_header, LUMP_FACES, bsp_faces, sizeof(BSPFace_t));

	//load texinfos
	num_texinfs = BSPGetLumpItemsNum(bsp_header, LUMP_TEXINFO);
	bsp_texinfs = new BSPTexinfo_t[num_texinfs];
	BSPCopyLump(bsp_header, LUMP_TEXINFO, bsp_texinfs, sizeof(BSPTexinfo_t));

	//load planes
	num_planes = BSPGetLumpItemsNum(bsp_header, LUMP_PLANES);
	bsp_planes = new BSPPlane_t[num_planes];
	BSPCopyLump(bsp_header, LUMP_PLANES, bsp_planes, sizeof(BSPPlane_t));

	///load models
	num_models = BSPGetLumpItemsNum(bsp_header, LUMP_MODELS);
	bsp_models = new BSPModel_t[num_models];
	BSPCopyLump(bsp_header, LUMP_MODELS, bsp_models, sizeof(BSPModel_t));

	//load leafs
	num_leafs = BSPGetLumpItemsNum(bsp_header, LUMP_LEAFS);
	bsp_leafs = new BSPLeaf_t[num_leafs];
	BSPCopyLump(bsp_header, LUMP_LEAFS, bsp_leafs, sizeof(BSPLeaf_t));

	//load nodes
	num_nodes = BSPGetLumpItemsNum(bsp_header, LUMP_NODES);
	bsp_nodes = new BSPNode_t[num_nodes];
	BSPCopyLump(bsp_header, LUMP_NODES, bsp_nodes, sizeof(BSPNode_t));

	//load marksurfaces
	num_marksurfaces = BSPGetLumpItemsNum(bsp_header, LUMP_MARKSURFACES);
	bsp_marksurfaces = new unsigned short[num_marksurfaces];
	BSPCopyLump(bsp_header, LUMP_MARKSURFACES, bsp_marksurfaces, sizeof(unsigned short));

	//load clipnodes
	num_clipnodes = BSPGetLumpItemsNum(bsp_header, LUMP_CLIPNODES);
	bsp_clipnodes = new BSPClipnode_t[num_clipnodes];
	BSPCopyLump(bsp_header, LUMP_CLIPNODES, bsp_clipnodes, sizeof(BSPClipnode_t));

	//load miptexs
	bsp_texdatasize = bsp_header->lumps[LUMP_TEXTURES].filelen;
	bsp_texdata = new byte[bsp_texdatasize];
	BSPCopyLump(bsp_header, LUMP_TEXTURES, bsp_texdata, bsp_header->lumps[LUMP_TEXTURES].filelen);

	//load visdata
	bsp_visdatasize = bsp_header->lumps[LUMP_VISIBILITY].filelen;
	if (bsp_visdatasize > 0)
	{
		bsp_visdata = new byte[bsp_visdatasize];
		BSPCopyLump(bsp_header, LUMP_VISIBILITY, bsp_visdata, bsp_visdatasize);
	}

	//load lightdata
	bsp_lightdatasize = bsp_header->lumps[LUMP_LIGHTING].filelen;
	bsp_lightdata = new byte[bsp_lightdatasize];
	BSPCopyLump(bsp_header, LUMP_LIGHTING, bsp_lightdata, bsp_lightdatasize);

	num_miptexs = bsp_texdatasize ? ((BSPMiptexlump_t*)bsp_texdata)->nummiptex : 0;

	//count leafs with PVS 
	visLeafs = 0;
	for (int i = 0 ; i < num_leafs; i++)
	{
		if (bsp_leafs[i].visofs >= 0)
			visLeafs++;
	}
	visrow = (visLeafs + 7) >> 3;

	//count triangles for vertex buffer
	int tri_num = 0;
	int vert_num = 0;
	unsigned short tmp[1024];

	for (int i = 0; i < num_faces; i++)
	{
		//собираем неповторяющиеся индексы вершин по гряням
		memset(tmp, 0xFFFF, sizeof(short) * 1024);

		int vert_in_face = 0;

		int last_edge = bsp_faces[i].firstedge + bsp_faces[i].numedges;
		for (int j = bsp_faces[i].firstedge; j < last_edge; j++)
		{
			bool isPresent = false;
			for (int k = 0; k < bsp_faces[i].numedges; k++)
			{
				if (tmp[k] == bsp_edges[abs(bsp_surfedges[j])].v[0])
				{
					isPresent = true;
				}
			}
			if (!isPresent)
			{
				tmp[vert_in_face] = bsp_edges[abs(bsp_surfedges[j])].v[0];
				vert_in_face++;
			}
			isPresent = false;

			for (int k = 0; k < bsp_faces[i].numedges; k++)
			{
				if (tmp[k] == bsp_edges[abs(bsp_surfedges[j])].v[1])
				{
					isPresent = true;
				}
			}
			if (!isPresent)
			{
				tmp[vert_in_face] = bsp_edges[abs(bsp_surfedges[j])].v[1];
				vert_in_face++;
			}
		}
			
		tri_num += vert_in_face-2;
		vert_num += vert_in_face;
	}

	loadLighmaps();
	//unsigned int usedLightedFaces = 0;

	/////////////////////////////////////
	// create DX buffers
	/////////////////////////////////////	

	//create vertex buffer
	HRESULT hr = pD3DDev9->CreateVertexBuffer( sizeof(D3DVertex) * vert_num, D3DUSAGE_WRITEONLY, D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2, D3DPOOL_MANAGED, &m_VB, 0 );
	if (FAILED(hr))
		throw("Ошибка при создании буффера вершин!");

	//create index buffer
	hr = pD3DDev9->CreateIndexBuffer( sizeof(short) * tri_num * 3, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_IB, 0);
	if (FAILED(hr))
		throw("Ошибка при создании буффера индексов!");

	D3DVertex* p_vdata = 0;
	D3DIndex* p_idata = 0;

	hr = m_VB->Lock(0, sizeof(D3DVertex) * vert_num, (void**)&p_vdata, 0);
	if (FAILED(hr))
		throw("Ошибка при блокировке вершинного буффера!");

	hr = m_IB->Lock( 0, sizeof(short) * tri_num * 3, (void**)& p_idata, 0);
	if (FAILED(hr))
		throw("Ошибка при блокировке индексного буффера!");
	
	int n = 0;
	int idx = 0;
	int first_idx = 0;
	int pos_in_vbuffer = 0;
	int pos_in_ibuffer = 0;
	float nx, ny, nz;

	fopen_s(&f, "../data/out_buf.txt", "w"); //test out

	ZeroMemory(rfaces, sizeof(RFace) * 64000);

	D3DSURFACE_DESC desc;
	pTexLightsAtlas->GetLevelDesc(0, &desc);
	unsigned int atlasW = desc.Width;
	unsigned int atlasH = desc.Height;

	unsigned int w = 0, h = 0, remappedX = 0, remappedY = 0;

	for ( int i = 0; i < num_faces; i++ )
	{
		//расчитаем размеры лайтмапа для фэйса
		float mins[2], maxs[2], tex_center[2], tex_bounds[2];
		int texsize[2];
		float val; int e;
		BSPTexinfo_t* tex;
		BSPVertex_t* vert;
		BSPFace_t* s;

		// compute lightmap texture size
		mins[0] = 99999.f; mins[1] = 99999.f;
		maxs[0] = -99999.f; maxs[1] = -99999.f;

		//if (i == 138)
		//	int g = 1221 + 1;

		tex = &bsp_texinfs[bsp_faces[i].texinfo];
		s = &bsp_faces[i];

		texsize[0] = 0; texsize[1] = 0;

		if (s->styles[0] == 0)
		{
			for (int j = s->firstedge; j < s->numedges + s->firstedge; j++)
			{
				e = bsp_surfedges[j];
				if (e >= 0)
					vert = bsp_verts + bsp_edges[e].v[0];
				else
					vert = bsp_verts + bsp_edges[-e].v[1];

				for (int k = 0; k < 2; k++)
				{
					val = vert->point[0] * tex->vecs[k][0] +
						vert->point[2] * tex->vecs[k][2] +
						vert->point[1] * tex->vecs[k][1] + tex->vecs[k][3];

					if (val < mins[k])
						mins[k] = val;
					if (val > maxs[k])
						maxs[k] = val;
				}
			}

			for (int l = 0; l < 2; l++)
			{
				//exmins[l] = mins[l];
				//exmaxs[l] = maxs[l];

				mins[l] = (float)floor(mins[l] / 16);
				maxs[l] = (float)ceil(maxs[l] / 16);

				texsize[l] = (int)(maxs[l] - mins[l] + 1);
				if (texsize[l] > 18) //17+1
					throw("Bad surface extents");
			}
		}

		nx = bsp_planes[bsp_faces[i].planenum].normal[0];
		ny = bsp_planes[bsp_faces[i].planenum].normal[2];  ///swap normal z<->y
		nz = bsp_planes[bsp_faces[i].planenum].normal[1];
		
		if (bsp_faces[i].side != 0)
		{
			nx = -nx;
			ny = -ny;
			nz = -nz;
		}

	
		////////////////////////////////////////////////
		//индексируем вершины на фейсе
		////////////////////////////////////////////////

		//собираем неповторяющиеся индексы вершин по гряням
		memset(tmp, 0xFFFF, sizeof(short) * 1024);
		
		int vert_in_face = 0;

		int last_edge = bsp_faces[i].firstedge + bsp_faces[i].numedges;
		for (int j = bsp_faces[i].firstedge; j < last_edge; j++)
		{
			if (bsp_surfedges[j] > 0)
			{
				bool isPresent = false;
				for (int k = 0; k < bsp_faces[i].numedges; k++)
				{
					if (tmp[k] == bsp_edges[abs(bsp_surfedges[j])].v[0])
					{
						isPresent = true;
					}
				}
				if (!isPresent)
				{
					tmp[vert_in_face] = bsp_edges[abs(bsp_surfedges[j])].v[0];
					vert_in_face++;
				}
				isPresent = false;

				for (int k = 0; k < bsp_faces[i].numedges; k++)
				{
					if (tmp[k] == bsp_edges[abs(bsp_surfedges[j])].v[1])
					{
						isPresent = true;
					}
				}
				if (!isPresent)
				{
					tmp[vert_in_face] = bsp_edges[abs(bsp_surfedges[j])].v[1];
					vert_in_face++;
				}
			}
			else
			{
				bool isPresent = false;
				for (int k = 0; k < bsp_faces[i].numedges; k++)
				{
					if (tmp[k] == bsp_edges[abs(bsp_surfedges[j])].v[1])
					{
						isPresent = true;
					}
				}
				if (!isPresent)
				{
					tmp[vert_in_face] = bsp_edges[abs(bsp_surfedges[j])].v[1];
					vert_in_face++;
				}
				isPresent = false;

				for (int k = 0; k < bsp_faces[i].numedges; k++)
				{
					if (tmp[k] == bsp_edges[abs(bsp_surfedges[j])].v[0])
					{
						isPresent = true;
					}
				}
				if (!isPresent)
				{
					tmp[vert_in_face] = bsp_edges[abs(bsp_surfedges[j])].v[0];
					vert_in_face++;
				}
			}
		}

		int* offs = (int*)(bsp_texdata + sizeof(int));
		BSPMiptex_t* miptex = (BSPMiptex_t*)(bsp_texdata + offs[bsp_texinfs[bsp_faces[i].texinfo].miptex]);

		/////////////////////////////////////////
		//заполняем вершинный буфер
		/////////////////////////////////////////

		w = atlas.getTextureWidthByBaseIndex(i);
		h = atlas.getTextureHeightByBaseIndex(i);
		remappedX = atlas.getTextureRemapedXPosByBaseIndex(i);
		remappedY = atlas.getTextureRemapedYPosByBaseIndex(i);

		int pre_pos_in_vbuffer = pos_in_vbuffer;
		for (int j = 0; j < vert_in_face; j++)
		{
			int g = pos_in_vbuffer;

			//p_vdata[g].color = 0xFFFFFFFF;
			p_vdata[g].x = bsp_verts[tmp[j]].point[0];
			p_vdata[g].y = bsp_verts[tmp[j]].point[2]; // swap y <-> z
			p_vdata[g].z = bsp_verts[tmp[j]].point[1];

			p_vdata[g].nx = nx;
			p_vdata[g].ny = ny;
			p_vdata[g].nz = nz;

			//u = tv00 * x + tv01 * z + tv02 * y + tv03
			//v = tv10 * x + tv11 * z + tv12 * y + tv13

			short ti = bsp_faces[i].texinfo;

			//NEW:
			p_vdata[g].tu =(
				bsp_texinfs[ti].vecs[0][0] * bsp_verts[tmp[j]].point[0] +
				bsp_texinfs[ti].vecs[0][2] * bsp_verts[tmp[j]].point[2] +
				bsp_texinfs[ti].vecs[0][1] * bsp_verts[tmp[j]].point[1] +
				bsp_texinfs[ti].vecs[0][3] );
			p_vdata[g].tv =(
				bsp_texinfs[ti].vecs[1][0] * bsp_verts[tmp[j]].point[0] +
				bsp_texinfs[ti].vecs[1][2] * bsp_verts[tmp[j]].point[2] +
				bsp_texinfs[ti].vecs[1][1] * bsp_verts[tmp[j]].point[1] +
				bsp_texinfs[ti].vecs[1][3] );

			if (s->styles[0] == 0)
			{
				tex_bounds[0] = maxs[0] - mins[0];
				tex_bounds[1] = maxs[1] - mins[1];
				tex_center[0] = tex_bounds[0] * 0.5f + mins[0];
				tex_center[1] = tex_bounds[1] * 0.5f + mins[1];

				float scaleU = (float)((float)texsize[0] - 1.f) / (float)texsize[0];
				float scaleV = (float)((float)texsize[1] - 1.f) / (float)texsize[1];

				float dott[2] = { (p_vdata[g].tu * 0.0625f), (p_vdata[g].tv * 0.0625f) }; // /16 = *0.0625

				p_vdata[g].tu2 = ((dott[0] - tex_center[0]) / tex_bounds[0]) * scaleU + 0.5f;
				p_vdata[g].tv2 = ((dott[1] - tex_center[1]) / tex_bounds[1]) * scaleV + 0.5f;

				float pxU = 1.f / atlasW;
				float pxV = 1.f / atlasH;

				//remap for Lightmap atlas
				p_vdata[g].tu2 = ( pxU * remappedX + p_vdata[g].tu2 * (float)w / atlasW);
				p_vdata[g].tv2 = ( pxV * remappedY + p_vdata[g].tv2 * (float)h / atlasH);

			}
			else
			{
				p_vdata[g].tu2 = 1.0f; //BUGFIX: unlighted polys was gray, now is white
				p_vdata[g].tv2 = 1.0f; 
			}

			p_vdata[g].tu /= miptex->width;
			p_vdata[g].tv /= miptex->height;

			pos_in_vbuffer++;
		}

		
		///////////////////////////////////////////////////////
		//по проиндексированным вершинам строим многоугольники
		///////////////////////////////////////////////////////

		int tris_in_face = vert_in_face - 2;

		rfaces[i].isDrawed = false;
		rfaces[i].start_indx = pos_in_ibuffer;
		rfaces[i].num_prim = tris_in_face;
		rfaces[i].miptex = bsp_texinfs[bsp_faces[i].texinfo].miptex;

		first_idx = pre_pos_in_vbuffer;
		for (int j = 0; j < tris_in_face ; j++)
		{
			int g = pos_in_ibuffer;
			p_idata[g] = first_idx;
			p_idata[g + 1] = j + 1 + first_idx ;
			p_idata[g + 2] = j + 2 + first_idx;
			pos_in_ibuffer += 3;

		}
	}

	r_num_i = pos_in_ibuffer;
	r_num_v = pos_in_vbuffer;

	m_VB->Unlock();
	m_IB->Unlock();

	*/

	pD3DDev9->SetRenderState(D3DRS_ZENABLE, true);
	pD3DDev9->SetRenderState(D3DRS_AMBIENT, 0xFFFFFFFF);
	pD3DDev9->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);


	pD3DDev9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	pD3DDev9->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	//pD3DDev9->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pD3DDev9->SetRenderState(D3DRS_ALPHATESTENABLE, true);
	pD3DDev9->SetRenderState(D3DRS_ALPHAREF, 0x00000010);
	pD3DDev9->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	

	D3DLIGHT9 l;
	ZeroMemory(&l, sizeof(l));
	l.Diffuse.a = 1.0f;
	l.Diffuse.r = 0.4f;
	l.Diffuse.g = 0.4f;
	l.Diffuse.b = 0.4f;
	l.Direction = D3DXVECTOR3(-1.f, -0.5f, -1.f);
	D3DXVec3Normalize(&(D3DXVECTOR3)(l.Direction), &D3DXVECTOR3(l.Direction));
	
	l.Type = D3DLIGHT_DIRECTIONAL;
	//l.Type = D3DLIGHT_POINT;

	l.Range = 9000.f;
	l.Position.x = 6400.f;
	l.Position.y = 1200.f;
	l.Position.z = 6400.f;

	l.Attenuation0 = 0.0f;
	l.Attenuation1 = 0.0003f;
	l.Attenuation2 = 0.0000001f;

	pD3DDev9->SetLight(0, &l);

	D3DMATERIAL9 m;
	ZeroMemory(&m, sizeof(m));
	m.Ambient.a = 1.0f;
	m.Ambient.r = 1.0f;
	m.Ambient.g = 1.0f;
	m.Ambient.b = 1.0f;

	m.Diffuse.a = 1.0f;
	m.Diffuse.r = 1.0f;
	m.Diffuse.g = 1.0f;
	m.Diffuse.b = 1.0f;

	m.Specular.a = 0.f;
	m.Specular.r = 0.f;
	m.Specular.b = 0.f;
	m.Specular.g = 0.f;

	m.Emissive.a = 0.0f;
	m.Emissive.r = 0.0f;
	m.Emissive.g = 0.0f;
	m.Emissive.b = 0.0f;

	pD3DDev9->SetMaterial(&m);
	pD3DDev9->SetRenderState(D3DRS_AMBIENT, 0xFF4f4f4f);
	pD3DDev9->LightEnable(0, true);
	pD3DDev9->SetRenderState(D3DRS_LIGHTING, true);

	//WAD TEX LOAD
	//loadWAD();

	pD3DDev9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	pD3DDev9->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	pD3DDev9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	pD3DDev9->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 16);

	pD3DDev9->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_GAUSSIANQUAD);
	pD3DDev9->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	pD3DDev9->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pD3DDev9->SetSamplerState(1, D3DSAMP_MAXANISOTROPY, 16);

	pD3DDev9->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pD3DDev9->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	pD3DDev9->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pD3DDev9->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	pD3DDev9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);

	D3DCAPS9 d3dCaps;

	pD3DDev9->GetDeviceCaps(&d3dCaps);
	int IndexedMatrixMaxSize = d3dCaps.MaxVertexBlendMatrixIndex;

	//TEST
	FILE* fo = 0;
	fopen_s(&fo, "out_entities.txt", "wb");
	fwrite(((byte*)bsp_header) + bsp_header->lumps[LUMP_ENTITIES].fileofs, bsp_header->lumps[LUMP_ENTITIES].filelen, 1, fo);
	fclose(fo);

	delete[] (byte*)bsp_header;

	rmgr.onRenderDeviceReset();
	smgr.getRootNode().computeTransform();

	printf("Map loaded\n");
}

inline bool isLeafVisible ( byte* decomprPVS, int leafBit )
{
	if (bsp_visdata == 0)
		return true;

	/*
	unsigned short b = leafBit >> 3; // leaf/8;
	unsigned short bitOffset = leafBit - ( b << 3 ); // b*8

	//extract bit
	b = ( decomprPVS[b] >> bitOffset ) & 1;

	if (!useFrustum)
		return b != 0;
	else
		return (b != 0) && isLeafInFrustum(leafBit+1);
	*/

	bool inFrustum = true;
	if (useFrustum)
		inFrustum =  isLeafInFrustum(leafBit + 1);

	return ((decomprPVS[leafBit >> 3] & (1 << (leafBit & 7)))) && inFrustum; //Original Valve Pvs test
}

int compressRow(byte* vis, byte* dest)
{
	int		j;
	int		rep;
	//int		visrow;
	byte* dest_p;

	dest_p = dest;
	//visrow = (num_leafs + 7) >> 3;
	

	for (j = 0; j < visrow; j++)
	{
		*dest_p++ = vis[j];
		if (vis[j])
			continue;

		rep = 1;
		for (j++; j < visrow; j++)
			if (vis[j] || rep == 255)
				break;
			else
				rep++;
		*dest_p++ = rep;
		j--;
	}

	return dest_p - dest;
}

int decompressRow(byte* visCompr, byte* dest)
{
	//TEST:
	//int bitbytes = ((visLeafs + 63) & ~63) >> 3;

	//int visrow = (visLeafs + 7) >> 3;
	int counter = 0;
	byte* p = &dest[0];
	byte* v = &visCompr[0];

	int compReaded = 0;

	while (counter < visrow)
	//while (counter < bitbytes)
	{
		if ((*v) != 0)
		{
			(*p) = (*v);
			p++; v++; compReaded++;
			counter++;
		}
		else
		{
			v++;
			memset(p, 0, *v); //fill in "dest" zeros
			p += *v;
			counter += *v;
			v++;
			compReaded += 2;
		}
	}
	return v - visCompr;
}


void unLoadScene()
{
	//rmgr.destroyResource("bspLevel", GRESGROUP_BSPLEVEL);

	//////////////////////////////////////////////////////////////////
	//// scene node graph test
	//smgr.destroyNode("new_central");
	//smgr.destroyNode("new_joint1");
	//smgr.destroyNode("new_joint2");
	//smgr.destroyNode("new_joint3");
	//smgr.destroyNode("new_joint4");
	//smgr.destroyNode("new_joint5");
	//smgr.destroyNode("new_joint51");
	//smgr.destroyNode("new_joint52");
	//smgr.destroyNode("new_joint53");
	//smgr.destroyNode("new_skin1");
	//smgr.destroyNode("new_skin2");
	//smgr.destroyNode("new_skin3");
	//smgr.destroyNode("new_crystal");
	//smgr.destroyNode("new_terrain");
	smgr.getRootNode().destroyChildren();

	smgr.destroyAllEntities();

	unLoadLightmaps();
	unLoadWAD();

	//terrain.unload();

	//smgr.getRootNode().destroyChildren();
	//smgr.destroyAllEntities();
	//rmgr.unloadAllResources();

	if (m_VB) m_VB->Release();
	m_VB = 0;
	if (m_IB) m_IB->Release();
	m_IB = 0;

	if (bsp_verts) delete[] bsp_verts; bsp_verts = 0;
	if (bsp_edges) delete[] bsp_edges; bsp_edges = 0;
	if (bsp_faces) delete[] bsp_faces; bsp_faces = 0;
	if (bsp_texinfs) delete[] bsp_texinfs; bsp_texinfs = 0;
	if (bsp_planes) delete[] bsp_planes; bsp_planes = 0;
	if (bsp_surfedges) delete[] bsp_surfedges; bsp_surfedges = 0;
	if (bsp_models) delete[] bsp_models; bsp_models = 0;
	if (bsp_leafs) delete[] bsp_leafs; bsp_leafs = 0;
	if (bsp_nodes) delete[] bsp_nodes; bsp_nodes = 0;
	if (bsp_marksurfaces) delete[] bsp_marksurfaces; bsp_marksurfaces = 0;
	if (bsp_clipnodes) delete[] bsp_clipnodes; bsp_clipnodes = 0;

	if (bsp_texdata) delete[] bsp_texdata; bsp_texdata = 0;
	if (bsp_lightdata) delete[] bsp_lightdata; bsp_lightdata = 0;
	if (bsp_visdata) delete[] bsp_visdata; bsp_visdata = 0;

	num_verts = 0;
	num_edges = 0;
	num_faces = 0;
	num_texinfs = 0;
	num_miptexs = 0;
	num_planes = 0;
	num_surfedges = 0;
	num_models = 0;
	num_leafs = 0;
	num_nodes = 0;
	num_marksurfaces = 0;
	num_clipnodes = 0;

	bsp_lightdatasize = 0;
	bsp_visdatasize = 0;
	bsp_texdatasize = 0;

	if (input)
	{
		delete input;
		input = 0;
	}

	if (timer)
	{
		delete timer;
		timer = 0;
	}

	rmgr.unloadAllResources();
}

#define PLANE_SIZE 5000.f

void drawPlane(int plane, unsigned int color)
{
	D3DXVECTOR3 normal( bsp_planes[plane].normal[0], bsp_planes[plane].normal[2], bsp_planes[plane].normal[1] );
	D3DXVECTOR3 up;

	if( normal.z != normal.y )
		up = D3DXVECTOR3(normal.x, normal.z, normal.y);
	else
	{
		if( normal.x != normal.y )
			up = D3DXVECTOR3(normal.y, normal.x, normal.z);
		else
			up = D3DXVECTOR3(normal.z, normal.y, normal.x);
	}
	D3DXVECTOR3 left;
	D3DXVec3Cross( &left, &up, &normal );

	D3DXVECTOR3 point, p0, p1, p2, p3;

	point = normal *  bsp_planes[plane].dist;
	p0 = point - (up + left) * PLANE_SIZE;
	p1 = point - up * PLANE_SIZE + left * PLANE_SIZE;
	p2 = point + (up + left) * PLANE_SIZE;
	p3 = point + up * PLANE_SIZE - left * PLANE_SIZE;
	
	gVertexAABB points[4] =
	{
		gVertexAABB(p0, color),
		gVertexAABB(p1, color),
		gVertexAABB(p2, color),
		gVertexAABB(p3, color),
	};

	unsigned short ind[6] =
	{
		0,1,2, 0,2,3
	};

	D3DXMATRIX m;
	D3DXMatrixIdentity(&m);

	LPDIRECT3DDEVICE9 pDev = pD3DDev9;
	if (!pDev)return;

	DWORD oldLightingState;
	pDev->GetRenderState(D3DRS_LIGHTING, &oldLightingState);
	pDev->SetRenderState(D3DRS_LIGHTING, false);

	pDev->SetTransform(D3DTS_WORLD, &m);
	pDev->SetTexture(0, 0);

	pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);

	// Set the source blend state.
	pDev->SetRenderState(D3DRS_SRCBLEND,
		D3DBLEND_SRCCOLOR);

	// Set the destination blend state.
	pDev->SetRenderState(D3DRS_DESTBLEND,
		D3DBLEND_INVSRCCOLOR);


	DWORD fvf;
	pDev->GetFVF(&fvf);
	pDev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, &ind, D3DFMT_INDEX16, &points, sizeof(gVertexAABB));
	pDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	pDev->SetFVF(fvf);
	pDev->SetRenderState(D3DRS_LIGHTING, oldLightingState);

	pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
}

void drawAABB(short* bbmin, short* bbmax, DWORD color = 0xFF00FF00, bool noZ = false)
{

	D3DXVECTOR3 bmin(bbmin[0], bbmin[2], bbmin[1]); //SWAP y-z
	D3DXVECTOR3 bmax(bbmax[0], bbmax[2], bbmax[1]);

	gVertexAABB points[8] =
	{
		gVertexAABB(D3DXVECTOR3(bmin.x, bmin.y, bmin.z), color),
		gVertexAABB(D3DXVECTOR3(bmax.x, bmin.y, bmin.z), color),
		gVertexAABB(D3DXVECTOR3(bmax.x, bmax.y, bmin.z), color),
		gVertexAABB(D3DXVECTOR3(bmin.x, bmax.y, bmin.z), color),


		gVertexAABB(D3DXVECTOR3(bmin.x, bmin.y, bmax.z), color),
		gVertexAABB(D3DXVECTOR3(bmax.x, bmin.y, bmax.z), color),
		gVertexAABB(D3DXVECTOR3(bmax.x, bmax.y, bmax.z), color),
		gVertexAABB(D3DXVECTOR3(bmin.x, bmax.y, bmax.z), color),
	};

	unsigned short ind[24] =
	{
		0,1,  1,2, 2,3, 0,3,

		4,5, 5,6, 6,7, 4,7,

		0,4, 1,5, 2,6, 3,7
	};

	D3DXMATRIX m;
	D3DXMatrixIdentity(&m);

	LPDIRECT3DDEVICE9 pDev = smgr.getResourseManager()->getDevice();
	if (!pDev)return;

	pDev->SetTransform(D3DTS_WORLD, &m);

	pDev->SetRenderState(D3DRS_LIGHTING, false);
	pDev->SetTexture(0, 0);

	DWORD fvf;
	pDev->GetFVF(&fvf);
	pDev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	//if(noZ)pDev->SetRenderState(D3DRS_ZENABLE, false);
	pDev->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 8, 12, &ind, D3DFMT_INDEX16, &points, sizeof(gVertexAABB));
	//if(noZ)pDev->SetRenderState(D3DRS_ZENABLE, true);
	pDev->SetFVF(fvf);
}

int last_tex = -1;
bool last_draw_useLMap = false;
FILE* fo = 0;

void drawFace(int i)
{
	rfaces[i].isDrawed = true;
}

void drawLeaf(int leaf)
{
	for (int f = bsp_leafs[leaf].firstmarksurface;
		f < bsp_leafs[leaf].firstmarksurface + bsp_leafs[leaf].nummarksurfaces; f++)
	{
		drawFace(bsp_marksurfaces[f]);
		drawedLeafs++;
	}
}

void renderFaces()
{
	pD3DDev9->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_GAUSSIANQUAD);
	pD3DDev9->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	pD3DDev9->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pD3DDev9->SetSamplerState(1, D3DSAMP_MAXANISOTROPY, 16);

	DWORD oldLightingState;
	pD3DDev9->GetRenderState(D3DRS_LIGHTING, &oldLightingState);
	pD3DDev9->SetRenderState(D3DRS_LIGHTING, false);
	pD3DDev9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	pD3DDev9->SetRenderState(D3DRS_ZWRITEENABLE, true);

	D3DXMATRIX mId;
	D3DXMatrixIdentity(&mId);
	pD3DDev9->SetTransform(D3DTS_WORLD, &mId);

	pD3DDev9->SetStreamSource(0, m_VB, 0, sizeof(D3DVertex));
	pD3DDev9->SetIndices(m_IB);
	pD3DDev9->SetFVF( getFVF(GVERTEXFORMAT::GVF_LEVEL) );

	//LPDIRECT3DTEXTURE9 lmap = ((gResource2DTexture*)rmgr.getResource("lmap", GRESGROUP_2DTEXTURE))->getTexture();
	 
	//set lightmap atlas
	pD3DDev9->SetTexture(1, pTexLightsAtlas);
	pD3DDev9->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	pD3DDev9->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	pD3DDev9->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);

	last_tex = -1;

	if( useLightmaps )
		pD3DDev9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
	else
		pD3DDev9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);


	for (int i = 0; i < num_faces; i++)
	{
		if (!rfaces[i].isDrawed)
			continue;
		else
			rfaces[i].isDrawed = false; //обнуляем для следующего кадра

		if( drawedFaces == 0 )
			last_draw_useLMap = !(bsp_faces[i].styles[0] == 0);

		drawedFaces++;

		if (rfaces[i].miptex != last_tex)
		{
			gResourceTexture* res = (gResourceTexture*)tmap[rfaces[i].miptex];
			LPDIRECT3DBASETEXTURE9 tex = 0;

			if (res) tex = (LPDIRECT3DBASETEXTURE9)res->getTexture();

			last_tex = rfaces[i].miptex;
			pD3DDev9->SetTexture(0, tex);
		}
/*
		if( (bsp_faces[i].styles[0] == 0) ) //если имеется лайт мап рисуем с ним
		{ 
			//pD3DDev9->SetTexture(1, ltmap[i]);
			if( !last_draw_useLMap)
			{
				last_draw_useLMap = true;
			}
		}
		else
		{
			if (last_draw_useLMap)
			{
				last_draw_useLMap = false;
			}
		}
*/
		pD3DDev9->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, r_num_v, rfaces[i].start_indx, rfaces[i].num_prim);
	}

	pD3DDev9->SetTexture(1, 0);
	pD3DDev9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTEXOPCAPS_DISABLE);
	pD3DDev9->SetRenderState(D3DRS_LIGHTING, oldLightingState);

	pD3DDev9->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDev9->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	pD3DDev9->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	pD3DDev9->SetSamplerState(1, D3DSAMP_MAXANISOTROPY, 16);
}

void drawModel( int model )
{
	if (isModelInFrustum(model))
	{
		modelsCounter++;

		D3DXMATRIX mOrigin;
		D3DXMatrixTranslation(&mOrigin, bsp_models[model].origin[0],
			bsp_models[model].origin[2], bsp_models[model].origin[1]);

		pD3DDev9->SetTransform( D3DTS_WORLD, &mOrigin );
		 
		for (int j = bsp_models[model].firstface; j < bsp_models[model].firstface + bsp_models[model].numfaces; j++)
		{
			drawFace(j);
		}
	}
}

void drawModels( int first, int last )
{
	for (int i = first; i < last; i++)
	{
		drawModel(i);
	}
}

void drawVisibleLeafs(int camleaf)
{
	byte pvs[1024];
	memset(&pvs[0], 0, 1024);

	int decomprSz = 0;
	if( bsp_visdatasize > 0)
		decomprSz = decompressRow(&bsp_visdata[bsp_leafs[camleaf].visofs], &pvs[0]);

	for (int leaf = 0; leaf < visLeafs; leaf++)
	{
		if (isLeafVisible(&pvs[0], leaf)) //&& isLeafInFrustum(leaf + 1))
		{
			drawLeaf(leaf+1);
			drawedLeafs++;
		}
	}
}

void drawVisibleLeafsAABB(int camleaf)
{
	byte pvs[1024];
	memset(&pvs[0], 0, 1024);

	int compSizes = decompressRow(&bsp_visdata[bsp_leafs[camleaf].visofs], &pvs[0]);

	for (int leaf = 0; leaf < visLeafs; leaf++)
	{
		if (isLeafVisible(&pvs[0], leaf)) //&& isLeafInFrustum(leaf + 1) )
		{
			drawAABB(bsp_leafs[leaf + 1].mins, bsp_leafs[leaf + 1].maxs, 0xFFFF0000);

		}
	}
}

void drawMarkedFace( int face )
{
	DWORD oldFillMode;
	pD3DDev9->GetRenderState(D3DRS_FILLMODE, &oldFillMode);
	pD3DDev9->SetRenderState(D3DRS_LIGHTING, false);
	pD3DDev9->SetTexture(0, 0);
	pD3DDev9->SetTexture(1, 0);

	pD3DDev9->SetRenderState(D3DRS_ZENABLE, false);
	pD3DDev9->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	pD3DDev9->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, r_num_v, rfaces[face].start_indx, rfaces[face].num_prim);
	pD3DDev9->SetRenderState(D3DRS_FILLMODE, oldFillMode);
	pD3DDev9->SetRenderState(D3DRS_ZENABLE, true);
}

void frame_render()
{
	HRESULT coopLevel = pD3DDev9->TestCooperativeLevel();
	if (SUCCEEDED(coopLevel))
	{
		pD3DDev9->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFF7F7F7F, 1.0f, 0);
		pD3DDev9->BeginScene();

		D3DXMATRIX mId;
		D3DXMatrixIdentity(&mId);
		pD3DDev9->SetTransform(D3DTS_WORLD, &mId);

		pD3DDev9->SetTransform(D3DTS_VIEW, &cam.getViewMatrix());
		pD3DDev9->SetTransform(D3DTS_PROJECTION, &cam.getProjMatrix());

		//pD3DDev9->SetStreamSource(0, m_VB, 0, sizeof(D3DVertex));
		//pD3DDev9->SetIndices(m_IB);
		//pD3DDev9->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2);

		int tri_counter = 0;
		int tris_in_face = 0;
		last_tex = -1;

		drawedFaces = 0;
		drawedLeafs = 0;
		numSetTexCalls = 0;

		//int steps = 0;
		//currentLeaf = getLeafInCamPosition(&steps);

		

		/*
		if (bsp_visdatasize != 0)
		{
			if (rmodels)
				drawModels(1, num_models);
		}
		else
			drawModels(0, num_models);
		*/
		
		/*
		D3DXVECTOR3 p = cam.getPosition();
		int content = isCollideWithWorld(p, 1500.f);

		if (currentLeaf > 0)
		{
			drawVisibleLeafs(currentLeaf);
		}
		else if (currentLeaf == 0)
		{
			for (int i = 0; i < visLeafs; i++)
			{
				drawLeaf(i);
			}
		}

		*/
		//renderFaces();

		/*
		char tmp[1024];
		sprintf_s(tmp, 1024, "Map: %s%s, Num Leafs: %i, Curent Leaf: %i, Steps to find: %i, Visible Leafs: %i, Visible Faces: %i, Frustum: %s, LMaps: %s, Content: %i",
			filesMapList[currentMap].c_str(), bsp_visdatasize > 0 ? "" : "(NO_VIS)", num_leafs, currentLeaf, steps, drawedLeafs, drawedFaces, useFrustum ? "on" : "off", useLightmaps ? "on" : "off", content);
		wnd_setTitle(tmp);
		*/

		/*
		if (rbboxes)
		{
			for (int i = 0; i < num_leafs; i++)
			{
				drawAABB(bsp_leafs[i].mins, bsp_leafs[i].maxs, 0xFF00FF00);
			}
		}
		
		if ((currentLeaf > 0) && rbboxes)
		{
			pD3DDev9->SetRenderState(D3DRS_ZENABLE, false);

			drawAABB(bsp_leafs[currentLeaf].mins, bsp_leafs[currentLeaf].maxs, 0xFFFF0000, true);
			drawVisibleLeafsAABB(currentLeaf);

			pD3DDev9->SetRenderState(D3DRS_ZENABLE, true);

		}
		*/
	

		smgr.frameRender(rqueue);

		if (input->isKeyDown(DIK_Y))
		{
			rqueue._debugOutSorted("out_queue_sorted.txt");
			rqueue._debugOutUnsorted("out_queue_unsorted.txt");
		}
		rqueue.render(pD3DDev9);


		//Draw Text Test
		pD3DDev9->SetRenderState(D3DRS_ZENABLE, false);
		gResourceTextDrawer* tdrawer = (gResourceTextDrawer*)rmgr.getResource("font1", GRESGROUP_TEXTDRAWER);
		tdrawer->drawInScreenSpace("Test test", 400, 400, 0xFF00FF00, 1024, 768);

		char buff[256];
		sprintf_s(buff, 256, "Yaw: %f    Pitch: %f", cam.getYaw(), cam.getPitch());
		tdrawer->drawInScreenSpace(buff, 20, 20, 0xFF00FF00, 1024, 768);

		//sprintf_s( buff, 256, "Current face: %i", currentFace );
		//tdrawer->drawInScreenSpace(buff, 20, 60, 0xFF00FF00, 1024, 768);

		D3DXVECTOR3 v;
		D3DVIEWPORT9 viewport;
		pD3DDev9->GetViewport(&viewport);
		cam.projPointToScreen(D3DXVECTOR3(0.f, 0.f, 0.f), v, viewport);

		tdrawer = (gResourceTextDrawer*)rmgr.getResource("font2", GRESGROUP_TEXTDRAWER);

		if (v.z < 0.f)
			tdrawer->drawInScreenSpace("Null Point\nПроверка", (int)v.x, (int)v.y, 0xFF00FF00, 1024, 768);

		//drawMarkedFace(currentFace);

		pD3DDev9->SetRenderState(D3DRS_ZENABLE, true);

		pD3DDev9->EndScene();
		pD3DDev9->Present(0, 0, 0, 0);
	}
}

void rebuildVB( float dt, float delta_x, float delta_y, float sc )
{
	D3DVertex* p_vdata = 0;

	char tmp[256];


	HRESULT hr = m_VB->Lock(0, sizeof(D3DVertex) * r_num_v, (void**)& p_vdata, D3DLOCK_NO_DIRTY_UPDATE );
	if (FAILED(hr))
		throw("Ошибка при блокировке вершинного буффера!");

	float old_x = offset_lmap_x;
	float old_y = offset_lmap_y;
	float old_scale = scale;

	offset_lmap_x += dt * delta_x;
	offset_lmap_y += dt * delta_y;
	scale += dt * sc;

	sprintf_s(tmp, "Lightmap offset_x: %f  offset_y: %f  scale: %f\n", offset_lmap_x, offset_lmap_y, scale);
	wnd_setTitle(tmp);

	for (int i = 0; i < r_num_v; i++, p_vdata++)
	{
		if (sc != 0)
		{	
			p_vdata->tu2 /= old_scale;
			p_vdata->tv2 /= old_scale;
			p_vdata->tu2 *= scale;
			p_vdata->tv2 *= scale;
		}

		p_vdata->tu2 -= old_x;
		p_vdata->tv2 -= old_y;

		p_vdata->tu2 += offset_lmap_x;
		p_vdata->tv2 += offset_lmap_y;
	}

	m_VB->Unlock();
}

bool frame_move()
{
	float dt = timer->getDelta();

	ti += dt * 0.2f;

	D3DXQUATERNION q;

	gSceneNode* node = smgr.getNode("new_central");
	if (node)
	{
		D3DXQuaternionRotationAxis(&q, &D3DXVECTOR3(0, 1.f, 0), ti * 1.5f);
		node->setRelativeOrientation(q);
	}

	node = smgr.getNode("new_joint2");
	if (node)
	{
		D3DXQuaternionRotationAxis(&q, &D3DXVECTOR3(0, 1.f, 0), ti * 5.5f);
		node->setRelativeOrientation(q);
	}
	node = smgr.getNode("new_joint4");
	if (node)
	{
		D3DXQuaternionRotationAxis(&q, &D3DXVECTOR3(0, 1.f, 0), ti * 10.5f);
		node->setRelativeOrientation(q);
	}

	//node = smgr.getNode("new_joint5");
	//if (node)
	//{
	//	D3DXQuaternionRotationAxis(&q, &D3DXVECTOR3(0, 1.f, 0), ti * 20.5f);
	//	node->setRelativeOrientation(q);
	//}

	input->update();
	//cam.tick(dt);

	//smgr.getRootNode().computeTransform();
	smgr.frameMove(dt);



	//cam.setRelativePosition(D3DXVECTOR3(-3000, 300, -3000));
	//if( cam.getViewingFrustum().testAABB( D3DXVECTOR3( -3000, 300, -3000 ), D3DXVECTOR3( -3010, -300, -3010 ) ) )
	//	wnd_setTitle("Видимо");
	//else
	//	wnd_setTitle("Не видимо");

	//set transform used in smrg
	//pD3DDev9->SetTransform(D3DTS_VIEW, &cam.getViewMatrix());
	//pD3DDev9->SetTransform(D3DTS_PROJECTION, &cam.getProjMatrix());

	if (input->isKeyPressed(DIK_RIGHT))
	{
		if (currentFace < num_faces)
			currentFace++;
	}

	if (input->isKeyPressed(DIK_LEFT))
	{
		if (currentFace > 0)
			currentFace--;
	}

	if (input->isKeyDown(DIK_UP))
	{
		if (currentFace < num_faces)
			currentFace++;
	}
		
	if (input->isKeyDown(DIK_DOWN))
	{
		if (currentFace > 0)
			currentFace--;
	}

	if (input->isKeyPressed(DIK_P))
		rebuildVB(dt, 0.f, 0.0f, 0.1f);
	if (input->isKeyPressed(DIK_O))
		rebuildVB(dt, 0.f, 0.0f, -0.1f);

	if (input->isKeyDown(DIK_Q))
		if (currentLeaf > 0)
			currentLeaf--;
	if (input->isKeyDown(DIK_E))
		if (currentLeaf < num_leafs)
			currentLeaf++;

	if (input->isKeyDown(DIK_G))
		cam.lookAt(D3DXVECTOR3(0, 0, 0));

	if (input->isKeyDown(DIK_F))
		cam.setOrientation(D3DXVECTOR3(0, 0, 1));

	if (input->isKeyPressed(DIK_R))
		if (currentLeaf > 0)
			currentLeaf--;
	if (input->isKeyPressed(DIK_T))
		if (currentLeaf < num_nodes)
			currentLeaf++;

	if (input->isKeyPressed(DIK_Y))
		if (currentNode > 0)
			currentNode--;
	if (input->isKeyPressed(DIK_U))
		if (currentNode < num_nodes)
			currentNode++;

	if (input->isKeyDown(DIK_B))
		rbboxes = !rbboxes;

	if (input->isKeyDown(DIK_F))
		useFrustum = !useFrustum;

	if (input->isKeyDown(DIK_M))
		rmodels = !rmodels;

	if (input->isKeyDown(DIK_L))
		useLightmaps = !useLightmaps;
	

	if ( input->isKeyDown(DIK_F1) )
	{
		if (currentMap <= 0)
			currentMap = filesMapList.size() - 1;
		else
			currentMap--;

		loadScene(filesMapList.find(currentMap)->second.c_str());
		frame_move();
	}

	if ( input->isKeyDown(DIK_F2) )
	{
		if (currentMap >= filesMapList.size() - 1)
			currentMap = 0;
		else
			currentMap++;

		loadScene(filesMapList.find(currentMap)->second.c_str());
		frame_move();
	}

	if (input->isKeyDown(DIK_F3))
	{
		loadScene(filesMapList.find(currentMap)->second.c_str());
		frame_move();
	}

	if (input->isKeyDown(DIK_RETURN) )
	{
		bFullscreen = !bFullscreen;

		//wnd_hide();
		unLoadScene();

		d3d9_setFullScreen(bFullscreen);
		if (bFullscreen)
			d3d9_setDisplayWH(1920, 1080);
		else
			d3d9_setDisplayWH(1024, 768);
		
		if (d3d9_reset())
		{
			loadScene(filesMapList[currentMap].c_str());
			frame_move();
			//wnd_show();
			return true;
		}
		return false;
	}

	static bool wf = false;
	if (input->isKeyDown(DIK_SPACE))
	{
		wf = !wf;
		pD3DDev9->SetRenderState(D3DRS_FILLMODE, wf ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
	}
	return true;
}

void fillMapsList()
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hf;
	hf = FindFirstFile( "../data/maps/*.bsp", &FindFileData );
	if (hf != INVALID_HANDLE_VALUE)
	{
		int count = 0;
		do
		{
			filesMapList[count] = FindFileData.cFileName;
			count++;
		} while (FindNextFile(hf, &FindFileData) != 0);
		FindClose(hf);
	}
}

void cleanUp()
{
	unLoadScene();
	d3d9_destroy();
	wnd_destroy();
}

int wmain(int argc, wchar_t* argv[])
{
	try
	{
		wnd_create( "BSP LEVEL LOADER", 1024, 768 );
		wnd_setFrameMoveCallBack(frame_move);
		wnd_setFrameRenderCallBack(frame_render);
		wnd_setCleanUpCallBack( cleanUp );
		d3d9_init(bFullscreen);
		fillMapsList();

		loadScene(filesMapList[currentMap].c_str());
		wnd_show();
		wnd_update();
		input->reset();

		MSG msg = {0};
		while (true)
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					cleanUp();
					return 0;
				};

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (frame_move())
				frame_render();
		}
	}
	catch( const char* msg )
	{
		wnd_hide();
		MessageBox( 0, msg, "BSP Loader ", 
			MB_OK | MB_ICONERROR | MB_SYSTEMMODAL );
	}
	cleanUp();
	return 0;
}