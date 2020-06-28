#pragma once

#ifndef _BMPFILE_H_
#define _BMPFILE_H_

class gFile;
struct tagRGBTRIPLE;

//используем в работе только 32бит режим
class gBMPFile
{
public:
    gBMPFile();
    ~gBMPFile();

    void createBitMap( unsigned int width, unsigned int height );
    bool loadFromFile( gFile* file, bool useVerticalFlip = true );
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
};

#endif


