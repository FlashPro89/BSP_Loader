#include <d3d9.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <map>
#include "RenderQueue.h"
#include "Mesh.h"
#include "Resources.h"
#include "Materials.h"

//-----------------------------------------------
//
//	CLASS: gRenderElement
//
//-----------------------------------------------

/*
gRenderElement::gRenderElement( gRenderElement& other )
{

}
*/

gRenderElement::gRenderElement()
{
	m_key = 0;
	m_pRenderable = 0;
	m_pMaterial = 0;
	m_distance = 0;
	m_pMatPalete = 0;
	m_paleteSize = 0;
	m_startBufferIndex = 0;
	m_primitiveCount = 0;
	m_vertexesNum = 0;
	m_pSkinBoneGroup = 0;
}

gRenderElement::gRenderElement(const gRenderable* renderable, const gMaterial* material, unsigned short distance, unsigned char matrixPaleteSize,
	const D3DXMATRIX* matrixPalete, unsigned int startIndex, unsigned int primitiveCount, unsigned int vertexesNum, const gSkinBoneGroup* remapedBones)
{
	if (!renderable)
		throw("Null renderable pointer!");

	m_pRenderable = renderable;
	m_pMaterial = material;
	m_distance = distance * 0xFFFF; //distance 0.f ... 1.f 
	m_pMatPalete = matrixPalete;
	m_paleteSize = matrixPaleteSize;

	m_startBufferIndex = startIndex;
	m_primitiveCount = primitiveCount;
	m_vertexesNum = vertexesNum;
	m_pSkinBoneGroup = remapedBones;
	m_distance = distance;

	_buildKey();
}

gRenderElement::~gRenderElement()
{

}

GRQSORTINGKEY gRenderElement::getKey() const
{
	return m_key;
}

const gRenderable* gRenderElement::getRenderable() const
{
	return m_pRenderable;
}

const gMaterial* gRenderElement::getMaterial() const
{
	return m_pMaterial;
}

const D3DXMATRIX* gRenderElement::getMatrixPalete() const
{
	return m_pMatPalete;
}

unsigned char gRenderElement::getMatrixPaleteSize() const
{
	return m_paleteSize;
}

unsigned short gRenderElement::getDistance() const
{
	return m_distance;
}

unsigned int gRenderElement::getStartIndex() const
{
	return m_startBufferIndex;
}

unsigned int gRenderElement::getPrimitiveCount()  const
{
	return m_primitiveCount;
}

unsigned int gRenderElement::getVertexesNum()  const
{
	return m_vertexesNum;
}

const gSkinBoneGroup* gRenderElement::getSkinBoneGroup() const
{
	return m_pSkinBoneGroup;
}

void gRenderElement::_buildKey()
{
	if (!m_pMaterial)
		throw("no mat!");

	unsigned __int64 alphaMask = ((unsigned __int64)1 << 60);
	unsigned __int64 skyMask = ((unsigned __int64)1 << 59);


	if( m_pMaterial->getTransparency() != 0xFF )
		alphaMask = 0;

	if (alphaMask == 0) //opaque
	{
		//16 bit of dist 
		m_key = m_distance;
		
		//1024 types of materials max - 10bits
		m_key = m_key << 10;
		m_key |= m_pMaterial->getId();

		// 1024 types of renderables 10bits
		m_key = m_key << 10;
		m_key |= m_pRenderable->getId();

	}
	else //solid
	{
		//1024 types of materials max - 10bits
		m_key = m_pMaterial->getId();

		// 1024 types of renderables 10bits
		m_key = m_key << 10;
		m_key |= m_pRenderable->getId();

		//16 bit of dist 
		m_key = m_key << 16;
		m_key |= 0xFFFF - m_distance;
	}

	//alphablend bit
	m_key |= alphaMask;

	//skybox bit
	if( m_pRenderable->getGroup() != GRESOURCEGROUP::GRESGROUP_SKYBOX )
		m_key |= skyMask;
}

//-----------------------------------------------
//
//	CLASS: gRenderQueue
//
//-----------------------------------------------

