#pragma once
#include "Texture.h"

class CTextureStore
{
	public:
	CArray<CTexture *,CTexture *>	m_pTextures;

public:
	CTextureStore(void);
	~CTextureStore(void);
	unsigned int FindOrAdd(LPCTSTR pathname);
	void Reload();
};

