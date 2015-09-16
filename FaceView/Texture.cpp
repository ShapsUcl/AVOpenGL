#include "StdAfx.h"
#include "Texture.h"
#include "SOIL.h"


CTexture::CTexture(void)
	: m_pathname(_T(""))
{
	m_texid=0;
}


CTexture::~CTexture(void)
{
}


bool CTexture::Load(LPCTSTR pathname)
{

	if (m_texid=SOIL_load_OGL_texture(pathname,4,0,SOIL_FLAG_MIPMAPS|SOIL_FLAG_INVERT_Y)) {
		m_pathname=pathname;

#if 0
		int	width,height,channels;
		unsigned char *imdata = SOIL_load_image(pathname,&width,&height,&channels,0);
		TRACE("Loaded %s into %dx%d with %d channels\n",pathname,width,height,channels);

		CString ofname;
		ofname.Format("c:\\temp\\texture%d.tga",m_texid);
		int res=SOIL_save_image((LPCTSTR)ofname,SOIL_SAVE_TYPE_TGA,width,height,channels,imdata);
		TRACE("Save image to %s returns code %d\n",(LPCTSTR)ofname,res);
		SOIL_free_image_data(imdata);
#endif

		return true;
	}
	else
		return false;
}

void CTexture::Reload()
{
	SOIL_load_OGL_texture((LPCTSTR)m_pathname,4,m_texid,SOIL_FLAG_MIPMAPS|SOIL_FLAG_INVERT_Y);
}
