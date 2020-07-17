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
	unsigned __int64 alphaMask = 0;

	if (m_pMaterial)
	{
		m_key = m_pMaterial->getId();
		if( m_pMaterial->isTransparent() )
			alphaMask = ((unsigned __int64)1 << 60);
	}
	m_key = m_key << 10; // 1024 types of materials max - 10bits

	m_key |= m_pRenderable->getId(); 
	m_key = m_key << 10; // 1024 types of renderables 10bits

	m_key |= m_distance; //16 bit of dist 
	//m_key = m_key << 16;

	//alphablend bit
	m_key |= alphaMask;
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
	//gRenderElement** ppE1 = (gRenderElement**)i;
	//gRenderElement** ppE2 = (gRenderElement**)j;

	__int64 x = (*((gRenderElement**)i))->getKey();
	__int64 y = (*((gRenderElement**)j))->getKey();

	if (x > y) return 1; // TODO: ������� ����� � ����
	else if (x == y) return 0;
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
	gRenderElement* pElement = 0;
	const gMaterial* pMaterial = 0;
	const gResource2DTexture* pTex = 0;
	const gRenderable* pRenderable = 0;
	const D3DXMATRIX* matPalete = 0;
	const gSkinBoneGroup* skinBoneGroup = 0;

	bool isSkinning = !( m_elementsPointers[m_arrayPos]->getMatrixPaleteSize() >1 );

	int lastRenderable = -1;
	int lastMaterial = -1;
	GVERTEXFORMAT lastVF = GVF_NUM;
	

	while (this->popBack(&pElement))
	{
		//set transforms
		matPalete = pElement->getMatrixPalete();
		if (pElement->getMatrixPaleteSize() > 1) //skinning
		{
			if (!isSkinning)
			{
				pDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE);
				pDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS);
			}
			isSkinning = true;

			skinBoneGroup = pElement->getSkinBoneGroup();
			auto rit = skinBoneGroup->remappedBones.begin();
			while (rit != skinBoneGroup->remappedBones.end())
			{
				pDevice->SetTransform(D3DTS_WORLDMATRIX(rit->second), &matPalete[rit->first]);
				rit++;
			}
		}
		else
		{
			if (isSkinning)
			{
				pDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
				pDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, false);
			}
			isSkinning = false;
			pDevice->SetTransform(D3DTS_WORLDMATRIX(0), matPalete);
		}

		//set mat of element and render it

		//setup material
		pMaterial = pElement->getMaterial();
		if (pMaterial)
		{
			if (pMaterial->getId() != lastMaterial)
			{
				for (unsigned char i = 0; i < 8; i++)
				{
					pTex = pMaterial->getTexture(i);
					if (pTex) pDevice->SetTexture(i, pTex->getTexture());
				}
				lastMaterial = pMaterial->getId();
			}
		}
		else
		{
			for (unsigned char i = 0; i < 8; i++)
			{
				pDevice->SetTexture(i, 0);
			}
		}

		//render it!
		pRenderable = pElement->getRenderable();
		if( pRenderable )
		{
			D3DPRIMITIVETYPE pt;
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

			if (pRenderable->getId() != lastRenderable)
			{
				//FFP only
				if (lastVF != pRenderable->getVertexFormat())
				{
					pDevice->SetFVF(getFVF(pRenderable->getVertexFormat()));
					lastVF = pRenderable->getVertexFormat();
				}

				pDevice->SetStreamSource(0, (LPDIRECT3DVERTEXBUFFER9)pRenderable->getVBuffer(),
					0, pRenderable->getVertexStride() );

				if (pRenderable->getIBuffer() != 0) //draw indexed
				{
					pDevice->SetIndices((LPDIRECT3DINDEXBUFFER9)pRenderable->getIBuffer());
				}
				lastRenderable = pRenderable->getId();
			}

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

void gRenderQueue::_debugOut( const char* fname )
{
	FILE* f = 0;
	errno_t err = fopen_s(&f, fname, "wt");
	if (err)
		return;

	for (unsigned int i = 0; i < m_arrayPos; i++)
	{
		if (m_elementsPointers[i]->getMaterial() != 0)
		{
			fprintf(f, "key: %lli mat(%i): %s res(%i): %s dist:%i\n",
				m_elementsPointers[i]->getKey(), m_elementsPointers[i]->getMaterial()->getId(), m_elementsPointers[i]->getMaterial()->getName(), m_elementsPointers[i]->getRenderable()->getId(),
				m_elementsPointers[i]->getRenderable()->getResourceName(), m_elementsPointers[i]->getDistance());
		}
		else
		{
			fprintf(f, "key: %lli mat: 0 res(%i): %s dist:%i\n",
				m_elementsPointers[i]->getKey(), m_elementsPointers[i]->getRenderable()->getId(),
				m_elementsPointers[i]->getRenderable()->getResourceName(), m_elementsPointers[i]->getDistance());
		}
	}
	fclose(f);
}