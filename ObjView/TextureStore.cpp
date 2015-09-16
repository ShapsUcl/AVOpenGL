#include "StdAfx.h"
#include "TextureStore.h"


CTextureStore::CTextureStore(void)
{
}


CTextureStore::~CTextureStore(void)
{
	for (int i=0;i<m_pTextures.GetCount();i++) delete m_pTextures[i];
	m_pTextures.RemoveAll();
}

unsigned int CTextureStore::FindOrAdd(LPCTSTR pathname)
{
	for (int i=0;i<m_pTextures.GetCount();i++) {
		if (m_pTextures[i]->m_pathname.CompareNoCase(pathname)==0) {
			TRACE("Found existing texture '%s' => %d\n",(LPCTSTR)pathname,m_pTextures[i]->m_texid);
			return(m_pTextures[i]->m_texid);
		}
	}

	CTexture *pTexture = new CTexture();

	if (pTexture->Load(pathname)) {
		m_pTextures.Add(pTexture);
		TRACE("Added texture '%s' => %d\n",(LPCTSTR)pathname,pTexture->m_texid);
		return pTexture->m_texid;
	}
	else {
		delete pTexture;	
		TRACE("Failed to load texture '%s'\n",(LPCTSTR)pathname);
		return 0;
	}
}

void CTextureStore::Reload()
{
	for (int i=0;i<m_pTextures.GetCount();i++) m_pTextures[i]->Reload();
}

