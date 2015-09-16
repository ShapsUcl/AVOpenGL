#pragma once
class CTexture
{
public:
	CTexture(void);
	~CTexture(void);
	CString m_pathname;
	unsigned int	m_texid;
	bool Load(LPCTSTR pathname);
	void Reload();
};

