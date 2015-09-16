#include "StdAfx.h"

#include "ObjFile.h"
#include "ObjStore.h"
#include "Morph.h"
#include "MorphStore.h"


CMorphStore::CMorphStore(void)
{
}


CMorphStore::~CMorphStore(void)
{
	for (int i=0;i<m_pMorphs.GetCount();i++) delete m_pMorphs[i];
	m_pMorphs.RemoveAll();
	m_Names.RemoveAll();
	m_Labels.RemoveAll();
}

int CMorphStore::Add(CMorph *pmorph,LPCTSTR label)
{
	m_pMorphs.Add(pmorph);
	m_Names.Add(label);
	m_Labels.SetAt(label,pmorph);
	return(m_pMorphs.GetCount()-1);
}

void CMorphStore::RemoveAll(void)
{
	for (int i=0;i<m_pMorphs.GetCount();i++) delete m_pMorphs[i];
	m_pMorphs.RemoveAll();
	m_Names.RemoveAll();
	m_Labels.RemoveAll();
}


CMorph * CMorphStore::GetAt(LPCTSTR label)
{
	CMorph *pmorph=NULL;
	if (m_Labels.Lookup(label,(void *&)pmorph))
		return pmorph;
	else
		return NULL;
}

CMorph * CMorphStore::GetAt(int index)
{
	if ((0<=index)&&(index<m_pMorphs.GetCount())) {
		return(m_pMorphs[index]);
	}
	return NULL;
}

CMorph * CMorphStore::operator[](int index)
{
	return GetAt(index);
}


int CMorphStore::GetCount(void)
{
	return m_pMorphs.GetCount();
}