gRenderQueue::gRenderQueue()
{
	m_elements = 0;
	m_elementsPointers = 0;
	m_elementsArraySize = 0;
	m_arrayPos = 0;

	memset(m_tmpIndexes, 0, sizeof(short) * 0xFFFF );
	m_tmpIndexesNum = 0;

	m_oldIB = 0;
	m_oldVB = 0;
	m_oldTextures[0] = m_oldTextures[1] = m_oldTextures[2] = m_oldTextures[3] = 0;
	m_oldTextures[4] = m_oldTextures[5] = m_oldTextures[6] = m_oldTextures[7] = 0;

}

gRenderQueue::~gRenderQueue()
{
	if( m_elements )
		delete[] m_elements;
	if (m_elementsPointers)
		delete[] m_elementsPointers;
}

void gRenderQueue::initialize( unsigned int elementsMaxNum )
{
	if (m_elements)
		delete[] m_elements;
	if (m_elementsPointers)
		delete[] m_elementsPointers;

	m_elementsArraySize = elementsMaxNum;
	m_elements = new gRenderElement[m_elementsArraySize];
	m_elementsPointers = new gRenderElement*[m_elementsArraySize];

	for (unsigned int i = 0; i < m_elementsArraySize; i++)
	{
		m_elementsPointers[i] = &m_elements[i];
	}
}

int compRE( const void* i, const void* j ) 
{
	gRenderElement* pE1 = *(gRenderElement**)i;
	gRenderElement* pE2 = *(gRenderElement**)j;

	__int64 x = pE1->getKey(); //(*((gRenderElement**)i))->getKey();
	__int64 y = pE2->getKey(); //(*((gRenderElement**)j))->getKey();

	if (x > y) return 1; // TODO: сделать чтото с этим
	else if (x == y)
	{
		return pE2->getStartIndex() - pE1->getStartIndex();
	}
	else
		return -1;
}

void gRenderQueue::sort()
{
	/*
	FILE* f = 0;
	errno_t err = fopen_s(&f, "out_sorting_queue.txt", "wt");

	for (unsigned int i = 0; i < m_arrayPos; i++)
	{
		fprintf(f, "%lli ", m_elementsPointers[i]->getKey() );
	}
	fprintf(f, "\n------------------------------------------------------\n");
	*/

	qsort( m_elementsPointers, m_arrayPos, sizeof(gRenderElement**), compRE);

	/*
	for (unsigned int i = 0; i < m_arrayPos; i++)
	{
		fprintf(f, "%lli ", m_elementsPointers[i]->getKey());
	}
	fclose(f);
	*/
}

bool gRenderQueue::pushBack(const gRenderElement& element)
{
	if (!m_elements || m_arrayPos >= m_elementsArraySize)
		return false;
	m_elementsPointers[m_arrayPos] = &m_elements[m_arrayPos];
	m_elements[m_arrayPos++] = element;
	return true;
}

bool gRenderQueue::popBack(gRenderElement** element)
{
	if( !m_elements || m_arrayPos == 0 )
		return false;
	*element = m_elementsPointers[--m_arrayPos];
	return true;
}

void gRenderQueue::clear()
{
	m_arrayPos = 0;
}

