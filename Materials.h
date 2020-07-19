#pragma once

#ifndef _MATERIALS_H_
#define _MATERIALS_H_

#include "RefCounter.h"
#include <map>
#include <string>

class gResource2DTexture;
typedef unsigned __int32 GCOLOR;

class gMaterial;

class gMaterialFactory
{
public:
	gMaterialFactory();
	~gMaterialFactory();

	gMaterial* cloneMaterial(gMaterial* src, const char* cloneName);
	gMaterial* createMaterial( const char* name );
	gMaterial* getMaterial( const char* name ) const;
	bool destroyMaterial(const char* name );

	void destroyAllMaterials();

protected:
	std::map < std::string, gMaterial* > m_pMaterialsMap;
	unsigned int m_idCounter;
};

class gMaterial : public gReferenceCounter
{
public:

	gMaterial( gMaterialFactory* factory, const char* name, unsigned short id );
	gMaterial( gMaterial* other, gMaterialFactory* factory, const char* name, unsigned short id );

	~gMaterial();

	void release();

	gMaterial* cloneMaterial(const char* cloneName);

	void setDiffuse(GCOLOR color);
	void setSpecular(GCOLOR color);
	void setEmissive(GCOLOR color);

	GCOLOR getDiffuse() const;
	GCOLOR getSpecular() const;
	GCOLOR getEmissive() const;

	void setSpecularPower(float power);
	float getSpecularPower() const;

	unsigned char getTexturesNum() const;
	gResource2DTexture* getTexture(unsigned char level) const;
	void setTexture(unsigned char level, gResource2DTexture* texture);
	
	unsigned short getId() const;

	const char* getName() const;

	unsigned char getTransparency() const;
	void setTransparency( unsigned char transparency );

protected:

	gMaterial(gMaterial&) {}

	char* m_name;
	gMaterialFactory* m_factory;
	unsigned char m_transparency;

	unsigned short m_pMaterialId; // only 65536 materials

	GCOLOR m_diffuse;
	GCOLOR m_specular;
	GCOLOR m_emissive;
	float m_specularPower;
	gResource2DTexture* m_textures[8];
};

#endif