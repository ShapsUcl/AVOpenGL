#pragma once

class CMorphStore
{
public:
	CArray<CMorph *,CMorph *>	m_pMorphs;
	CArray<CString,CString>	m_Names;
	CMapStringToPtr	m_Labels;

public:
	CMorphStore(void);
	~CMorphStore(void);
	int Add(CMorph *pmorph,LPCTSTR label);
	void RemoveAll(void);
	CMorph * GetAt(LPCTSTR label);
	CMorph * GetAt(int index);
	CMorph * operator[](int index);
	int GetCount(void);

};

