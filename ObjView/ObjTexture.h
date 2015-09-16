#pragma once

typedef struct _TGAHeader {
    GLubyte Header[12];                                 // TGA File Header
} TGAHeader;


typedef struct _TGA {
    GLubyte     header[6];                              // First 6 Useful Bytes From The Header
} TGA;

class CObjTexture
{
public:
    GLubyte *imageData;                                     // Image Data (Up To 32 Bits)
    GLuint  bpp;                                            // Image Color Depth In Bits Per Pixel
    GLuint  width;                                          // Image Width
    GLuint  height;                                         // Image Height
    GLuint  texID;                                          // Texture ID Used To Select A Texture
    GLuint  type;                                           // Image Type (GL_RGB, GL_RGBA)

public:
	CObjTexture(void);
	~CObjTexture(void);

	bool LoadTGA(LPCTSTR filename);
	bool LoadCompressedTGA(LPCTSTR filename, FILE *fTGA);
	bool LoadUncompressedTGA(LPCTSTR filename, FILE *fTGA);

};

