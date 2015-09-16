#pragma once
class CVisemeMixture
{
public:
	CVisemeMixture(void);
	CVisemeMixture(LPCTSTR v,float w);
	~CVisemeMixture(void);
	char	m_viseme[4];
	float	m_weight;
};

