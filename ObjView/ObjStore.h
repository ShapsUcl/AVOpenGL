#pragma once

#include "ObjFile.h"
#include "TextureStore.h"

class CObjStore
{
public:
	CArray<CObjFile *,CObjFile *>	m_pObjs;
	CTextureStore	m_Textures;

public:
	CObjStore(void);
	~CObjStore(void);
	int Add(LPCTSTR fname);
	void RemoveAll(void);
	CObjFile * GetAt(int index);
	CObjFile * operator[](int index);
	int GetCount(void);
	void	ReloadTextures();
};

