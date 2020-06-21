#pragma once

#ifndef _UTIL_H_
#define _UTIL_H_

#define D3D9_SHADER_DEBUG_NO

#ifdef D3D9_SHADER_DEBUG
#define D3D_DEBUG_INFO
#endif

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

//vars
extern HWND hwnd;
extern LPDIRECT3D9 pD3D9;
extern LPDIRECT3DDEVICE9 pD3DDev9;

//prototypes

//window util
void wnd_create( const char *title, int w, int h );
void wnd_destroy();
void wnd_show();
void wnd_hide();
void wnd_update();
void wnd_mode( bool fs );
void wnd_setTitle( const char* title );

//d3d9
void d3d9_init();
void d3d9_destroy();

#endif