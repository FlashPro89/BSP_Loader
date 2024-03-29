#include "FileSystem.h"
#include <stdarg.h>
#include <Windows.h>

#define STRING_BUFFER_SIZE 4096
char sBuffer[STRING_BUFFER_SIZE] = "";

//-----------------------------------------------
//
//	CLASS: gFile
//-----------------------------------------------

const bool gFile::isBinary()
{
	return m_binary;
}

const bool gFile::isWriteable()
{
	return m_writeable;
}

const char* gFile::getFileName()
{
	return m_filename;
}

//-----------------------------------------------
//
//	CLASS: gFileImpl
//-----------------------------------------------


gFileImpl::gFileImpl( const char* filename, bool writeable, bool binary, void* data, unsigned int datasize )
{
	m_binary = binary;
	m_writeable = writeable;
	
	if (filename)
	{
		m_filename = new char[strlen(filename) + 1];
		strcpy_s( m_filename, strlen(filename) + 1, filename );
	}
	else
		m_filename = 0;

	m_memFileSize = 0;
	m_memFileData = 0;
	m_memCurrentPos = 0;

	// from file
	if (filename != 0)
	{
		unsigned char p = 1;
		char mode[3];

		writeable ? mode[0] = 'w' : mode[0] = 'r';
		binary ? mode[1] = 'b' : mode[1] = 't';
		mode[2] = 0;

		errno_t err = fopen_s(&m_file, filename, mode);
		if (err) throw("Cannot open file!");
	}
	else
	{
		//from memory
		m_memFileSize = datasize;
		m_memFileData = data;
	}
}

gFileImpl::~gFileImpl()
{
	if (m_filename)
		delete[] m_filename;

	if (m_file)
		fclose(m_file);
}

const unsigned int gFileImpl::getFileSize() const
{
	if( m_memFileSize )
		return m_memFileSize;
	else
	{
		unsigned int oldPos = ftell(m_file);
		fseek(m_file, 0, SEEK_END);
		unsigned int sz = ftell(m_file);
		fseek(m_file, oldPos, SEEK_SET);
		return sz;
	}
}

bool gFileImpl::seek(gFileSeek mode, unsigned int position)
{
	if (m_memFileSize) //memory file
	{
		if (position < m_memFileSize)
		{
			m_memCurrentPos = position;
			return true;
		}
		else
			return false;
	}
	else
	{
		return !fseek( m_file, position, SEEK_SET );
	}
}

const unsigned int gFileImpl::tell() const
{
	if (m_memFileSize) //memory file
	{
		return m_memCurrentPos;
	}
	else
		return ftell( m_file );
}

const size_t gFileImpl::read(void* dst, size_t size) const
{
	if (!m_binary)
		return 0;

	if (m_memFileSize) //memory file
	{
		int readSz = size;
		if ((size + m_memCurrentPos) > m_memFileSize)
			readSz = m_memFileSize - m_memCurrentPos;
		memcpy_s( dst, readSz, (void*)((char*)m_memFileData + m_memCurrentPos), readSz);
		m_memCurrentPos += readSz;
		return readSz;
	}
	else
		return fread_s( dst, size, 1, size, m_file );
}

size_t gFileImpl::write(void* src, size_t size)
{
	if (!m_binary || !m_writeable)
		return 0;

	if (m_memFileSize) //memory file
	{
		int writeSz = size;
		if ((size + m_memCurrentPos) > m_memFileSize)
			writeSz = m_memFileSize - m_memCurrentPos;
		memcpy_s((void*)( (char*)m_memFileData + m_memCurrentPos ), writeSz, src, writeSz);
		m_memCurrentPos += writeSz;
		return writeSz;
	}
	else
		return fwrite( src, 1, size, m_file);
}


const bool gFileImpl::gets(char* dst, size_t buffsz) const
{
	if (m_binary && !m_memFileSize)
		return false;

	if (m_memFileSize) //memory file
	{
		if (m_memCurrentPos >= m_memFileSize)
			return false;

		char* ptr = (char*)m_memFileData + m_memCurrentPos;
		size_t sl = strchr(ptr, '\n') - ptr + 1;

		if ((sl + m_memCurrentPos) > m_memFileSize)
			sl = m_memFileSize - m_memCurrentPos;

		if (sl > buffsz)
			sl = buffsz-1;
		
		if( dst )
			strncpy_s( dst, buffsz, ptr, sl);

		m_memCurrentPos += sl;

		return true;
	}
	else
	{
		if (dst)
		{
			return 0 != fgets(dst, buffsz, m_file);
		}
		else
			return false;
	}
}

