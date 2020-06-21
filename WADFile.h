#pragma once

#ifndef _WADFILE_H_
#define _WADFILE_H_

#include <stdio.h>
#include <string>

typedef struct
{
	char		identification[4];		// should be WAD2/WAD3
	int			numlumps;
	int			infotableofs;
} WADHeader;

typedef struct
{
	char			name[16];
	int				width, height;
	unsigned int	data[4];			// variably sized
} WADPic;

struct WADLumpInfo_t
{
	int			filepos;
	int			disksize;
	int			size;					// uncompressed
	char		type;
	char		compression;
	char		pad1, pad2;
	char		name[16];				// must be null terminated

};

struct WADFile
{
	WADFile() { lumpInfo = 0; memset( &header, 0, sizeof(WADHeader) ); }
	~WADFile() { if (lumpInfo) delete[] lumpInfo; }

	std::string name;
	WADHeader header;
	WADLumpInfo_t* lumpInfo;
};

typedef struct
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
} icolor;

typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
} iwadcolor;

#define MAX_TEX_SIZE 256*256
#define MAX_IMG_DISK_SZ MAX_TEX_SIZE*4

int filelength(FILE* f);
size_t WADLoadLumpFromFile(const char* fname, void* dest, int offset, int size);
WADLumpInfo_t* WADLoadFromFile( WADHeader* dest, const char* fname );


#endif

