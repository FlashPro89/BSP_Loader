#pragma once

#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_

#include <cstdio>
#include <string>

enum gFileSeek
{
	GFS_SET = 0,
	GFS_CURRENT = 1, 
	GFS_END = 2
};

class gFile
{
public:
	virtual ~gFile() {};

	const bool isBinary();
	const bool isWriteable();
	const char* getFileName(); 

	virtual const unsigned int getFileSize() = 0;

	virtual bool seek( gFileSeek mode, unsigned int position = 0 ) = 0;
	virtual const unsigned int tell() = 0;

	virtual const size_t read( void* dst, size_t size ) = 0;
	virtual size_t write( void* src, size_t size ) = 0;

	virtual const bool gets( char* dst, size_t buffsz ) = 0;
	virtual bool puts( const char* src ) = 0;

	virtual int printf(const char* fmt, ...) = 0;
	virtual const int scanf(const char* fmt, ...) = 0;

protected:
	gFile() {};
	gFile(gFile&) {};

	char* m_filename;
	bool m_writeable;
	bool m_binary;
};

class gFileImpl : public gFile
{
public:
	gFileImpl( const char* filename, bool writeable, bool binary = false, void* data = 0, unsigned int datasize = 0 );
	~gFileImpl();

	const unsigned int getFileSize();

	bool seek( gFileSeek mode, unsigned int position = 0 );
	const unsigned int tell();

	const size_t read( void* dst, size_t size );
	size_t write( void* src, size_t size );

	const bool gets( char* dst, size_t buffsz );
	bool puts( const char* src );

	int printf( const char* fmt, ... );
	const int scanf(const char* fmt, ...);

protected:
	// from file
	FILE* m_file;

	//from memory
	size_t m_memFileSize;
	size_t m_memCurrentPos;
	void* m_memFileData;
};

class gFileSystem
{
public:
	gFileSystem();
	~gFileSystem();

	gFile* openFile( const char* filename, bool writeable, bool binary );
	void closeFile( gFile* file );

	size_t getFileSize( const char* filename ) const;
	bool isFileExist(const char* filename) const;

	bool OpenFileDialogBox(char* fname, unsigned short maxLenght, 
		const char* filter = 0, unsigned short filterLenght = 0, 
		const char* title = 0, const char* defaultName = 0 );
	bool SaveFileDialogBox(char* fname, unsigned short maxLenght, const char* filter = 0, const char* title = 0, const char* defaultName = 0);

};

#endif