bool gFileImpl::puts( const char* src )
{
	if (m_binary || !m_writeable)
		return 0;

	if (m_memFileSize) //memory file
	{
		if (m_memCurrentPos >= m_memFileSize)
			return false;

		char* ptr = (char*)m_memFileData + m_memCurrentPos;
		size_t sl = strchr(src, '\n') - src + 1;

		if ((sl + m_memCurrentPos) > m_memFileSize)
			sl = m_memFileSize - m_memCurrentPos + 1;

		strncpy_s(ptr, m_memFileSize - m_memCurrentPos, src, sl);

		m_memCurrentPos += sl;
		return true;
	}
	else
	{
		return EOF != fputs( src, m_file );
	}
}

int gFileImpl::printf(const char* fmt, ...)
{
	if (m_binary || !m_writeable)
		return 0;

	int result = 0;
	va_list argList;
	__crt_va_start(argList, fmt);

	if (m_memFileSize) //memory file
	{
		result = _vsprintf_s_l(sBuffer, STRING_BUFFER_SIZE, fmt, NULL, argList);
		puts(sBuffer);
	}
	else
		result = _vfprintf_s_l(m_file, fmt, NULL, argList);

	__crt_va_end(argList);
	return result;
}

const int gFileImpl::scanf( const char* fmt, ...) const
{
	if (m_binary && !m_memFileSize)
		return 0;

	int result = 0;
	va_list argList;
	__crt_va_start(argList, fmt);

	if (m_memFileSize) //memory file
	{
		if (gets(sBuffer, STRING_BUFFER_SIZE))
			result = _vsscanf_s_l(sBuffer, fmt, NULL, argList);
	}
	else
		result = _vfscanf_s_l( m_file, fmt, NULL, argList );

	__crt_va_end(argList);
	return result;
}

char gFileImpl::getc( bool nostep ) const
{
	char t;
	if (m_memFileSize) //memory file
	{
		t = ((char*)m_memFileData)[m_memCurrentPos];
		if (!nostep)
			m_memCurrentPos++;
	}
	else
	{
		t = fgetc(m_file);
		if (nostep)
			fseek(m_file, -1, SEEK_CUR);
	}
	return t;
}

bool gFileImpl::eof() const
{
	if (m_memFileSize) //memory file
	{
		return ( m_memCurrentPos >= m_memFileSize );
	}
	else
	{
		return feof(m_file) != 0;
	}
}

//-----------------------------------------------
//
//	CLASS: gFilesSystem
//-----------------------------------------------

gFileSystem::gFileSystem()
{

}

gFileSystem::~gFileSystem()
{

}

gFile* gFileSystem::openFileInMemory( void* data, unsigned int datasize, bool writeable )
{
	return new gFileImpl( 0, writeable, true, data, datasize );
}

gFile* gFileSystem::openFile( const char* filename, bool writeable, bool binary )
{
	return new gFileImpl( filename, writeable, binary );
}

void gFileSystem::closeFile( gFile* file )
{
	if (file)
		delete ((gFileImpl*)file);
}

size_t gFileSystem::getFileSize( const char* filename ) const
{
	size_t sz = 0;
	FILE* f = 0;
	errno_t err = fopen_s( &f, filename, "rb" );
	if (err) 
		return sz;

	struct stat buff;
	fstat(_fileno(f), &buff);
	fclose(f);

	sz = buff.st_size;

	return sz;
}

bool gFileSystem::isFileExist( const char* filename ) const
{
	return getFileSize(filename) != 0;
} 


bool gFileSystem::OpenFileDialogBox(char* fname, unsigned short maxLenght,
	const char* filter, unsigned short filterLenght,
	const char* title, const char* defaultName)
{
	OPENFILENAME ofn = { 0 };

	char fileTitle[256];
	memset(fileTitle, 0, 256);
	if (title)
		strcpy_s(fileTitle, 256, title);

	char* fileFilter = new char[filterLenght+1];
	memset(fileFilter, 0, filterLenght);
	if (filter)
		memcpy_s(fileFilter, filterLenght+1, filter, filterLenght );

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;

	ofn.lpstrFile = fname;
	if (defaultName)
		strcpy_s(fname, maxLenght, defaultName);
	else
		*(ofn.lpstrFile) = 0;

	ofn.nMaxFile = maxLenght;
	ofn.lpstrFilter = fileFilter;
	
	if( filter != 0 )
		ofn.nFilterIndex = 1;

	ofn.lpstrFileTitle = fileTitle;
	*(ofn.lpstrFileTitle) = 0;
	ofn.nMaxFileTitle = 0;

	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

	char path[MAX_PATH];
	GetCurrentDirectory(sizeof(path), path); //HACK)
	GetOpenFileName(&ofn);
	SetCurrentDirectory(path);

	return true;
}

bool gFileSystem::SaveFileDialogBox( char* out, unsigned short maxLenght, const char* filter,
	const char* title, const char* defaultName )
{
	
	return true;
}
