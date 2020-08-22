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
	auto it = m_pMaterialsMap.begin();
	while (it != m_pMaterialsMap.end())
	{
		if( it->second )
			delete it->second;
	}
	m_pMaterialsMap.clear();
}

gMaterial* gMaterialFactory::cloneMaterial(gMaterial* src, const char* cloneName)
{
	if (!cloneName)
		return (gMaterial*)0;

	auto it = m_pMaterialsMap.find(cloneName);
	if (it != m_pMaterialsMap.end())
		return (gMaterial*)0;

	gMaterial* material = new gMaterial( src, this, cloneName, m_idCounter++);
	m_pMaterialsMap[cloneName] = material;
	material->addRef();
	return material;
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

	m_transparency = 0xFF;

	m_pMaterialId = id;

	m_lightingEnable = true;

	if (name)
	{
		size_t lenght = strlen(name);
		m_name = new char[ lenght + 1];
		memcpy( m_name, name, lenght );
		m_name[lenght] = 0;
	}
	else
		throw("Material name null pointer!");

	m_zEnable = true;
	m_zWriteEnable = true;
	m_visibility = true;
}

gMaterial::gMaterial( gMaterial* other, gMaterialFactory* factory, const char* name, unsigned short id)
{
	m_factory = factory;

	m_diffuse = other->getDiffuse();
	m_specular = other->getSpecular();
	m_emissive = other->getEmissive();
	m_specularPower = other->getSpecularPower();

	setTexture(0, other->getTexture(0));
	setTexture(1, other->getTexture(1));
	setTexture(2, other->getTexture(2));
	setTexture(3, other->getTexture(3));
	setTexture(4, other->getTexture(4));
	setTexture(5, other->getTexture(5));
	setTexture(6, other->getTexture(6));
	setTexture(7, other->getTexture(7));

	m_transparency = other->getTransparency();
	m_lightingEnable = other->getLightingEnable();
	m_zEnable = other->getZEnable();
	m_zWriteEnable = other->getZWriteEnable();

	m_visibility = other->isVisible();

	m_pMaterialId = id;

	if (name)
	{
		size_t lenght = strlen(name);
		m_name = new char[lenght + 1];
		memcpy(m_name, name, lenght);
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

gMaterial* gMaterial::cloneMaterial(const char* cloneName)
{
	return m_factory->cloneMaterial(this, cloneName);
}

bool gMaterial::getLightingEnable() const
{
	return m_lightingEnable;
}

void gMaterial::setLightingEnable(bool enable)
{
	m_lightingEnable = enable;
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

unsigned char gMaterial::getTexturesNum() const
{
	for( unsigned char i = 7; i >= 0; i-- )
	{
		if (m_textures[i] != 0)
			return i + 1;
	}
	return 0;
}

gResourceTexture* gMaterial::getTexture(unsigned char level) const
{
	//if (level > 7)
	//	return;

	// (m_textures[level])
	//	m_textures[level]->addRef();

	return m_textures[level];
}

void gMaterial::setTexture(unsigned char level, gResourceTexture* texture)
{
	//if (level > 7)
	//	return;


	if (m_textures[level])
		m_textures[level]->release();

	m_textures[level] = texture;
	if (m_textures[level])
		m_textures[level]->addRef();
}

void gMaterial::setTextureStageState(unsigned char level, unsigned long state, unsigned long value)
{
	if (level >= 8) return;
	auto it = m_TSS[level].find(state);

	if (it != m_TSS[level].end())
	{
		it->second = value;
	}
	else
	{
		(m_TSS[level])[state] = value;
	}
}

void gMaterial::setSamplerState(unsigned char level, unsigned long state, unsigned long value)
{
	if (level >= 8) return;
	auto it = m_SS[level].find(state);

	if (it != m_SS[level].end())
	{
		it->second = value;
	}
	else
	{
		(m_SS[level])[state] = value;
	}
}

void gMaterial::setRenderState(unsigned long state, unsigned long value)
{
	auto it = m_RS.find(state);

	if (it != m_RS.end())
	{
		it->second = value;
	}
	else
	{
		m_RS[state] = value;
	}
}

bool gMaterial::getTextureStageState(unsigned long* outValue, unsigned char level, unsigned long state) const
{
	auto it = m_TSS[level].find(state);

	if (it != m_TSS[level].end())
	{
		*outValue = it->second;
		return true;
	}
	else
		return false;
}

bool gMaterial::getSamplerState(unsigned long* outValue, unsigned char level, unsigned long state) const
{
	auto it = m_SS[level].find(state);

	if (it != m_SS[level].end())
	{
		*outValue = it->second;
		return true;
	}
	else
		return false;
}

bool gMaterial::getRenderState(unsigned long* outValue, unsigned long state) const
{
	auto it = m_RS.find(state);

	if (it != m_RS.end())
	{
		*outValue = it->second;
		return true;
	}
	else
		return false;
}

const gMaterialStateMap& gMaterial::getTextureStageStateMap(unsigned char level) const
{
	return m_TSS[level];
}

const gMaterialStateMap& gMaterial::getSamplerStateMap(unsigned char level) const
{
	return m_SS[level];
}

const gMaterialStateMap& gMaterial::getRenderState() const
{
	return m_RS;
}

unsigned short gMaterial::getId() const
{
	return m_pMaterialId;
}

const char* gMaterial::getName() const
{
	return m_name;
}

unsigned char gMaterial::getTransparency() const
{
	return m_transparency;
}

void gMaterial::setTransparency(unsigned char transparency)
{
	m_transparency = transparency;
}

void gMaterial::setZWriteEnable(bool zwrite)
{
	m_zWriteEnable = zwrite;
}
bool gMaterial::getZWriteEnable() const
{
	return m_zWriteEnable;
}

void gMaterial::setZEnable(bool zenable)
{
	m_zEnable = zenable;
}

bool gMaterial::getZEnable() const
{
	return m_zEnable;
}

void gMaterial::setVisibility( bool visibility )
{
	m_visibility = visibility;
}

bool gMaterial::isVisible() const
{
	return m_visibility;
}
