#include "StdAfx.h"
#include "ObjTexture.h"


CObjTexture::CObjTexture(void)
{
	imageData=NULL;
	bpp=0;
	width=0;
	height=0;
	texID=0;
	type=0;
}


CObjTexture::~CObjTexture(void)
{
	if (imageData) delete [] imageData;
}

static GLubyte uTGAcompare[12] = {0,0,2,0,0,0,0,0,0,0,0,0};    // Uncompressed TGA Header
static GLubyte cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0};   // Compressed TGA Header


bool CObjTexture::LoadTGA(LPCTSTR filename)
{
	TGAHeader tgaheader;                                    // TGA header
	FILE	*ip;

	if ((ip=fopen(filename,"rb"))==NULL) {
		TRACE("Could not open '%s'\n",filename);
		return(false);
	}

    if (fread(&tgaheader, sizeof(TGAHeader), 1, ip) == 0) {               // Attempt to read 12 byte header from file
		TRACE("Could not read header from '%s'\n",filename);
		fclose(ip);
        return false;                                                       // Exit function
    }

	// decode appropriately
    if (memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0) {          // See if header matches the predefined header of an Uncompressed TGA image
		if (!LoadUncompressedTGA(filename,ip)) {
			fclose(ip);
			return false;
		}
    } else if (memcmp(cTGAcompare, &tgaheader, sizeof(tgaheader)) == 0) {   // See if header matches the predefined header of an RLE compressed TGA image
		if (!LoadCompressedTGA(filename, ip)) {
			fclose(ip);
			return false;
		}
    } else {                                                                // If header matches neither type
		TRACE("Not a compressed or uncompressed TGA file: '%s'\n",filename);
        fclose(ip);
        return false;                                                               // Exit function
    }
	fclose(ip);

//	TRACE("LoadTGA(%s) => width=%d height=%d bpp=%d type=%d\n",filename,width,height,bpp,type);
//	TRACE("Bytes=");
	for (int i=0;(i<16)&&(i<width*height);i++) TRACE("%02X%02X%02X,",imageData[3*i],imageData[3*i+1],imageData[3*i+2]);
	TRACE("\n");

    return true; 
}

// TGA Loading code nehe.gamedev.net
bool CObjTexture::LoadUncompressedTGA(LPCTSTR filename, FILE *fTGA)
{
	TGA tga;                                                // TGA image data

	if (fread(tga.header, sizeof(tga.header), 1, fTGA) == 0) {              // Read TGA header
		TRACE("failed to read header from '%s'\n",filename);
        return false;                                                       // Return failular
    }

	width  = tga.header[1] * 256 + tga.header[0];                  // Determine The TGA Width  (highbyte*256+lowbyte)
	height = tga.header[3] * 256 + tga.header[2];                  // Determine The TGA Height (highbyte*256+lowbyte)
	bpp    = tga.header[4];                                        // Determine the bits per pixel

    if ((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32))) { // Make sure all information is valid
		TRACE("Invalid information in TGA file '%s'\n",filename);
        return false;                                                       // Return failed
    }

    if (bpp == 24)                                                 // If the BPP of the image is 24...
        type   = GL_RGB;                                           // Set Image type to GL_RGB
    else                                                                    // Else if its 32 BPP
        type   = GL_RGBA;                                          // Set image type to GL_RGBA

	int	bytesPerPixel   = (bpp / 8);                                    // Compute the number of BYTES per pixel
	int	imageSize       = (bytesPerPixel * width * height);     // Compute the total amout ofmemory needed to store data
    imageData  = new GLubyte[imageSize];                 // Allocate that much memory

    if (fread(imageData, 1, imageSize, fTGA) != imageSize) { // Attempt to read image data
		TRACE("Failed to read image data from '%s'\n",filename);
        return false;                                                       // Return failed
    }

    // Byte Swapping Optimized By Steve Thomas
    for (GLuint cswap = 0; cswap < (GLuint)imageSize; cswap += bytesPerPixel) {
		imageData[cswap] ^= imageData[cswap+2] ^= imageData[cswap] ^= imageData[cswap+2];
    }

    return true;                                                            // Return success

}

