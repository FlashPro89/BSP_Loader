#include "WADFile.h"
#include <stdio.h>
#include <string.h>
#include <corecrt_malloc.h>

int filelength(FILE* f)
{
	int		pos;
	int		end;

	pos = ftell(f);
	fseek(f, 0, SEEK_END);
	end = ftell(f);
	fseek(f, pos, SEEK_SET);

	return end;
}

size_t WADLoadLumpFromFile(const char* fname, void* dest, int offset, int size)
{
	FILE* f = 0;

	errno_t err = fopen_s(&f, fname, "rb");
	if (err)
		return 0;

	fseek(f, offset, SEEK_SET);
	size_t sz = fread(dest, 1, size, f);

	fclose(f);

	return sz;
}

//загружаем заголовок и разметку
WADLumpInfo_t* WADLoadFromFile(WADHeader* dest, const char* fname ) // Clear memory after this func
{
	FILE* f = 0;

	errno_t err = fopen_s(&f,fname, "rb");
	if (err)
		return 0;

	//read head
	//dest = new WADHeader;
	size_t sz = fread(dest, sizeof(WADHeader), 1, f);
	if (sz != 1)
		return 0;

	if (strncmp(dest->identification, "WAD2", 4) &&
		strncmp(dest->identification, "WAD3", 4))
		throw("Invalid WAD header");
	fseek(f, 0, SEEK_SET);

	//read lumps offsets info
	fseek(f, dest->infotableofs, SEEK_SET);

	WADLumpInfo_t* wadlumps = new WADLumpInfo_t[ dest->numlumps ];
	

	sz = fread( wadlumps, sizeof(WADLumpInfo_t), dest->numlumps, f);
	if (sz != dest->numlumps)
	{
		delete[] wadlumps;
		fclose(f);
		return 0;
	}

	fclose(f);
	return wadlumps;
}