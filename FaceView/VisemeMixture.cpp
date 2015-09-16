#include "StdAfx.h"
#include "VisemeMixture.h"


CVisemeMixture::CVisemeMixture(void)
{
	strcpy(m_viseme,"");
	m_weight=0;
}

CVisemeMixture::CVisemeMixture(LPCTSTR v,float w)
{
	strncpy(m_viseme,v,3);
	m_weight=w;
}


CVisemeMixture::~CVisemeMixture(void)
{
}