void gRenderQueue::render(IDirect3DDevice9* pDevice)
{
	if (m_arrayPos == 0)
		return; //null queue

	D3DXMATRIX mId;
	D3DXMatrixIdentity(&mId);

	D3DXMATRIX mView, mProj;
	pDevice->GetTransform(D3DTS_VIEW, &mView);
	pDevice->GetTransform(D3DTS_PROJECTION, &mProj);

	for (unsigned char i = 0; i < 8; i++)
	{
		m_SS[i].clear();
		m_TSS[i].clear();
	}
	m_RS.clear();
	m_Transform.clear();

	_setTransform(D3DTS_TEXTURE0, &mId, pDevice);
	_setTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2, pDevice);

	LPDIRECT3DINDEXBUFFER9 pSrcIB = 0;
	LPDIRECT3DINDEXBUFFER9 pDestBatchIB = 0;

	gRenderElement* batch_pFirstElement = 0;
	bool batch_begin = false;
	//bool batch_rasterize = false;
	unsigned int batch_primCount = 0;
	unsigned short* batch_pDestBuffData = 0;
	unsigned short* batch_pSrcBuffData = 0;

	gRenderElement* pElement = 0;
	const gMaterial* pMaterial = 0;
	const gResourceTexture* pTex = 0;
	const gRenderable* pRenderable = 0;
	const D3DXMATRIX* matPalete = 0;
	const gSkinBoneGroup* skinBoneGroup = 0;

	const D3DXMATRIX* pLastMatrix = 0;
	int m_lastLighting = -1;
	int m_lastRenderable = -1;
	int m_lastMaterial = -1;

	int m_lastMatSkinned = !( m_elementsPointers[m_arrayPos-1]->getMatrixPaleteSize() >1 );
	int m_lastMatUseBlending = m_elementsPointers[m_arrayPos-1]->getMaterial()->getTransparency() == 0xFF;

	GVERTEXFORMAT lastVF = GVF_NUM;
	D3DPRIMITIVETYPE pt;
	
	m_oldIB = 0;
	m_oldVB = 0;
	m_oldTextures[0] = m_oldTextures[1] = m_oldTextures[2] = m_oldTextures[3] = 0;
	m_oldTextures[4] = m_oldTextures[5] = m_oldTextures[6] = m_oldTextures[7] = 0;

	while (this->popBack(&pElement))
	{
		pRenderable = pElement->getRenderable();
		pMaterial = pElement->getMaterial();

	restart:

		//run batch if available
		if(  (pRenderable->getBatchIBufferSize() != 0) && !batch_begin )
		{
			pSrcIB = (LPDIRECT3DINDEXBUFFER9)pRenderable->getIBuffer();
			pDestBatchIB = (LPDIRECT3DINDEXBUFFER9)pRenderable->getBatchIBuffer();

			HRESULT hr1 = pSrcIB->Lock(0, pRenderable->getIBufferSize(), (void**)&batch_pSrcBuffData, D3DLOCK_READONLY);

			batch_pFirstElement = pElement;

			m_tmpIndexesNum = 0;

			if (SUCCEEDED(hr1))// && SUCCEEDED(hr2))
			{
				batch_primCount = 0;
				batch_begin = true;

				memcpy(&m_tmpIndexes[m_tmpIndexesNum], &batch_pSrcBuffData[pElement->getStartIndex()],
					pElement->getPrimitiveCount() * 3 * sizeof(short));

				m_tmpIndexesNum += pElement->getPrimitiveCount() * 3;
				batch_primCount += pElement->getPrimitiveCount();
			}
			else //неудалось залочить 2 буффера
			{
				if (SUCCEEDED(hr1))
					pSrcIB->Unlock();
			}
		}
		// add data to batch
		else if( batch_begin && 
			( batch_pFirstElement->getMatrixPalete() == matPalete) &&
			( pRenderable->getId() == m_lastRenderable) &&
			( pMaterial->getId() == m_lastMaterial ) )
		{
			memcpy( &m_tmpIndexes[m_tmpIndexesNum], &batch_pSrcBuffData[pElement->getStartIndex()],
				pElement->getPrimitiveCount() * 3 * sizeof(short) );

			m_tmpIndexesNum += pElement->getPrimitiveCount() * 3;
			batch_primCount += pElement->getPrimitiveCount();
			continue;
		}

		else if( batch_begin && (
			(batch_pFirstElement->getMatrixPalete() != matPalete) ||
			(pRenderable->getId() != m_lastRenderable) ||
			(pMaterial->getId() != m_lastMaterial ))) //rasterize batch
		{
			HRESULT hr1 = pSrcIB->Unlock();
			HRESULT hr2 = pDestBatchIB->Lock(0, m_tmpIndexesNum*sizeof(short), (void**)&batch_pDestBuffData, D3DLOCK_DISCARD);

			memcpy( batch_pDestBuffData, m_tmpIndexes, m_tmpIndexesNum * sizeof(short) );
			hr2 = pDestBatchIB->Unlock();
			_setIB(pDestBatchIB, pDevice);

			pDevice->DrawIndexedPrimitive( pt, 0, 0, batch_pFirstElement->getVertexesNum(),
				0, batch_primCount );

			m_tmpIndexesNum = 0;
			batch_primCount = 0;
			batch_begin = false;

			goto restart;
		}

		//set transforms
		matPalete = pElement->getMatrixPalete();
		if (pElement->getMatrixPaleteSize() > 1) //skinning
		{
			skinBoneGroup = pElement->getSkinBoneGroup();
			auto rit = skinBoneGroup->remappedBones.begin();


			_setRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, true, pDevice );
			_setRenderState( D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS, pDevice );
			_setTransform( D3DTS_VIEW, &mView, pDevice );
			_setTransform( D3DTS_PROJECTION, &mProj, pDevice );
			_setTransform(D3DTS_TEXTURE0, &mId, pDevice);
			_setTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2, pDevice);


			while (rit != skinBoneGroup->remappedBones.end())
			{
				_setTransform(D3DTS_WORLDMATRIX(rit->second), &matPalete[rit->first], pDevice);
				rit++;
			}
		}
		else if ( pElement->getRenderable()->getGroup() == GRESGROUP_SKYBOX ) 
		{
			_setRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, false, pDevice );
			_setRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE, pDevice );
			_setTransform( D3DTS_WORLD, &mId, pDevice );
			_setTransform( D3DTS_VIEW, &mId, pDevice);
			_setTransform( D3DTS_PROJECTION, &mId, pDevice);

			_setTransform( D3DTS_TEXTURE0, &matPalete[0], pDevice );
			_setTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3, pDevice );

		}
		else
		{
			_setRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, false, pDevice);
			_setRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE, pDevice);
			_setTransform(D3DTS_VIEW, &mView, pDevice);
			_setTransform(D3DTS_PROJECTION, &mProj, pDevice);
			_setTransform(D3DTS_WORLDMATRIX(0), &matPalete[0], pDevice);
			_setTransform(D3DTS_TEXTURE0, &mId, pDevice);
			_setTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2, pDevice);
		}

		//render it!
		if( pRenderable )
		{
			switch (pRenderable->getPrimitiveType())
			{
			case GPT_POINTLIST:
				pt = D3DPT_POINTLIST;
				break;
			case GPT_LINELIST:
				pt = D3DPT_LINELIST;
				break;
			case GPT_TRIANGLELIST:
				pt = D3DPT_TRIANGLELIST;
				break;
			default:
				pt = D3DPT_FORCE_DWORD; // ??
			}

			if (pRenderable->getId() != m_lastRenderable)
			{
				_setVB((LPDIRECT3DVERTEXBUFFER9)pRenderable->getVBuffer(), pRenderable->getVertexStride(),
					getFVF(pRenderable->getVertexFormat()), pDevice);

				if ( (pRenderable->getIBuffer() != 0)  && !batch_begin ) //draw indexed
				{		
					_setIB((LPDIRECT3DINDEXBUFFER9)pRenderable->getIBuffer(), pDevice);
				}
			
				m_lastRenderable = pRenderable->getId();
			}

			//setup material
			if (pMaterial)
			{
				_setRenderState( D3DRS_LIGHTING, pMaterial->getLightingEnable(), pDevice );
				_setRenderState(D3DRS_ZENABLE, pMaterial->getZEnable(), pDevice);
				_setRenderState( D3DRS_ZWRITEENABLE, pMaterial->getZWriteEnable(), pDevice );

				unsigned char transpByte = m_elementsPointers[m_arrayPos]->getMaterial()->getTransparency();

				if (pMaterial->getId() != m_lastMaterial)
				{
					m_lastMaterial = pMaterial->getId();

					unsigned char samplers = pMaterial->getTexturesNum() < 8 ? pMaterial->getTexturesNum() + 1 : 8;

					for (unsigned char i = 0; i < samplers; i++)
					{
						pTex = pMaterial->getTexture(i);
						if (pTex)
						{
							_setTexture(i, (LPDIRECT3DBASETEXTURE9)pTex->getTexture(), pDevice);
							_setTextureStageState(i, D3DTSS_COLOROP, D3DTOP_MODULATE, pDevice);

						}
						else
						{
							_setTexture(i, 0, pDevice);
							_setTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE, pDevice);

						}
					}

					if ( transpByte == 0xFF)
					{
						_setRenderState(D3DRS_ALPHABLENDENABLE, false, pDevice);
						_setRenderState(D3DRS_ZWRITEENABLE, true, pDevice);					}
					else
					{
						DWORD blendFactor = transpByte | transpByte << 8 | transpByte << 16 | transpByte << 24;
						_setRenderState(D3DRS_BLENDFACTOR, blendFactor, pDevice);
						_setRenderState(D3DRS_ALPHABLENDENABLE, true, pDevice);
						_setRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR, pDevice);
						_setRenderState(D3DRS_DESTBLEND, D3DBLEND_INVBLENDFACTOR, pDevice);
						_setRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD, pDevice);
						_setRenderState(D3DRS_ZWRITEENABLE, false, pDevice);
					}
				}
			}
			else
			{
				for (unsigned char i = 0; i < 8; i++)
				{
					_setTexture(i, 0, pDevice);
				}
			}

			if (!batch_begin)
			{
				if (pRenderable->getIBuffer() != 0) //draw indexed
				{
					pDevice->DrawIndexedPrimitive(pt, 0, 0, pElement->getVertexesNum(),
						pElement->getStartIndex(), pElement->getPrimitiveCount());
				}
				else // draw noindexed
				{
					pDevice->DrawPrimitive(pt, pElement->getStartIndex(), pElement->getPrimitiveCount());
				}
			}
		}
	}
	

	if ( batch_begin ) //rasterize batch
	{

		HRESULT hr1 = pSrcIB->Unlock();
		HRESULT hr2 = pDestBatchIB->Lock(0, m_tmpIndexesNum * sizeof(short), (void**)&batch_pDestBuffData, D3DLOCK_DISCARD);

		memcpy(batch_pDestBuffData, m_tmpIndexes, m_tmpIndexesNum * sizeof(short));
		hr2 = pDestBatchIB->Unlock();
		_setIB(pDestBatchIB, pDevice );

		pDevice->DrawIndexedPrimitive(pt, 0, 0, batch_pFirstElement->getVertexesNum(),
			0, batch_primCount);
	}
}

