#pragma once
#ifndef _G_MATH_H_
#define _G_MATH_H_

//===========================================================
//	FROM GPUGEMS IV:
//===========================================================

// QuatTypes.h
/*** Definitions ***/
typedef struct { float x, y, z, w; } Quat; /* Quaternion */
enum QuatPart { X, Y, Z, W };
typedef float HMatrix[4][4]; /* Right-handed, for column vectors */
typedef Quat EulerAngles;    /* (x,y,z)=ang 1,2,3, w=order code  */

// EulerAngles.h

/*** Order type constants, constructors, extractors ***/
	/* There are 24 possible conventions, designated by:    */
	/*	  o EulAxI = axis used initially		    */
	/*	  o EulPar = parity of axis permutation		    */
	/*	  o EulRep = repetition of initial axis as last	    */
	/*	  o EulFrm = frame from which axes are taken	    */
	/* Axes I,J,K will be a permutation of X,Y,Z.	    */
	/* Axis H will be either I or K, depending on EulRep.   */
	/* Frame S takes axes from initial static frame.	    */
	/* If ord = (AxI=X, Par=Even, Rep=No, Frm=S), then	    */
	/* {a,b,c,ord} means Rz(c)Ry(b)Rx(a), where Rz(c)v	    */
	/* rotates v around Z by c radians.			    */
#define EulFrmS	     0
#define EulFrmR	     1
#define EulFrm(ord)  ((unsigned)(ord)&1)
#define EulRepNo     0
#define EulRepYes    1
#define EulRep(ord)  (((unsigned)(ord)>>1)&1)
#define EulParEven   0
#define EulParOdd    1
#define EulPar(ord)  (((unsigned)(ord)>>2)&1)
/* this code is merely a quick (and legal!) way to set arrays, EulSafe being 0,1,2,0 */
#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulAxI(ord)  ((int)(EulSafe[(((unsigned)(ord)>>3)&3)]))
#define EulAxJ(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)==EulParOdd)]))
#define EulAxK(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)!=EulParOdd)]))
#define EulAxH(ord)  ((EulRep(ord)==EulRepNo)?EulAxK(ord):EulAxI(ord))
	/* EulGetOrd unpacks all useful information about order simultaneously. */
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=(unsigned)ord;f=o&1;o>>=1;s=o&1;o>>=1;\
    n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}
	/* EulOrd creates an order value between 0 and 23 from 4-tuple choices. */
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
	/* Static axes */
#define EulOrdXYZs    EulOrd(X,EulParEven,EulRepNo,EulFrmS)
#define EulOrdXYXs    EulOrd(X,EulParEven,EulRepYes,EulFrmS)
#define EulOrdXZYs    EulOrd(X,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdXZXs    EulOrd(X,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdYZXs    EulOrd(Y,EulParEven,EulRepNo,EulFrmS)
#define EulOrdYZYs    EulOrd(Y,EulParEven,EulRepYes,EulFrmS)
#define EulOrdYXZs    EulOrd(Y,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdYXYs    EulOrd(Y,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdZXYs    EulOrd(Z,EulParEven,EulRepNo,EulFrmS)
#define EulOrdZXZs    EulOrd(Z,EulParEven,EulRepYes,EulFrmS)
#define EulOrdZYXs    EulOrd(Z,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdZYZs    EulOrd(Z,EulParOdd,EulRepYes,EulFrmS)
	/* Rotating axes */
#define EulOrdZYXr    EulOrd(X,EulParEven,EulRepNo,EulFrmR)
#define EulOrdXYXr    EulOrd(X,EulParEven,EulRepYes,EulFrmR)
#define EulOrdYZXr    EulOrd(X,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdXZXr    EulOrd(X,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdXZYr    EulOrd(Y,EulParEven,EulRepNo,EulFrmR)
#define EulOrdYZYr    EulOrd(Y,EulParEven,EulRepYes,EulFrmR)
#define EulOrdZXYr    EulOrd(Y,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdYXYr    EulOrd(Y,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdYXZr    EulOrd(Z,EulParEven,EulRepNo,EulFrmR)
#define EulOrdZXZr    EulOrd(Z,EulParEven,EulRepYes,EulFrmR)
#define EulOrdXYZr    EulOrd(Z,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdZYZr    EulOrd(Z,EulParOdd,EulRepYes,EulFrmR)

EulerAngles Eul_(float ai, float aj, float ah, int order);
Quat Eul_ToQuat(EulerAngles ea);
void Eul_ToHMatrix(EulerAngles ea, HMatrix M);
EulerAngles Eul_FromHMatrix(HMatrix M, int order);
EulerAngles Eul_FromQuat(Quat q, int order);

//===========================================================

struct D3DXQUATERNION;
struct D3DXVECTOR3;
class gSkinBone;

//quat - euler funcs
void _quatShortestArc(D3DXQUATERNION& out, const D3DXVECTOR3& v0, const D3DXVECTOR3& v1);
void _quatToYawPitchRoll(const D3DXQUATERNION& q, float* yaw, float* pitch, float* roll);
void _quatToEuler(const D3DXQUATERNION& q, float* a0, float* a1, float* a2, int eulOrder);
void _YawPitchRolltoQuat(D3DXQUATERNION& q, float yaw, float pitch, float roll);
void _EultoQuat(D3DXQUATERNION& q, float a0, float a1, float a2, int order);

//Hierarchy transforms
void _transformHierarchyVec3( const D3DXQUATERNION& q_parent, const D3DXVECTOR3& v_parent,
	const D3DXVECTOR3& v_child, D3DXVECTOR3& vout);
void _transformHierarchyFrameBones( gSkinBone* frameBones, int bone, int bonesNum );

#endif
