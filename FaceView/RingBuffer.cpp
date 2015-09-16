#include "StdAfx.h"
#include "RingBuffer.h"


CRingBuffer::CRingBuffer(void)
{
	InitializeCriticalSection(&m_lock);
	m_rbin=m_rbout=0;
	m_ring=(float *)calloc(1000,sizeof(float));
	m_size=1000;
}


CRingBuffer::~CRingBuffer(void)
{
	DeleteCriticalSection(&m_lock);
	free(m_ring);
}


int CRingBuffer::SetSize(int count)
{
	EnterCriticalSection(&m_lock);
	free(m_ring);
	m_ring=(float *)calloc(count,sizeof(float));
	m_size=count;
	LeaveCriticalSection(&m_lock);
	return 0;
}


int CRingBuffer::PutSample(float sample)
{
	EnterCriticalSection(&m_lock);
	m_rbin=(m_rbin+1)%m_size;
	if (m_rbin==m_rbout) {
		// buffer full!
		// overflow++;
		m_rbout=(m_rbout+1)%m_size;
	}
	m_ring[m_rbin]=sample;
	LeaveCriticalSection(&m_lock);
	return 0;
}


float CRingBuffer::GetSample(void)
{
	float	samp=0;
	EnterCriticalSection(&m_lock);
	if (m_rbout==m_rbin) {
		// buffer empty!
		// underflow++
	}
	else {
		m_rbout=(m_rbout+1)%m_size;
		samp = m_ring[m_rbout];
	}
	LeaveCriticalSection(&m_lock);
	return(samp);
}


int CRingBuffer::GetCount(void)
{
	int	count=0;
	EnterCriticalSection(&m_lock);
	if (m_rbin > m_rbout)
		count = (m_rbin-m_rbout);
	else if (m_rbin < m_rbout)
		count = (m_size+m_rbin-m_rbout);
	LeaveCriticalSection(&m_lock);
	return count;
}


int CRingBuffer::Reset(void)
{
	EnterCriticalSection(&m_lock);
	m_rbin=m_rbout=0;
	LeaveCriticalSection(&m_lock);
	return 0;
}
