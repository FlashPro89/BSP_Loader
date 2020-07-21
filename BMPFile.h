#pragma once

#ifndef _BMPFILE_H_
#define _BMPFILE_H_

class gFile;
struct tagRGBTRIPLE;

//используем в работе только 24бит режим
class gBMPFile
{
public:
    gBMPFile();
    ~gBMPFile();

    void createBitMap( unsigned int width, unsigned int height, unsigned char fillingByte = 0xFF );
    bool loadFromFile( gFile* file, bool useVerticalFlip = true );
    void loadFromMemory( void* src, unsigned int width, unsigned int height );
    bool saveToFile( gFile* file ) const;

    unsigned int getWidth() const;
    unsigned int getHeight() const;

    void verticalFlip();

    const tagRGBTRIPLE* getBitMap() const;
    bool overlapOther( const gBMPFile& other, unsigned int inPosX, unsigned int inPosY );

protected:

    gBMPFile(gBMPFile&);

    unsigned int m_width;
    unsigned int m_height;
    tagRGBTRIPLE* m_bitmap;
    bool m_loadedFromMem;
};

#endif


