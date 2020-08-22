#pragma once

#ifndef _MATERIALS_H_
#define _MATERIALS_H_

#include "RefCounter.h"
#include <map>
#include <string>

class gResourceTexture;
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

typedef std::map< unsigned long, unsigned long > gMaterialStateMap;

class gMaterialStateIterator
{
public:
	gMaterialStateIterator();
	~gMaterialStateIterator();

protected:

};

class gMaterial : public gReferenceCounter
{
public:

	gMaterial(gMaterialFactory* factory, const char* name, unsigned short id);
	gMaterial(gMaterial* other, gMaterialFactory* factory, const char* name, unsigned short id);

	~gMaterial();

	void release();

	gMaterial* cloneMaterial(const char* cloneName);

	bool getLightingEnable() const;
	void setLightingEnable(bool enable);

	void setDiffuse(GCOLOR color);
	void setSpecular(GCOLOR color);
	void setEmissive(GCOLOR color);

	GCOLOR getDiffuse() const;
	GCOLOR getSpecular() const;
	GCOLOR getEmissive() const;

	void setSpecularPower(float power);
	float getSpecularPower() const;

	unsigned char getTexturesNum() const;
	gResourceTexture* getTexture(unsigned char level) const;
	void setTexture(unsigned char level, gResourceTexture* texture);

	void setTextureStageState(unsigned char level, unsigned long state, unsigned long value);
	void setSamplerState(unsigned char level, unsigned long state, unsigned long value);
	void setRenderState(unsigned long state, unsigned long value);

	bool getTextureStageState(unsigned long* outValue, unsigned char level, unsigned long state) const;
	bool getSamplerState(unsigned long* outValue, unsigned char level, unsigned long state) const;
	bool getRenderState(unsigned long* outValue, unsigned long state) const;

	const gMaterialStateMap& getTextureStageStateMap( unsigned char level ) const;
	const gMaterialStateMap& getSamplerStateMap( unsigned char level ) const;
	const gMaterialStateMap& getRenderState() const;

	unsigned short getId() const;
	const char* getName() const;

	unsigned char getTransparency() const;
	void setTransparency(unsigned char transparency);

	void setZWriteEnable(bool zwrite);
	bool getZWriteEnable() const;

	void setZEnable(bool zenable);
	bool getZEnable() const;

	void setVisibility( bool visibility );
	bool isVisible() const;

protected:

	gMaterial(gMaterial&) {}

	char* m_name;
	gMaterialFactory* m_factory;
	unsigned char m_transparency;

	bool m_zEnable;
	bool m_zWriteEnable;

	unsigned short m_pMaterialId; // only 65536 materials ??? now use 1024 materials (10bit)
	bool m_lightingEnable;
	GCOLOR m_diffuse;
	GCOLOR m_specular;
	GCOLOR m_emissive;
	float m_specularPower;
	bool m_visibility;

	gMaterialStateMap m_TSS[8];
	gMaterialStateMap m_SS[8];
	gMaterialStateMap m_RS;
	gResourceTexture* m_textures[8];
};

#endif