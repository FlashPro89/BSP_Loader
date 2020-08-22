#pragma once

#ifndef _COL_DET_H_
#define _COL_DET_H_

#include <d3dx9.h>

class gAABB
{

public:
	gAABB();
	gAABB( const D3DXVECTOR3& min, const D3DXVECTOR3& max);

	const D3DXVECTOR3& getMaxBounds() const;
	const D3DXVECTOR3& getMinBounds() const;

	void setMaxBounds( const D3DXVECTOR3& max );
	void setMinBounds( const D3DXVECTOR3& min );
	void setMaxBounds( float x, float y, float z );
	void setMinBounds( float x, float y, float z );

	void addPoint( const D3DXVECTOR3& point );
	void addAABB( const gAABB& other );

	void reset();
	bool isEmpty() const;

	void getTransformedByMatrix( gAABB& out, const D3DXMATRIX& transform ) const;

	void getCenterPoint(D3DXVECTOR3* outCenter);
	void setScale( float scale );

protected:
	D3DXVECTOR3 m_bmin;
	D3DXVECTOR3 m_bmax;
};

#endif