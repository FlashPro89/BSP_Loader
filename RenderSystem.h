#pragma once

#ifndef _RENDER_SYSTEM_H_
#define _RENDER_SYSTEM_H_

struct gVideoAdapterDesc
{

};

class gRenderSystem
{
public:
	virtual ~gRenderSystem() = 0;

	virtual unsigned char getAdapterNum() = 0;
	virtual const gVideoAdapterDesc& getAdapterDesc() = 0;

protected:
	gRenderSystem() {}
};

#endif