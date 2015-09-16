#pragma once

#include "ObjFile.h"
#include "TextureStore.h"

class CObjStore
{
public:
	CArray<CObjFile *,CObjFile *>	m_pObjs;
	CTextureStore	m_Textures;
	CMapStringToPtr	m_Labels;

public:
	CObjStore(void);
	~CObjStore(void);
	int Add(LPCTSTR fname,LPCTSTR label);
	void RemoveAll(void);
	CObjFile * GetAt(LPCTSTR label);
	CObjFile * GetAt(int index);
	CObjFile * operator[](int index);
	int GetCount(void);
	void	ReloadTextures();
	bool GetLabel(int index,CString &label);
};