void gRenderQueue::_debugOutSorted( const char* fname )
{
	FILE* f = 0;
	errno_t err = fopen_s(&f, fname, "wt");
	if (err)
		return;

	for (unsigned int i = 0; i < m_arrayPos; i++)
	{
		if (m_elementsPointers[i]->getMaterial() != 0)
		{
			fprintf(f, "key: %lli mat(%i): %s res(%i): %s dist:%i  start:%i num:%i batchBufferSize:%i renderamt:%i\n",
				m_elementsPointers[i]->getKey(), m_elementsPointers[i]->getMaterial()->getId(), m_elementsPointers[i]->getMaterial()->getName(), m_elementsPointers[i]->getRenderable()->getId(),
				m_elementsPointers[i]->getRenderable()->getResourceName(), m_elementsPointers[i]->getDistance(),
				m_elementsPointers[i]->getStartIndex(), m_elementsPointers[i]->getPrimitiveCount(), m_elementsPointers[i]->getRenderable()->getBatchIBufferSize(),
				m_elementsPointers[i]->getMaterial()->getTransparency());
		}
		else
		{
			fprintf(f, "key: %lli mat: 0 res(%i): %s dist:%i  start:%i num:%i batchBufferSize:%i\n",
				m_elementsPointers[i]->getKey(), m_elementsPointers[i]->getRenderable()->getId(),
				m_elementsPointers[i]->getRenderable()->getResourceName(), m_elementsPointers[i]->getDistance(),
				m_elementsPointers[i]->getStartIndex(), m_elementsPointers[i]->getPrimitiveCount(), m_elementsPointers[i]->getRenderable()->getBatchIBufferSize());
		}
	}
	fclose(f);
}