bool CObjTexture::LoadCompressedTGA(LPCTSTR filename, FILE *fTGA)
{
	TGA tga;                                                // TGA image data

	if (fread(tga.header, sizeof(tga.header), 1, fTGA) == 0) {              // Attempt to read header
		TRACE("Failed to read header from '%s'\n",filename);
        return false;                                                       // Return failed
    }

	width  = tga.header[1] * 256 + tga.header[0];                  // Determine The TGA Width  (highbyte*256+lowbyte)
	height = tga.header[3] * 256 + tga.header[2];                  // Determine The TGA Height (highbyte*256+lowbyte)
	bpp    = tga.header[4];                                        // Determine Bits Per Pixel

    if ((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32))) { // Make sure all information is valid
		TRACE("Invalid information in TGA file '%s'\n",filename);
        return false;                                                       // Return failed
    }

    if (bpp == 24)                                                 // If the BPP of the image is 24...
        type   = GL_RGB;                                           // Set Image type to GL_RGB
    else                                                                    // Else if its 32 BPP
        type   = GL_RGBA;                                          // Set image type to GL_RGBA

	int	bytesPerPixel   = (bpp / 8);                                    // Compute the number of BYTES per pixel
	int	imageSize       = (bytesPerPixel * width * height);     // Compute the total amout ofmemory needed to store data
    imageData  = new GLubyte[imageSize];                 // Allocate that much memory

    GLuint pixelcount   = height * width;                           // Nuber of pixels in the image
    GLuint currentpixel = 0;                                                // Current pixel being read
    GLuint currentbyte  = 0;                                                // Current byte
    GLubyte colorbuffer[4];

    do {
        GLubyte chunkheader = 0;                                            // Storage for "chunk" header

        if (fread(&chunkheader, sizeof(GLubyte), 1, fTGA) == 0) {           // Read in the 1 byte header
			TRACE("Failed to read chunk header from '%s'\n",filename);
            return false;                                                   // Return failed
        }

        if (chunkheader < 128) {                                            // If the ehader is < 128, it means the that is the number of RAW color packets minus 1 that follow the header
            chunkheader++;                                                  // add 1 to get number of following color values
            for (short counter = 0; counter < chunkheader; counter++) {     // Read RAW color values
                if (fread(colorbuffer, 1, bytesPerPixel, fTGA) != bytesPerPixel) { // Try to read 1 pixel
					TRACE("Failed to read pixel from '%s'\n",filename);
                    return false;                                                       // Return failed
                }
                // write to memory
				imageData[currentbyte      ] = colorbuffer[2];                 // Flip R and B vcolor values around in the process
				imageData[currentbyte + 1  ] = colorbuffer[1];
				imageData[currentbyte + 2  ] = colorbuffer[0];

                if (bytesPerPixel == 4) {                                           // if its a 32 bpp image
					imageData[currentbyte + 3] = colorbuffer[3];               // copy the 4th byte
                }

                currentbyte += bytesPerPixel;                                       // Increase thecurrent byte by the number of bytes per pixel
                currentpixel++;                                                         // Increase current pixel by 1

                if (currentpixel > pixelcount) {                                        // Make sure we havent read too many pixels
					TRACE("Read error on '%s'\n",filename);
                    return false;                                                       // Return failed
                }
            }
        } else {                                                                        // chunkheader > 128 RLE data, next color reapeated chunkheader - 127 times
            chunkheader -= 127;                                                         // Subteact 127 to get rid of the ID bit
            if (fread(colorbuffer, 1, bytesPerPixel, fTGA) != bytesPerPixel) {  // Attempt to read following color values
				TRACE("Failed to read pixel from '%s'\n",filename);
				return false;                                                       // Return failed
			}

            for (short counter = 0; counter < chunkheader; counter++) {                 // copy the color into the image data as many times as dictated
                // by the header
				imageData[currentbyte      ] = colorbuffer[2];                 // switch R and B bytes areound while copying
                imageData[currentbyte + 1  ] = colorbuffer[1];
                imageData[currentbyte + 2  ] = colorbuffer[0];

                if (bytesPerPixel == 4) {                                           // If TGA images is 32 bpp
                    imageData[currentbyte + 3] = colorbuffer[3];               // Copy 4th byte
                }

                currentbyte += bytesPerPixel;                                       // Increase current byte by the number of bytes per pixel
                currentpixel++;                                                         // Increase pixel count by 1

                if (currentpixel > pixelcount) {                                        // Make sure we havent written too many pixels
					TRACE("Read error on '%s'\n",filename);
                    return false;                                                       // Return failed
                }
            }
        }
    } while (currentpixel < pixelcount);                                                  // Loop while there are still pixels left

    return true;      
}

