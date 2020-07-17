#include "Materials.h"
#include "Resources.h"

//-----------------------------------------------
//
//	CLASS: gMaterialFactory
//-----------------------------------------------

gMaterialFactory::gMaterialFactory()
{
	m_idCounter = 0;
}

gMaterialFactory::~gMaterialFactory()
{

}

//after create Material always free mem by mat->release()
gMaterial* gMaterialFactory::createMaterial(const char* name)
{
	if (!name)
		return (gMaterial*)0;
	auto it = m_pMaterialsMap.find(name);
	if (it != m_pMaterialsMap.end())
		return 0;

	gMaterial* material = new gMaterial(this, name, m_idCounter++);
	m_pMaterialsMap[name] = material;
	material->addRef();
	return material;
}

gMaterial* gMaterialFactory::getMaterial( const char* name ) const
{
	if (!name)
		return (gMaterial*)0;
	auto it = m_pMaterialsMap.find(name);
	if (it != m_pMaterialsMap.end())
	{
		//if (it->second) 
		//	it->second->addRef();
		return it->second;
	}
	else
		return 0;
}

bool gMaterialFactory::destroyMaterial( const char* name )
{
	auto it = m_pMaterialsMap.find(name);
	if (it != m_pMaterialsMap.end())
	{
		if (it->second)
		{
			delete it->second;
		}
		m_pMaterialsMap.erase(it);
		return true;
	}
	return false;
}

void gMaterialFactory::destroyAllMaterials()
{
	auto it = m_pMaterialsMap.begin();
	while (it != m_pMaterialsMap.end())
	{
		if( it->second )
			delete it->second;
		it++;
	}
	m_pMaterialsMap.clear();
}

//-----------------------------------------------
//
//	CLASS: gMaterial
//-----------------------------------------------

gMaterial::gMaterial( gMaterialFactory* factory, const char* name, unsigned short id ) : gReferenceCounter()
{
	m_factory = factory;

	m_diffuse = 0xFFFFFFFF;
	m_specular = 0xFFFFFFFF;
	m_emissive = 0xFFFFFFFF;
	m_specularPower = 1.f;
	m_textures[0] = 0; m_textures[1] = 0; m_textures[2] = 0; m_textures[3] = 0;
	m_textures[4] = 0; m_textures[5] = 0; m_textures[6] = 0; m_textures[7] = 0;

	m_transparent = false;

	m_pMaterialId = id;

	if (name)
	{
		size_t lenght = strlen(name);
		m_name = new char[ lenght + 1];
		memcpy( m_name, name, lenght );
		m_name[lenght] = 0;
	}
	else
		throw("Material name null pointer!");
}

gMaterial::~gMaterial()
{
	if (m_name)
		delete[] m_name;

	for (unsigned char i = 0; i < 8; i++)
		if (m_textures[i]) m_textures[i]->release();
}

void gMaterial::release()
{
	m_refCounter--;

	if (m_refCounter == 0)
		m_factory->destroyMaterial(m_name);
}

void gMaterial::setDiffuse(GCOLOR color)
{
	m_diffuse = color;
}

void gMaterial::setSpecular(GCOLOR color)
{
	m_specular = color;
}

void gMaterial::setEmissive(GCOLOR color)
{
	m_emissive = color;
}

GCOLOR gMaterial::getDiffuse() const
{
	return m_diffuse;
}

GCOLOR gMaterial::getSpecular() const
{
	return m_specular;
}

GCOLOR gMaterial::getEmissive() const
{
	return m_emissive;
}

void gMaterial::setSpecularPower(float power)
{
	m_specularPower = power;
}

float gMaterial::getSpecularPower() const
{
	return m_specularPower;
}

gResource2DTexture* gMaterial::getTexture(unsigned char level)
{
	//if (level > 7)
	//	return;

	// (m_textures[level])
	//	m_textures[level]->addRef();

	return m_textures[level];
}

void gMaterial::setTexture(unsigned char level, gResource2DTexture* texture)
{
	//if (level > 7)
	//	return;


	if (m_textures[level])
		m_textures[level]->release();

	m_textures[level] = texture;
	if (m_textures[level])
		m_textures[level]->addRef();
}

unsigned short gMaterial::getId() const
{
	return m_pMaterialId;
}

const char* gMaterial::getName() const
{
	return m_name;
}


bool gMaterial::isTransparent() const
{
	return m_transparent;
}

void gMaterial::setTransparent( bool isTransparent )
{
	m_transparent = isTransparent;
}