void gRenderQueue::_debugOutUnsorted(const char* fname)
{
	FILE* f = 0;
	errno_t err = fopen_s(&f, fname, "wt");
	if (err)
		return;

	for (unsigned int i = 0; i < m_arrayPos; i++)
	{
		if (m_elements[i].getMaterial() != 0)
		{
			fprintf(f, "key: %lli mat(%i): %s res(%i): %s dist:%i  start:%i num:%i\n",
				m_elements[i].getKey(), m_elements[i].getMaterial()->getId(), m_elements[i].getMaterial()->getName(), m_elements[i].getRenderable()->getId(),
				m_elements[i].getRenderable()->getResourceName(), m_elements[i].getDistance(), 
				m_elements[i].getStartIndex(), m_elements[i].getPrimitiveCount() );
		}
		else
		{
			fprintf(f, "key: %lli mat: 0 res(%i): %s dist:%i  start:%i num:%i\n",
				m_elements[i].getKey(), m_elements[i].getRenderable()->getId(),
				m_elements[i].getRenderable()->getResourceName(), m_elements[i].getDistance(),
				m_elements[i].getStartIndex(), m_elements[i].getPrimitiveCount());
		}
	}
	fclose(f);	
}

void gRenderQueue::_setTransform(DWORD transform, const D3DXMATRIX* matrix, IDirect3DDevice9* pDevice)
{
	auto it = (m_Transform).find(transform);
	if (it == m_Transform.end())
	{
		pDevice->SetTransform((D3DTRANSFORMSTATETYPE)transform, matrix );
		m_Transform[transform] = matrix;
	}
	else
	{
		if ( it->second != matrix )
		{
			pDevice->SetTransform( (D3DTRANSFORMSTATETYPE)transform, matrix );
			it->second = matrix;
		}
	}
}

