#include "BMPFile.h"
#include "FileSystem.h"
#include <Windows.h>
#include <string.h>

gBMPFile::gBMPFile()
{
    m_width = 0;
    m_height = 0;
    m_bitmap = 0;
}

gBMPFile::~gBMPFile()
{
    if (m_bitmap)
        delete[] m_bitmap;
}

//24bit
void gBMPFile::createBitMap(unsigned int width, unsigned int height)
{
    if (m_bitmap)
        delete[] m_bitmap;
    m_bitmap = new tagRGBTRIPLE[width * height];
    memset( m_bitmap, 0, width * height * sizeof(tagRGBTRIPLE) );
    m_width = width;
    m_height = height;
}

//load 8bit indexed bitmap and convert to 24bit or simply load 24bit
bool gBMPFile::loadFromFile( gFile* file, bool useVerticalFlip )
{
    tagBITMAPINFOHEADER inf;
    tagBITMAPFILEHEADER header;
    tagRGBQUAD palete[256];
    unsigned char* bitmap = 0;

    size_t sz = file->read(&header, sizeof(tagBITMAPFILEHEADER));
    if( !( sz== sizeof(tagBITMAPFILEHEADER) ) ||  ( header.bfType != 19778 ) )
        return false;
    
    sz = file->read(&inf, sizeof(tagBITMAPINFOHEADER));
    if (!(sz == sizeof(tagBITMAPINFOHEADER)))
        return false;

    if (inf.biBitCount = 8)
    {
        sz = file->read(palete, inf.biClrUsed * sizeof(tagRGBQUAD));
        //sz = file->read(palete, 0xFF * sizeof(tagRGBQUAD));

        m_width = inf.biWidth; m_height = inf.biHeight;
        createBitMap(m_width, m_height);

        bitmap = new unsigned char[m_width * m_height];
        file->seek(GFS_SET, header.bfOffBits);
        file->read(bitmap, m_width * m_height);

        unsigned int index;

        for (unsigned int x = 0; x < m_width; x++)
        {
            for (unsigned y = 0; y < m_height; y++)
            {
                index = x + y * m_width;
                m_bitmap[index].rgbtRed = palete[bitmap[index]].rgbRed;
                m_bitmap[index].rgbtGreen = palete[bitmap[index]].rgbGreen;
                m_bitmap[index].rgbtBlue = palete[bitmap[index]].rgbBlue;
            }
        }
        delete[] bitmap;
    }
    else if ( inf.biBitCount == 24 )
    {
        sz = file->read(m_bitmap, m_width * m_height * sizeof(tagRGBTRIPLE));
        if (sz != m_width * m_height * sizeof(tagRGBTRIPLE))
            return false;
    }
    else
        return false;

    //need vertical flip ?
    if (useVerticalFlip)
        verticalFlip();

    return true;
}

//save 24bit image
bool gBMPFile::saveToFile(gFile* file) const
{
    if ( (!m_bitmap) || (m_width == 0 )|| (m_height == 0) )
        return false;

    tagBITMAPINFOHEADER inf;
    tagBITMAPFILEHEADER header;

    header.bfOffBits = sizeof(tagBITMAPINFOHEADER) + sizeof(tagBITMAPFILEHEADER);
    header.bfType = 19778; // "BM"
    header.bfReserved1 = header.bfReserved2 = 0;
    header.bfSize = sizeof(tagBITMAPINFOHEADER) + sizeof(tagBITMAPFILEHEADER) + 
        sizeof(tagRGBTRIPLE) * m_width * m_height;

    inf.biSize = sizeof(tagBITMAPINFOHEADER);
    inf.biWidth = m_width;
    inf.biHeight = m_height;
    inf.biPlanes = 1;
    inf.biClrUsed = 0;
    inf.biBitCount = 24;
    inf.biXPelsPerMeter = inf.biYPelsPerMeter = 0;
    inf.biSizeImage = 0;
    inf.biClrImportant = 0;
    inf.biCompression = BI_RGB;

    size_t sz = file->write(&header, sizeof(tagBITMAPFILEHEADER));
    if (sz != sizeof(tagBITMAPFILEHEADER))
        return false;

    sz = file->write(&inf, sizeof(tagBITMAPINFOHEADER));
    if (sz != sizeof(tagBITMAPINFOHEADER))
        return false;

    //need vertical flip
    tagRGBTRIPLE* tmp = new tagRGBTRIPLE[m_width * m_height];
    
    unsigned int index;
    for (unsigned int x = 0; x < m_width; x++)
    {
        for (unsigned y = 0; y < m_height; y++)
        {
            index = x + ((m_height - 1) - y) * m_width;
            tmp[x + y * m_width] = m_bitmap[index];
        }
    }
   
    file->write( tmp, sizeof(tagRGBTRIPLE) * m_width * m_height);
    delete[] tmp;

    return true;
}

unsigned int gBMPFile::getWidth() const
{
    return m_width;
}
unsigned int gBMPFile::getHeight() const
{
    return m_height;
}

void gBMPFile::verticalFlip()
{
    if (!m_bitmap || !m_width || !m_height)
        return;

    tagRGBTRIPLE* tmp = new tagRGBTRIPLE[m_width * m_height];
    unsigned int index;

    for (unsigned int x = 0; x < m_width; x++)
    {
        for (unsigned y = 0; y < m_height; y++)
        {
            index = x + ((m_height - 1) - y) * m_width;
            tmp[x + y * m_width] = m_bitmap[index];
        }
    }
    memcpy(m_bitmap, tmp, m_width * m_height * sizeof(tagRGBTRIPLE));
    delete[] tmp;
}

const tagRGBTRIPLE* gBMPFile::getBitMap() const
{
    return m_bitmap;
}

bool gBMPFile::overlapOther(const gBMPFile& other, unsigned int inPosX, unsigned int inPosY)
{
    int otherW = other.getWidth();
    int otherH = other.getHeight();

    int usedW = (int)( inPosX + other.getWidth() ) - (int)m_width;
    int usedH = (int)( inPosY + other.getHeight() ) - (int)m_height;

    //другой битмап за рамками 
    if ((usedW >= otherW) || (usedH >= otherH))
        return false;

    const tagRGBTRIPLE* otherMap =  other.getBitMap();
    unsigned int otherMapIndex = 0;

    if (usedW > 0)
        usedW = otherW - usedW;
    else
        usedW = otherW;

    if (usedH > 0)
        usedH = otherH - usedH;
    else
        usedH = otherH;

    for (unsigned int x = inPosX; x < inPosX + usedW; x++)
    {
        for (unsigned y = inPosY; y < inPosY + usedH; y++)
        {
            otherMapIndex = x - inPosX + (y - inPosY) * otherW;
            m_bitmap[x + y * m_width] = otherMap[otherMapIndex];
        }
    }

    return true;
}

