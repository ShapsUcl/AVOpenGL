#pragma once
class CRingBuffer
{
protected:
	float	*m_ring;
	int		m_rbin;
	int		m_rbout;
	int		m_size;
	CRITICAL_SECTION	m_lock;	// critical section lock
public:
	CRingBuffer(void);
	~CRingBuffer(void);
	int SetSize(int count);
	int PutSample(float sample);
	float GetSample(void);
	int GetCount(void);
	int Reset(void);
};