void gRenderQueue::_setTextureStageState(unsigned char level, DWORD state, DWORD value, IDirect3DDevice9* pDevice)
{
	auto it = (m_TSS[level]).find(state);
	if (it == m_TSS[level].end())
	{
		pDevice->SetTextureStageState(level, (D3DTEXTURESTAGESTATETYPE)state, value);
		m_TSS[level][state] = value;
	}
	else
	{
		if (it->second != value)
		{
			pDevice->SetTextureStageState( level, (D3DTEXTURESTAGESTATETYPE)state, value);
			it->second = value;
		}
	}
}

void gRenderQueue::_setSamplerState( unsigned char level, DWORD state, DWORD value, IDirect3DDevice9* pDevice )
{
	auto it = (m_SS[level]).find(state);
	if (it == m_SS[level].end())
	{
		pDevice->SetSamplerState(level, (D3DSAMPLERSTATETYPE)state, value);
		m_SS[level][state] = value;
	}
	else
	{
		if (it->second != value)
		{
			pDevice->SetSamplerState(level, (D3DSAMPLERSTATETYPE)state, value);
			it->second = value;
		}
	}
}

void gRenderQueue::_setRenderState(DWORD state, DWORD value, IDirect3DDevice9* pDevice )
{
	auto it = m_RS.find(state);
	if (it == m_RS.end())
	{
		pDevice->SetRenderState((D3DRENDERSTATETYPE)state, value);
		m_RS[state] = value;
	}
	else
	{
		if (it->second != value)
		{
			pDevice->SetRenderState((D3DRENDERSTATETYPE)state, value);
			it->second = value;
		}
	}
}

void gRenderQueue::_forceSetRenderState(DWORD state, DWORD value, IDirect3DDevice9* pDevice)
{
	auto it = m_RS.find(state);
	if (it == m_RS.end())
	{
		m_RS[state] = value;
	}
	else
		it->second = value;
		
	pDevice->SetRenderState((D3DRENDERSTATETYPE)state, value);
}

void gRenderQueue::_setTexture( unsigned char level, IDirect3DBaseTexture9* tex, IDirect3DDevice9* pDevice )
{
	if (m_oldTextures[level] != tex)
	{
		pDevice->SetTexture(level, tex);
		m_oldTextures[level] = tex;
	}
}

void gRenderQueue::_setIB(IDirect3DIndexBuffer9* pIB, IDirect3DDevice9* pDevice)
{
	if ( pIB && (m_oldIB!=pIB) )
	{
		pDevice->SetIndices(pIB);
		m_oldIB = pIB;
	}
}

void gRenderQueue::_setVB( IDirect3DVertexBuffer9* pVB, unsigned char stride, DWORD fvf, IDirect3DDevice9* pDevice )
{
	if (pVB && (m_oldVB != pVB))
	{
		pDevice->SetStreamSource( 0, pVB, 0, stride );
		pDevice->SetFVF(fvf);
		m_oldVB = pVB;
	}
}

