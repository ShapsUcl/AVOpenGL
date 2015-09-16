// CoreAudio - support for Windows Core Audio API
//
// Mark Huckvale - University College London
//
// version 1 - June 2015
//

#include "stdafx.h"
#include "CoreAudio.h"
#include "Resample.h"

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

CCoreAudio::CCoreAudio(void)
{
	m_pMMRenderDeviceCollection=NULL;
	m_pMMCaptureDeviceCollection=NULL;
	m_pMMDefaultRenderDevice=NULL;
	m_pMMDefaultCaptureDevice=NULL;
	m_pMMRenderDevice=NULL;
	m_pMMCaptureDevice=NULL;
	memset(&m_render_wvf,0,sizeof(m_render_wvf));
	memset(&m_capture_wvf,0,sizeof(m_capture_wvf));
	memset(&m_render_wvf_ex,0,sizeof(m_render_wvf_ex));
	memset(&m_capture_wvf_ex,0,sizeof(m_capture_wvf_ex));
	m_render_exmodebits=0;
	m_capture_exmodebits=0;
}


CCoreAudio::~CCoreAudio(void)
{
	if (m_pMMRenderDeviceCollection) m_pMMRenderDeviceCollection->Release();
	if (m_pMMCaptureDeviceCollection) m_pMMCaptureDeviceCollection->Release();
	if (m_pMMDefaultRenderDevice) m_pMMDefaultRenderDevice->Release();
	if (m_pMMDefaultCaptureDevice) m_pMMDefaultCaptureDevice->Release();

}

int CCoreAudio::init(void)
{
	HRESULT hr;
	IMMDeviceEnumerator *pMMDeviceEnumerator;
	hr=CoCreateInstance(__uuidof(MMDeviceEnumerator),NULL,CLSCTX_ALL,__uuidof(IMMDeviceEnumerator),(void **)&pMMDeviceEnumerator);
	if (hr!=0) return(hr);
	hr=pMMDeviceEnumerator->EnumAudioEndpoints(eRender,DEVICE_STATE_ACTIVE,&m_pMMRenderDeviceCollection);
	if (hr!=0) return(hr);
	hr=pMMDeviceEnumerator->EnumAudioEndpoints(eCapture,DEVICE_STATE_ACTIVE,&m_pMMCaptureDeviceCollection);
	if (hr!=0) return(hr);
	hr=pMMDeviceEnumerator->GetDefaultAudioEndpoint(eRender,eConsole,&m_pMMDefaultRenderDevice);
	if (hr!=0) return(hr);
	hr=pMMDeviceEnumerator->GetDefaultAudioEndpoint(eCapture,eConsole,&m_pMMDefaultCaptureDevice);
	if (hr!=0) return(hr);
	pMMDeviceEnumerator->Release();

	return 0;
}

int CCoreAudio::getNumRenderDevices()
{
	if (!m_pMMRenderDeviceCollection) return 0;
	unsigned int	count=0;
	m_pMMRenderDeviceCollection->GetCount(&count);
	return(count);
}

int CCoreAudio::getNumCaptureDevices()
{
	if (!m_pMMCaptureDeviceCollection) return 0;
	unsigned int	count=0;
	m_pMMCaptureDeviceCollection->GetCount(&count);
	return(count);
}

BOOL Unicode16ToAnsi(WCHAR *in_Src, CHAR *out_Dst, INT in_MaxLen)
{
    /* locals */
    INT  lv_Len;
    BOOL lv_UsedDefault;

  // do NOT decrease maxlen for the eos
  if (in_MaxLen <= 0)
    return FALSE;

  // let windows find out the meaning of ansi
  // - the SrcLen=-1 triggers WCTMB to add a eos to Dst and fails if MaxLen is too small.
  // - if SrcLen is specified then no eos is added
  // - if (SrcLen+1) is specified then the eos IS added
  lv_Len = WideCharToMultiByte(
     CP_ACP, 0, in_Src, -1, out_Dst, in_MaxLen, 0, &lv_UsedDefault);

  // validate
  if (lv_Len < 0)
    lv_Len = 0;

  // ensure eos, watch out for a full buffersize
  // - if the buffer is full without an eos then clear the output like WCTMB does
  //   in case of too small outputbuffer
  // - unfortunately there is no way to let WCTMB return shortened strings,
  //   if the outputbuffer is too small then it fails completely
  if (lv_Len < in_MaxLen)
    out_Dst[lv_Len] = 0;
  else if (out_Dst[in_MaxLen-1])
    out_Dst[0] = 0;

  // return whether invalid chars were present
  return !lv_UsedDefault;
}

int CCoreAudio::getRenderDeviceNameW(int id,wchar_t *buf,int bufsize)
{
	int ndev=getNumRenderDevices();
	if ((0<=id)&&(id<ndev)) {
		IMMDevice *pMMDevice;
		m_pMMRenderDeviceCollection->Item(id,&pMMDevice);
		IPropertyStore *pPropertyStore;
		pMMDevice->OpenPropertyStore(STGM_READ,&pPropertyStore);
		pMMDevice->Release();
		PROPVARIANT pv;
		PropVariantInit(&pv);
		pPropertyStore->GetValue(PKEY_Device_FriendlyName,&pv);
		pPropertyStore->Release();
		wcsncpy(buf,pv.pwszVal,bufsize);
		PropVariantClear(&pv);
		return(1);
	}
	else if (m_pMMDefaultRenderDevice) {
		IPropertyStore *pPropertyStore;
		m_pMMDefaultRenderDevice->OpenPropertyStore(STGM_READ,&pPropertyStore);
		PROPVARIANT pv;
		PropVariantInit(&pv);
		pPropertyStore->GetValue(PKEY_Device_FriendlyName,&pv);
		pPropertyStore->Release();
		wcsncpy(buf,pv.pwszVal,bufsize);
		PropVariantClear(&pv);
		return(1);
	}
	return 0;
}


int CCoreAudio::getRenderDeviceName(int id,char *buf,int bufsize)
{
	int ndev=getNumRenderDevices();
	if ((0<=id)&&(id<ndev)) {
		IMMDevice *pMMDevice;
		m_pMMRenderDeviceCollection->Item(id,&pMMDevice);
		IPropertyStore *pPropertyStore;
		pMMDevice->OpenPropertyStore(STGM_READ,&pPropertyStore);
		pMMDevice->Release();
		PROPVARIANT pv;
		PropVariantInit(&pv);
		pPropertyStore->GetValue(PKEY_Device_FriendlyName,&pv);
		pPropertyStore->Release();
		Unicode16ToAnsi(pv.pwszVal,buf,bufsize);
		PropVariantClear(&pv);
		return(1);
	}
	else if (m_pMMDefaultRenderDevice) {
		IPropertyStore *pPropertyStore;
		m_pMMDefaultRenderDevice->OpenPropertyStore(STGM_READ,&pPropertyStore);
		PROPVARIANT pv;
		PropVariantInit(&pv);
		pPropertyStore->GetValue(PKEY_Device_FriendlyName,&pv);
		pPropertyStore->Release();
		Unicode16ToAnsi(pv.pwszVal,buf,bufsize);
		PropVariantClear(&pv);
		return(1);
	}
	return 0;
}

int CCoreAudio::getCaptureDeviceNameW(int id,wchar_t *buf,int bufsize)
{
	int ndev=getNumCaptureDevices();
	if ((0<=id)&&(id<ndev)) {
		IMMDevice *pMMDevice;
		m_pMMCaptureDeviceCollection->Item(id,&pMMDevice);
		IPropertyStore *pPropertyStore;
		pMMDevice->OpenPropertyStore(STGM_READ,&pPropertyStore);
		pMMDevice->Release();
		PROPVARIANT pv;
		PropVariantInit(&pv);
		pPropertyStore->GetValue(PKEY_Device_FriendlyName,&pv);
		pPropertyStore->Release();
		wcsncpy(buf,pv.pwszVal,bufsize);
		PropVariantClear(&pv);
		return(1);
	}
	else if (m_pMMDefaultCaptureDevice) {
		IPropertyStore *pPropertyStore;
		m_pMMDefaultCaptureDevice->OpenPropertyStore(STGM_READ,&pPropertyStore);
		PROPVARIANT pv;
		PropVariantInit(&pv);
		pPropertyStore->GetValue(PKEY_Device_FriendlyName,&pv);
		pPropertyStore->Release();
		wcsncpy(buf,pv.pwszVal,bufsize);
		PropVariantClear(&pv);
		return(1);
	}
	return 0;
}

int CCoreAudio::getCaptureDeviceName(int id,char *buf,int bufsize)
{
	int ndev=getNumCaptureDevices();
	if ((0<=id)&&(id<ndev)) {
		IMMDevice *pMMDevice;
		m_pMMCaptureDeviceCollection->Item(id,&pMMDevice);
		IPropertyStore *pPropertyStore;
		pMMDevice->OpenPropertyStore(STGM_READ,&pPropertyStore);
		pMMDevice->Release();
		PROPVARIANT pv;
		PropVariantInit(&pv);
		pPropertyStore->GetValue(PKEY_Device_FriendlyName,&pv);
		pPropertyStore->Release();
		Unicode16ToAnsi(pv.pwszVal,buf,bufsize);
		PropVariantClear(&pv);
		return(1);
	}
	else if (m_pMMDefaultCaptureDevice) {
		IPropertyStore *pPropertyStore;
		m_pMMDefaultCaptureDevice->OpenPropertyStore(STGM_READ,&pPropertyStore);
		PROPVARIANT pv;
		PropVariantInit(&pv);
		pPropertyStore->GetValue(PKEY_Device_FriendlyName,&pv);
		pPropertyStore->Release();
		Unicode16ToAnsi(pv.pwszVal,buf,bufsize);
		PropVariantClear(&pv);
		return(1);
	}
	return 0;
}

WAVEFORMATEXTENSIBLE * CCoreAudio::getRenderDeviceFormat(int id)
{
	int ndev=getNumRenderDevices();
	if ((0<=id)&&(id<ndev)) {
		IMMDevice *pMMDevice=NULL;
		m_pMMRenderDeviceCollection->Item(id,&pMMDevice);
		IAudioClient *pAudioClient = NULL;
		pMMDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
		WAVEFORMATEX *pwvf;
		if (pAudioClient->GetMixFormat(&pwvf)==0) {
			memcpy(&m_render_wvf,pwvf,sizeof(WAVEFORMATEXTENSIBLE));
			CoTaskMemFree(pwvf);
			pAudioClient->Release();
			return(&m_render_wvf);
		}
		pAudioClient->Release();
	}
	else if (m_pMMDefaultRenderDevice) {
		IAudioClient *pAudioClient = NULL;
		m_pMMDefaultRenderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
		WAVEFORMATEX *pwvf;
		if (pAudioClient->GetMixFormat(&pwvf)==0) {
			memcpy(&m_render_wvf,pwvf,sizeof(WAVEFORMATEXTENSIBLE));
			CoTaskMemFree(pwvf);
			pAudioClient->Release();
			return(&m_render_wvf);
		}
		pAudioClient->Release();
	}
	return NULL;
}

WAVEFORMATEXTENSIBLE * CCoreAudio::getCaptureDeviceFormat(int id)
{
	int ndev=getNumCaptureDevices();
	if ((0<=id)&&(id<ndev)) {
		IMMDevice *pMMDevice=NULL;
		m_pMMCaptureDeviceCollection->Item(id,&pMMDevice);
		IAudioClient *pAudioClient = NULL;
		pMMDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
		WAVEFORMATEX *pwvf;
		if (pAudioClient->GetMixFormat(&pwvf)==0) {
			memcpy(&m_render_wvf,pwvf,sizeof(WAVEFORMATEXTENSIBLE));
			CoTaskMemFree(pwvf);
			pAudioClient->Release();
			return(&m_render_wvf);
		}
		pAudioClient->Release();
	}
	else if (m_pMMDefaultCaptureDevice) {
		IAudioClient *pAudioClient = NULL;
		m_pMMDefaultCaptureDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
		WAVEFORMATEX *pwvf;
		if (pAudioClient->GetMixFormat(&pwvf)==0) {
			memcpy(&m_render_wvf,pwvf,sizeof(WAVEFORMATEXTENSIBLE));
			CoTaskMemFree(pwvf);
			pAudioClient->Release();
			return(&m_render_wvf);
		}
		pAudioClient->Release();
	}
	return NULL;
}

int CCoreAudio::checkRenderDeviceFormat(int id,int nchan,int nbits,int srate)
{
	WAVEFORMATEX wvf;
	memset(&wvf,0,sizeof(wvf));
	wvf.cbSize=0;
	wvf.nChannels=nchan;
	wvf.wBitsPerSample=nbits;
	wvf.nSamplesPerSec=srate;
	wvf.nBlockAlign=nchan*(nbits+7)/8;
	wvf.nAvgBytesPerSec=srate*wvf.nBlockAlign;
	wvf.wFormatTag=WAVE_FORMAT_PCM;

	WAVEFORMATEXTENSIBLE wvx;
	memset(&wvx,0,sizeof(wvx));
	wvx.Format=wvf;
	wvx.Format.cbSize=sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
	wvx.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
	wvx.Samples.wValidBitsPerSample=nbits;
	wvx.dwChannelMask=3;
	wvx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;

	int ndev=getNumRenderDevices();
	if ((0<=id)&&(id<ndev)) {
		IMMDevice *pMMDevice=NULL;
		m_pMMRenderDeviceCollection->Item(id,&pMMDevice);
		IAudioClient *pAudioClient = NULL;
		pMMDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
		if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvx,NULL)==S_OK) {
			pAudioClient->Release();
			return(1);
		}
		else if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,&wvf,NULL)==S_OK) {
			pAudioClient->Release();
			return(1);
		}
		pAudioClient->Release();
	}
	else if (m_pMMDefaultRenderDevice) {
		IAudioClient *pAudioClient = NULL;
		m_pMMDefaultRenderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
		if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvx,NULL)==S_OK) {
			pAudioClient->Release();
			return(1);
		}
		else if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,&wvf,NULL)==S_OK) {
			pAudioClient->Release();
			return(1);
		}
		pAudioClient->Release();
	}
	return 0;
}

int CCoreAudio::setRenderDevice(int id)
{
	int ndev=getNumRenderDevices();
	if ((0<=id)&&(id<ndev)) {
		// select chosen device
		m_pMMRenderDeviceCollection->Item(id,&m_pMMRenderDevice);
	}
	else {
		// select default device
		IMMDeviceEnumerator *pMMDeviceEnumerator;
		CoCreateInstance(__uuidof(MMDeviceEnumerator),NULL,CLSCTX_ALL,__uuidof(IMMDeviceEnumerator),(void **)&pMMDeviceEnumerator);
		pMMDeviceEnumerator->GetDefaultAudioEndpoint(eRender,eConsole,&m_pMMRenderDevice);
		pMMDeviceEnumerator->Release();
	}
	IAudioClient *pAudioClient = NULL;
	m_pMMRenderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
	WAVEFORMATEX *pwvf;
	pAudioClient->GetMixFormat(&pwvf);
	memcpy(&m_render_wvf,pwvf,sizeof(WAVEFORMATEXTENSIBLE));
	CoTaskMemFree(pwvf);
	pAudioClient->Release();

//	TRACE("Selected Render device %d, rate=%d, nchan=%d, nbits=%d\n",id,m_render_wvf.Format.nSamplesPerSec,m_render_wvf.Format.nChannels,m_render_wvf.Format.wBitsPerSample);
	return 0;
}

int CCoreAudio::checkCaptureDeviceFormat(int id,int nchan,int nbits,int srate)
{
	WAVEFORMATEX wvf;
	memset(&wvf,0,sizeof(wvf));
	wvf.cbSize=0;
	wvf.nChannels=nchan;
	wvf.wBitsPerSample=nbits;
	wvf.nSamplesPerSec=srate;
	wvf.nBlockAlign=nchan*(nbits+7)/8;
	wvf.nAvgBytesPerSec=srate*wvf.nBlockAlign;
	wvf.wFormatTag=WAVE_FORMAT_PCM;

	WAVEFORMATEXTENSIBLE wvx;
	memset(&wvx,0,sizeof(wvx));
	wvx.Format=wvf;
	wvx.Format.cbSize=sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
	wvx.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
	wvx.Samples.wValidBitsPerSample=nbits;
	wvx.dwChannelMask=3;
	wvx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;

	int ndev=getNumCaptureDevices();
	if ((0<=id)&&(id<ndev)) {
		IMMDevice *pMMDevice=NULL;
		m_pMMCaptureDeviceCollection->Item(id,&pMMDevice);
		IAudioClient *pAudioClient = NULL;
		pMMDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
		if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvx,NULL)==S_OK) {
			pAudioClient->Release();
			return(1);
		}
		else if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,&wvf,NULL)==S_OK) {
			pAudioClient->Release();
			return(1);
		}
		pAudioClient->Release();
	}
	else if (m_pMMDefaultRenderDevice) {
		IAudioClient *pAudioClient = NULL;
		m_pMMDefaultCaptureDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
		if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvx,NULL)==S_OK) {
			pAudioClient->Release();
			return(1);
		}
		else if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,&wvf,NULL)==S_OK) {
			pAudioClient->Release();
			return(1);
		}
		pAudioClient->Release();
	}
	return 0;
}

int CCoreAudio::setCaptureDevice(int id)
{
	int ndev=getNumCaptureDevices();
	if ((0<=id)&&(id<ndev)) {
		// select chosen device
		m_pMMCaptureDeviceCollection->Item(id,&m_pMMCaptureDevice);
	}
	else {
		// select default device
		IMMDeviceEnumerator *pMMDeviceEnumerator;
		CoCreateInstance(__uuidof(MMDeviceEnumerator),NULL,CLSCTX_ALL,__uuidof(IMMDeviceEnumerator),(void **)&pMMDeviceEnumerator);
		pMMDeviceEnumerator->GetDefaultAudioEndpoint(eCapture,eConsole,&m_pMMCaptureDevice);
		pMMDeviceEnumerator->Release();
	}
	IAudioClient *pAudioClient = NULL;
	m_pMMCaptureDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
	WAVEFORMATEX *pwvf;
	pAudioClient->GetMixFormat(&pwvf);
	memcpy(&m_capture_wvf,pwvf,sizeof(WAVEFORMATEXTENSIBLE));
	CoTaskMemFree(pwvf);
	pAudioClient->Release();

//	TRACE("Selected Capture device %d, rate=%d, nchan=%d, nbits=%d\n",id,m_capture_wvf.Format.nSamplesPerSec,m_capture_wvf.Format.nChannels,m_capture_wvf.Format.wBitsPerSample);
	return 0;
}

int CCoreAudio::setRenderExclusiveMode(int mode)
{
	m_render_exmodebits=0;
	if (!m_pMMRenderDevice) return 0;
	if (mode==0) return 0;

	WAVEFORMATEX wvf;
	memset(&wvf,0,sizeof(wvf));
	wvf.cbSize=0;
	wvf.nChannels=m_render_wvf.Format.nChannels;
	wvf.wBitsPerSample=32;
	wvf.nSamplesPerSec=m_render_wvf.Format.nSamplesPerSec;
	wvf.nBlockAlign=wvf.nChannels*(wvf.wBitsPerSample+7)/8;
	wvf.nAvgBytesPerSec=wvf.nSamplesPerSec*wvf.nBlockAlign;
	wvf.wFormatTag=WAVE_FORMAT_PCM;

	WAVEFORMATEXTENSIBLE wvx;
	memset(&wvx,0,sizeof(wvx));
	wvx.Format=wvf;
	wvx.Format.cbSize=sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
	wvx.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
	wvx.Samples.wValidBitsPerSample=wvf.wBitsPerSample;
	wvx.dwChannelMask=3;
	wvx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;

	IAudioClient *pAudioClient = NULL;
	m_pMMRenderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
	if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvx,NULL)==S_OK) {
		m_render_exmodebits=32;
		memcpy(&m_render_wvf_ex,&wvx,sizeof(WAVEFORMATEXTENSIBLE));
	}
	else if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvf,NULL)==S_OK) {
		m_render_exmodebits=32;
		memcpy(&m_render_wvf_ex,&wvf,sizeof(WAVEFORMATEX));
	}
	else {
		wvf.wBitsPerSample=24;
		wvf.nBlockAlign=wvf.nChannels*(wvf.wBitsPerSample+7)/8;
		wvf.nAvgBytesPerSec=wvf.nSamplesPerSec*wvf.nBlockAlign;
		wvx.Format=wvf;
		wvx.Format.cbSize=sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
		wvx.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
		wvx.Samples.wValidBitsPerSample=wvf.wBitsPerSample;
		if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvx,NULL)==S_OK) {
			m_render_exmodebits=24;
			memcpy(&m_render_wvf_ex,&wvx,sizeof(WAVEFORMATEXTENSIBLE));
		}
		else if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvf,NULL)==S_OK) {
			m_render_exmodebits=24;
			memcpy(&m_render_wvf_ex,&wvf,sizeof(WAVEFORMATEX));
		}
		else {
			wvf.wBitsPerSample=16;
			wvf.nBlockAlign=wvf.nChannels*(wvf.wBitsPerSample+7)/8;
			wvf.nAvgBytesPerSec=wvf.nSamplesPerSec*wvf.nBlockAlign;
			wvx.Format=wvf;
			wvx.Format.cbSize=sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
			wvx.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
			wvx.Samples.wValidBitsPerSample=wvf.wBitsPerSample;
			if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvx,NULL)==S_OK) {
				m_render_exmodebits=16;
				memcpy(&m_render_wvf_ex,&wvx,sizeof(WAVEFORMATEXTENSIBLE));
			}
			else if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvf,NULL)==S_OK) {
				m_render_exmodebits=16;
				memcpy(&m_render_wvf_ex,&wvf,sizeof(WAVEFORMATEX));
			}
		}
	}
	pAudioClient->Release();
	return m_render_exmodebits;
}

int CCoreAudio::getRenderExclusiveMode()
{
	return m_render_exmodebits;
}

int CCoreAudio::setCaptureExclusiveMode(int mode)
{
	m_capture_exmodebits=0;
	if (!m_pMMCaptureDevice) return 0;
	if (mode==0) return 0;

	WAVEFORMATEX wvf;
	memset(&wvf,0,sizeof(wvf));
	wvf.cbSize=0;
	wvf.nChannels=m_capture_wvf.Format.nChannels;
	wvf.wBitsPerSample=32;
	wvf.nSamplesPerSec=m_capture_wvf.Format.nSamplesPerSec;
	wvf.nBlockAlign=wvf.nChannels*(wvf.wBitsPerSample+7)/8;
	wvf.nAvgBytesPerSec=wvf.nSamplesPerSec*wvf.nBlockAlign;
	wvf.wFormatTag=WAVE_FORMAT_PCM;

	WAVEFORMATEXTENSIBLE wvx;
	memset(&wvx,0,sizeof(wvx));
	wvx.Format=wvf;
	wvx.Format.cbSize=sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
	wvx.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
	wvx.Samples.wValidBitsPerSample=wvf.wBitsPerSample;
	wvx.dwChannelMask=3;
	wvx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;

	IAudioClient *pAudioClient = NULL;
	m_pMMCaptureDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
	if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvx,NULL)==S_OK) {
		m_capture_exmodebits=32;
		memcpy(&m_capture_wvf_ex,&wvx,sizeof(WAVEFORMATEXTENSIBLE));
	}
	else if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvf,NULL)==S_OK) {
		m_capture_exmodebits=32;
		memcpy(&m_capture_wvf_ex,&wvf,sizeof(WAVEFORMATEX));
	}
	else {
		wvf.wBitsPerSample=24;
		wvf.nSamplesPerSec=m_capture_wvf.Format.nSamplesPerSec;
		wvf.nBlockAlign=wvf.nChannels*(wvf.wBitsPerSample+7)/8;
		wvf.nAvgBytesPerSec=wvf.nSamplesPerSec*wvf.nBlockAlign;
		wvx.Format=wvf;
		wvx.Format.cbSize=sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
		wvx.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
		wvx.Samples.wValidBitsPerSample=wvf.wBitsPerSample;
		if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvx,NULL)==S_OK) {
			m_capture_exmodebits=24;
			memcpy(&m_capture_wvf_ex,&wvx,sizeof(WAVEFORMATEXTENSIBLE));
//			TRACE("SetCaptureExclusive wvx = 24\n");
		}
		else
			if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvf,NULL)==S_OK) {
			m_capture_exmodebits=24;
			memcpy(&m_capture_wvf_ex,&wvf,sizeof(WAVEFORMATEX));
//			TRACE("SetCaptureExclusive wvf = 24\n");
		}
		else {
			wvf.wBitsPerSample=16;
			wvf.nBlockAlign=wvf.nChannels*(wvf.wBitsPerSample+7)/8;
			wvf.nAvgBytesPerSec=wvf.nSamplesPerSec*wvf.nBlockAlign;
			wvx.Format=wvf;
			wvx.Format.cbSize=sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
			wvx.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
			wvx.Samples.wValidBitsPerSample=wvf.wBitsPerSample;
			if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvx,NULL)==S_OK) {
				m_capture_exmodebits=16;
				memcpy(&m_capture_wvf_ex,&wvx,sizeof(WAVEFORMATEXTENSIBLE));
//				TRACE("SetCaptureExclusive wvx = 16\n");
			}
			else if (pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,(WAVEFORMATEX *)&wvf,NULL)==S_OK) {
				m_capture_exmodebits=16;
				memcpy(&m_capture_wvf_ex,&wvf,sizeof(WAVEFORMATEX));
//				TRACE("SetCaptureExclusive wvf = 16\n");
			}
		}
	}
	pAudioClient->Release();
	return m_capture_exmodebits;

}

int CCoreAudio::getCaptureExclusiveMode()
{
	return m_capture_exmodebits;
}


void CCoreAudio::reverse(float *buf,int spos,int len)
{
	float	tmp;
	int	i;

	for (i=0;i<len;i++) {
		int j=len-1-i;
		if (j <= i) break;
		tmp=buf[spos+i];
		buf[spos+i]=buf[spos+j];
		buf[spos+j]=tmp;
	}
}

void CCoreAudio::interleave(float *buf,int len)
{
	int	n=len/2,m=1;
	int	d,p=1;
	float s,t;

	while ((p*3)<=(len+1)) p *= 3;
	m=(p-1)/2;

	if (2*m < len) {
		reverse(buf,m+1,n);
		reverse(buf,m+1,m);
		reverse(buf,2*m+1,n-m);
	}

	p=1;
	while (p < 2*m) {
		s = buf[p];
		d = (2*p) % (2*m+1);
		while (d!=p) {
			t = buf[d];
			buf[d] = s;
			s = t;
			d = (2*d) % (2*m+1);
		}
		buf[p]=t;
		p = p*3;
	}

	if (2*m < len) {
		interleave(buf+2*m,len-2*m);
	}
}

void CCoreAudio::uninterleave(float *buf,int len)
{
	int	n=len/2,m=1;
	int	d,p=1;
	float s,t;

	while ((p*3)<=(len+1)) p *= 3;
	m=(p-1)/2;

	if (2*m < len) {
		uninterleave(buf+2*m,len-2*m);
	}

	p=1;
	while (p < 2*m) {
		s = buf[p];
		if (p&1)
			d = ((p+2*m+1)/2) % (2*m+1);
		else
			d = p/2;
		while (d!=p) {
			t = buf[d];
			buf[d] = s;
			s = t;
			if (d&1)
				d = ((d+2*m+1)/2) % (2*m+1);
			else
				d = d/2;
		}
		buf[p]=t;
		p = p*3;
	}

	if (2*m < len) {
		reverse(buf,2*m+1,n-m);
		reverse(buf,m+1,m);
		reverse(buf,m+1,n);
	}

}

int CCoreAudio::resample(float *buf,int nsamp,double srate,int nchan,float *obuf,int osamp,double orate)
{
	if (nchan==2) {
		uninterleave(buf-1,2*nsamp);
		CResample rs1;
		rs1.init(orate/srate);
		int	nused;
		int ocnt=rs1.process(buf,nsamp,0,&nused,obuf,osamp);
		rs1.close();
//		printf("(1) Resample %d samples (%d used) to %d samples (%d wanted)\n",nsamp,nused,ocnt,osamp);
		CResample rs2;
		rs2.init(orate/srate);
		rs2.process(buf+nsamp,nsamp,0,&nused,obuf+ocnt,osamp);
		rs2.close();
//		printf("(2) Resample %d samples (%d used) to %d samples (%d wanted)\n",nsamp,nused,ocnt,osamp);
		interleave(obuf-1,2*ocnt);
		return ocnt;
	}
	else {
		CResample rs;
		rs.init(orate/srate);
		int	nused;
		int ocnt=rs.process(buf,nsamp,0,&nused,obuf,osamp);
		rs.close();
		return ocnt;
	}
}

void CCoreAudio::convertBuffer(void *src,void *dst,int nsamp,int nbits,int dir)
{
	int	i;
//	TRACE("convertbuffer(%d,%d,%d)\n",nsamp,nbits,dir);
	if (dir==0) {
		float	*fbuf=(float *)src;
		// float to int
		if (nbits==16) {
			short *sbuf=(short *)dst;
			for (i=0;i<nsamp;i++) sbuf[i] = (short)(32767*fbuf[i]);
		}
		else if (nbits==24) {
			unsigned char *cbuf=(unsigned char *)dst;
			for (i=0;i<nsamp;i++) {
				int	s=(int)(8388607*fbuf[i]);
				cbuf[3*i] = s & 0xFF;
				cbuf[3*i+1] = (s>>8) & 0xFF;
				cbuf[3*i+2] = (s>>16) & 0xFF;
			}
		}
	}
	else {
		float	*fbuf=(float *)dst;
		// int to float
		if (nbits==16) {
			short *sbuf=(short *)src;
			for (i=nsamp-1;i>=0;i--) fbuf[i] = (float)(sbuf[i]/32767.0);
		}
		else if (nbits==24) {
			unsigned char *cbuf=(unsigned char *)src;
			for (i=nsamp-1;i>=0;i--) {
				int s=(signed char)cbuf[3*i+2];
				s = s<<8 | cbuf[3*i+1];
				s = s<<8 | cbuf[3*i];
				fbuf[i] = (float)(s/8388607.0);
			}
		}
	}
}

int CCoreAudio::playBuffer(float *buf,int nsamp,double srate,int nchan)
{
	HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC/5;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    UINT32 numFramesPadding;
	int pos=0;
	IAudioClient *pAudioClient = NULL;
    IAudioRenderClient *pRenderClient = NULL;
	BYTE *pData;
	float *fData;
	float	*rbuf=(float *)buf;

	// perform resampling if required
	if (srate != m_render_wvf.Format.nSamplesPerSec) {
		double factor = m_render_wvf.Format.nSamplesPerSec/srate;
		int ncnt = (int)(0.5+nsamp * factor);
		rbuf=(float *)calloc(100+ncnt*nchan,sizeof(float));
		if (nchan==2) {
			uninterleave(buf-1,2*nsamp);
			CResample rs1;
			rs1.init(factor);
			int	nused;
			int ocnt=rs1.process(buf,nsamp,0,&nused,rbuf,ncnt);
			rs1.close();
			CResample rs2;
			rs2.init(factor);
			rs2.process(buf+nsamp,nsamp,0,&nused,rbuf+ocnt,ncnt);
			rs2.close();
			interleave(rbuf-1,2*ocnt);
			interleave(buf-1,2*nsamp);
			nsamp=ocnt;
		}
		else {
			CResample rs;
			rs.init(m_render_wvf.Format.nSamplesPerSec/srate);
			int	nused;
			int ocnt=rs.process(buf,nsamp,0,&nused,rbuf,ncnt);
			rs.close();
			nsamp=ocnt;
		}
	}

	hr = m_pMMRenderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
	if (hr!=0) {
//		fprintf(stderr,"Failed to activate audio client\n");
		return(hr);
	}

	hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,hnsRequestedDuration, 0, (WAVEFORMATEX *)&m_render_wvf, NULL);
	if ((hr!=0)&&(hr!=AUDCLNT_E_ALREADY_INITIALIZED)) {
//		fprintf(stderr,"Failed to initialise audio client\n");
		return(hr);
	}

	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
	HANDLE hNeedDataEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = pAudioClient->SetEventHandle(hNeedDataEvent);
	hr = pAudioClient->GetService(__uuidof(IAudioRenderClient),(void**)&pRenderClient);
	hr = pRenderClient->GetBuffer(bufferFrameCount, &pData);
//	printf("BufferSize=%d frames\n",bufferFrameCount);

    hnsActualDuration = (REFERENCE_TIME)((double)REFTIMES_PER_SEC * bufferFrameCount / srate);

	// store data
	fData = (float *)pData;
	for (int i=0;i<(int)bufferFrameCount;i++) {
		for (int j=0;j<m_render_wvf.Format.nChannels;j++) {
			if (pos < nsamp) {
				if (j<nchan)
					fData[m_render_wvf.Format.nChannels*i+j] = rbuf[nchan*pos+j];
				else
					fData[m_render_wvf.Format.nChannels*i+j] = rbuf[nchan*pos];
			}
			else
				fData[m_render_wvf.Format.nChannels*i+j]=0;
		}
		pos++;
	}

	hr = pRenderClient->ReleaseBuffer(bufferFrameCount, 0);

    // register with MMCSS
    DWORD nTaskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristics(_T("Playback"), &nTaskIndex);

	// Start playing.
    hr = pAudioClient->Start();

    while (pos < nsamp) {

		// wait for buffer to play
        DWORD retval = WaitForSingleObject(hNeedDataEvent,2000);
		if (retval != WAIT_OBJECT_0) {
            // Event handle timed out after a 2-second wait.
			pAudioClient->Stop();
//			fprintf(stderr,"Event timed out\n");
			break;
        }

        // See how much buffer space is available.
        hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
		numFramesAvailable = bufferFrameCount - numFramesPadding;
//printf("bufferFrameCount=%d numFramesAvailable=%d\n",bufferFrameCount,numFramesAvailable);

        // Grab all the available space in the shared buffer.
        hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);

        // Get next block of data from the audio source.
		fData = (float *)pData;
		for (int i=0;i<(int)numFramesAvailable;i++) {
			for (int j=0;j<m_render_wvf.Format.nChannels;j++) {
				if (pos < nsamp) {
					if (j<nchan)
						fData[m_render_wvf.Format.nChannels*i+j] = rbuf[nchan*pos+j];
					else
						fData[m_render_wvf.Format.nChannels*i+j] = rbuf[nchan*pos];
				}
				else
					fData[m_render_wvf.Format.nChannels*i+j]=0;
			}
			pos++;
		}

		hr = pRenderClient->ReleaseBuffer(numFramesAvailable, 0);
    }

    // Wait for last data in buffer to play before stopping.
    Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC));

	// Stop playing.
    hr = pAudioClient->Stop();
	AvRevertMmThreadCharacteristics(hTask);
	pRenderClient->Release();
	CloseHandle(hNeedDataEvent);
	pAudioClient->Release();

	if (rbuf!=buf) free(rbuf);

	return 0;
}

int CCoreAudio::capture(double srate,int nchan,int buffersize,int (*CopyData)(void *,float *,int,int),void *context)
{
	HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC/100;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
	IAudioClient *pAudioClient = NULL;
    IAudioCaptureClient *pCaptureClient = NULL;
    BOOL bDone = FALSE;
    UINT32 packetLength = 0;
    BYTE *pData;
    DWORD flags;
	CResample rs1,rs2;
	int capture_mode=0;
	BYTE *tcapbuf=NULL;

	double factor = srate/m_capture_wvf.Format.nSamplesPerSec;
	rs1.init(factor);
	rs2.init(factor);

	hr = m_pMMCaptureDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
	if (hr!=0) {
//		TRACE("Failed to activate audio client\n");
		return(hr);
	}

	REFERENCE_TIME p1,p2;
	pAudioClient->GetDevicePeriod(&p1,&p2);
//	TRACE("capture device period = %d,%d\n",(int)p1,(int)p2);

	if (m_capture_exmodebits &&
		((hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,p2,p2, (WAVEFORMATEX *)&m_capture_wvf_ex, NULL))==0)) {
//		TRACE("Opened capture in EXCLUSIVE mode\n");
		capture_mode=1;
	}
	else {
		m_capture_exmodebits=0;
		hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,hnsRequestedDuration, 0, (WAVEFORMATEX *)&m_capture_wvf, NULL);
//		TRACE("Opened capture in SHARED mode\n");
		capture_mode=0;
	}
	if ((hr!=0)&&(hr!=AUDCLNT_E_ALREADY_INITIALIZED)) {
//		TRACE("Failed to initialise audio client, hr=%X\n",hr);
		return(hr);
	}

	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
//	TRACE("BufferSize=%d frames\n",bufferFrameCount);
	HANDLE hHaveDataEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = pAudioClient->SetEventHandle(hHaveDataEvent);
	hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient),(void**)&pCaptureClient);

    hnsActualDuration = (REFERENCE_TIME)((double)REFTIMES_PER_SEC * bufferFrameCount / m_capture_wvf.Format.nSamplesPerSec);

	if (capture_mode) tcapbuf = (BYTE *)calloc(bufferFrameCount,sizeof(float));

    // register with MMCSS
    DWORD nTaskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristics(_T("Audio"), &nTaskIndex);

    hr = pAudioClient->Start();  // Start recording.
	if (hr!=0) return(hr);

    // Each loop fills about half of the shared buffer.
    while (bDone == FALSE)
    {
		// wait for data to become available
        DWORD retval = WaitForSingleObject(hHaveDataEvent, 2000);
		if (retval != WAIT_OBJECT_0) {
            // Event handle timed out after a 2-second wait.
//			fprintf(stderr,"Capture: Event timed out\n");
			break;
        }

        hr = pCaptureClient->GetNextPacketSize(&packetLength);

        while ((!bDone) && (packetLength != 0)) {
            // Get the available data in the shared buffer.
            hr = pCaptureClient->GetBuffer(&pData,&numFramesAvailable,&flags, NULL, NULL);
			float *buf=(float *)pData;
			float *rbuf=buf;
			int		nsamp=numFramesAvailable;

			if (capture_mode) {
				convertBuffer(buf,tcapbuf,m_capture_wvf_ex.Format.nChannels*nsamp,m_capture_exmodebits,1);
				rbuf=(float *)tcapbuf;
			}

			float max=0;
			for (int i=0;i<nsamp;i++) if (fabs(rbuf[i])>max) max=fabs(rbuf[i]);
//			if (max > 1.0) TRACE("CoreAudio::Render, max=%g\n",max);

			if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {

				// map number of channels to requirements
				if (m_capture_wvf.Format.nChannels != nchan) {
					if ((m_capture_wvf.Format.nChannels==1) && (nchan==2)) {
						rbuf=(float *)calloc(2*nsamp,sizeof(float));
						for (int i=0;i<nsamp;i++) {
							rbuf[2*i] = buf[i];
							rbuf[2*i+1] = buf[i];
						}
					}
					else if ((m_capture_wvf.Format.nChannels==2) && (nchan==1)) {
						rbuf=(float *)calloc(nsamp,sizeof(float));
						for (int i=0;i<nsamp;i++) {
							rbuf[i] = (buf[2*i]+buf[2*i+1])/2;
						}
					}
				}

				// perform resampling if required
				if (srate != m_capture_wvf.Format.nSamplesPerSec) {
					double factor = srate/m_capture_wvf.Format.nSamplesPerSec;
					int ncnt = (int)(0.5+nsamp*factor);
					float *tbuf=(float *)calloc(100+ncnt*nchan,sizeof(float));
					if (nchan==2) {
						uninterleave(rbuf-1,2*nsamp);
						int	nused;
						int ocnt=rs1.process(rbuf,nsamp,0,&nused,tbuf,ncnt);
						rs2.process(rbuf+nsamp,nsamp,0,&nused,tbuf+ocnt,ncnt);
						interleave(tbuf-1,2*ocnt);
						nsamp=ocnt;
						if ((rbuf!=buf)&&(rbuf!=(float *)tcapbuf)) free(rbuf);
						rbuf=tbuf;
					}
					else {
						int	nused;
						int ocnt=rs1.process(rbuf,nsamp,0,&nused,tbuf,ncnt);
						nsamp=ocnt;
						if ((rbuf!=buf)&&(rbuf!=(float *)tcapbuf)) free(rbuf);
						rbuf=tbuf;
					}
				}

				// Copy the available capture data to the audio sink.
				int n = (*CopyData)(context,rbuf,nsamp,nchan);
				if (n < 0) bDone=TRUE;
				if ((rbuf!=buf)&&(rbuf!=(float *)tcapbuf)) free(rbuf);
			}

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            hr = pCaptureClient->GetNextPacketSize(&packetLength);
        }
    }

//	TRACE("Capture complete\n");

	pAudioClient->Stop();  // Stop recording.
	AvRevertMmThreadCharacteristics(hTask);
	pCaptureClient->Release();
	CloseHandle(hHaveDataEvent);
	pAudioClient->Release();

	if (tcapbuf) free(tcapbuf);

	return 0;
}

int CCoreAudio::render(double srate,int nchan,int buffersize,int (*CopyData)(void *,float *,int,int),void *context)
{
	HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC/100;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    UINT32 numFramesPadding;
	IAudioClient *pAudioClient = NULL;
    IAudioRenderClient *pRenderClient = NULL;
	BYTE *pData;
	CResample rs1,rs2;
	float	*rbuf;
	int		rleft=0;
	double factor = m_render_wvf.Format.nSamplesPerSec/srate;
	int render_mode=0;
	BYTE *trenbuf=NULL;

	rs1.init(factor);
	rs2.init(factor);

	hr = m_pMMRenderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioClient);
	if (hr!=0) {
//		TRACE("Failed to activate audio client\n");
		return(hr);
	}

	REFERENCE_TIME p1,p2;
	pAudioClient->GetDevicePeriod(&p1,&p2);
//	TRACE("render device period = %d,%d\n",(int)p1,(int)p2);

	if (m_render_exmodebits &&
		((hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,p2,p2, (WAVEFORMATEX *)&m_render_wvf_ex, NULL))==0)) {
//		TRACE("Opened render in EXCLUSIVE mode\n");
		render_mode=1;
	}
	else {
		m_render_exmodebits=0;
		hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,hnsRequestedDuration, 0, (WAVEFORMATEX *)&m_render_wvf, NULL);
//		TRACE("Opened render in SHARED mode\n");
		render_mode=0;
	}
	if ((hr!=0)&&(hr!=AUDCLNT_E_ALREADY_INITIALIZED)) {
//		TRACE("Failed to initialise audio client, hr=%X\n",hr);
		return(hr);
	}

	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
//	TRACE("Render, bufferFrameCount=%d\n",bufferFrameCount);

	HANDLE hNeedDataEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = pAudioClient->SetEventHandle(hNeedDataEvent);
	hr = pAudioClient->GetService(__uuidof(IAudioRenderClient),(void**)&pRenderClient);
	hr = pRenderClient->GetBuffer(bufferFrameCount, &pData);
	int nsamp = bufferFrameCount;

	hnsActualDuration = (REFERENCE_TIME)((double)REFTIMES_PER_SEC * bufferFrameCount / srate);

	int rbuflen = 4*bufferFrameCount;
	if (factor < 1) rbuflen = (int)(4 * bufferFrameCount / factor);
	rbuf = (float *)calloc(rbuflen,sizeof(float));
//	TRACE("Render, rbuf size = %d\n",rbuflen);

	if (render_mode) trenbuf = (BYTE *)calloc(bufferFrameCount,sizeof(float));
	float *buf = (render_mode) ? (float *)trenbuf : (float *)pData;

	// get initial buffer
	if (m_render_wvf.Format.nSamplesPerSec!=srate) {
		nsamp = (int)(nsamp/factor);
		if (nsamp > buffersize) nsamp=buffersize;
		nsamp=CopyData(context,rbuf,nsamp,nchan);
		if (nsamp <=0) {
			free(rbuf);
			return 0;
		}
		int ncnt = (int)(nsamp*factor);
		float *tbuf=(float *)calloc(2*ncnt*nchan,sizeof(float));
//		TRACE("render: nsamp=%d ncnt=%d nchan=%d\n",nsamp,ncnt,nchan);
		if (nchan==2) {
			uninterleave(rbuf-1,2*nsamp);
			int	nused;
			int ocnt=rs1.process(rbuf,nsamp,0,&nused,tbuf,ncnt);
			rs2.process(rbuf+nsamp,nsamp,0,&nused,tbuf+ocnt,ncnt);
			interleave(tbuf-1,2*ocnt);
			if (nused < nsamp) {
				rleft=nsamp-nused;
				memmove(rbuf,rbuf+nused*nchan,rleft*nchan*sizeof(float));
			}
//			TRACE("render: nsamp=%d ncnt=%d nchan=%d ocnt=%d\n",nsamp,ncnt,nchan,ocnt);
			memcpy(rbuf,tbuf,ocnt*nchan*sizeof(float));
			nsamp=ocnt;
		}
		else {
			if (nsamp > buffersize) nsamp=buffersize;
			int	nused;
			int ocnt=rs1.process(rbuf,nsamp,0,&nused,tbuf,ncnt);
			if (nused < nsamp) {
				rleft=nsamp-nused;
				memmove(rbuf,rbuf+nused*nchan,rleft*nchan*sizeof(float));
			}
			memcpy(rbuf,tbuf,ocnt*nchan*sizeof(float));
			nsamp=ocnt;
		}
		free(tbuf);
	}
	else {
		if (nsamp > buffersize) nsamp=buffersize;
		nsamp=CopyData(context,rbuf,nsamp,nchan);
		rleft=0;
	}
	for (int i=0;i<nsamp;i++) {
		for (int j=0;j<m_render_wvf.Format.nChannels;j++) {
			if (j<nchan)
				buf[m_render_wvf.Format.nChannels*i+j] = rbuf[nchan*i+j];
			else
				buf[m_render_wvf.Format.nChannels*i+j] = rbuf[nchan*i];
		}
	}
	if (render_mode) convertBuffer(buf,pData,m_render_wvf_ex.Format.nChannels*nsamp,m_render_exmodebits,0);
	hr = pRenderClient->ReleaseBuffer(nsamp, 0);

    // register with MMCSS
    DWORD nTaskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristics(_T("Playback"), &nTaskIndex);

	// Start playing.
    hr = pAudioClient->Start();

    while (1) {

		// wait for buffer to play
        DWORD retval = WaitForSingleObject(hNeedDataEvent,2000);
		if (retval != WAIT_OBJECT_0) {
            // Event handle timed out after a 2-second wait.
			pAudioClient->Stop();
//			TRACE("Render: Event timed out\n");
			break;
        }

        // See how much buffer space is available.
        hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
		numFramesAvailable = bufferFrameCount - numFramesPadding;

        // Grab all the available space in the shared buffer.
        hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);
		buf=(render_mode) ? (float *)trenbuf : (float *)pData;
		nsamp = numFramesAvailable;

		// get next buffer
		if (m_render_wvf.Format.nSamplesPerSec!=srate) {
			nsamp = (int)(nsamp/factor);
			if (nsamp+rleft > buffersize) nsamp=buffersize-rleft;
			nsamp=CopyData(context,rbuf+rleft*nchan,nsamp-rleft,nchan);
			if (nsamp < 0) {
//				TRACE("CoreAudio::Render. CopyData returns %d\n",nsamp);
				break;
			}
			nsamp += rleft;
			rleft=0;
			int ncnt = (int)(nsamp*factor);
			float *tbuf=(float *)calloc(2*ncnt*nchan,sizeof(float));
			if (nchan==2) {
				uninterleave(rbuf-1,2*nsamp);
				int	nused;
				int ocnt=rs1.process(rbuf,nsamp,0,&nused,tbuf,ncnt);
				rs2.process(rbuf+nsamp,nsamp,0,&nused,tbuf+ocnt,ncnt);
				interleave(tbuf-1,2*ocnt);
				if (nused < nsamp) {
					rleft=nsamp-nused;
					memmove(rbuf,rbuf+nused*nchan,rleft*nchan*sizeof(float));
				}
				memcpy(rbuf,tbuf,ocnt*nchan*sizeof(float));
				nsamp=ocnt;
			}
			else {
				if (nsamp+rleft > buffersize) nsamp=buffersize-rleft;
				int	nused;
				int ocnt=rs1.process(rbuf,nsamp,0,&nused,tbuf,ncnt);
				if (nused < nsamp) {
					rleft=nsamp-nused;
					memmove(rbuf,rbuf+nused*nchan,rleft*nchan*sizeof(float));
				}
				memcpy(rbuf,tbuf,ocnt*nchan*sizeof(float));
				nsamp=ocnt;
			}
			free(tbuf);
		}
		else {
			if (nsamp+rleft > buffersize) nsamp=buffersize-rleft;
			nsamp=CopyData(context,rbuf+rleft*nchan,nsamp-rleft,nchan);
			if (nsamp < 0) {
//				TRACE("CoreAudio::Render. CopyData returns %d\n",nsamp);
				break;
			}
			nsamp += rleft;
			rleft=0;
		}
		for (int i=0;i<nsamp;i++) {
			for (int j=0;j<m_render_wvf.Format.nChannels;j++) {
				if (j<nchan)
					buf[m_render_wvf.Format.nChannels*i+j] = rbuf[nchan*i+j];
				else
					buf[m_render_wvf.Format.nChannels*i+j] = rbuf[nchan*i];
			}
		}
//		TRACE("buffered %d rleft=%d\n",nsamp,rleft);
		if (render_mode) convertBuffer(buf,pData,m_render_wvf_ex.Format.nChannels*nsamp,m_render_exmodebits,0);
		hr = pRenderClient->ReleaseBuffer(nsamp, 0);
	}

//	TRACE("Render complete\n");

    // Wait for last data in buffer to play before stopping.
    Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC));

	// Stop playing.
    pAudioClient->Stop();
	AvRevertMmThreadCharacteristics(hTask);
	pRenderClient->Release();
	CloseHandle(hNeedDataEvent);
	pAudioClient->Release();

	free(rbuf);
	if (trenbuf) free(trenbuf);

	return 0;
}

int CCoreAudio::process(double srate,int nchan,int buffersize,int (*ProcessData)(void *,float *,float *,int,int),void *context)
{
	HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC/200;
    REFERENCE_TIME hnsActualDuration;
    UINT32 ibufferFrameCount,obufferFrameCount;
    UINT32 inumFramesAvailable,onumFramesAvailable;
	IAudioClient *pAudioCaptureClient = NULL;
	IAudioClient *pAudioRenderClient = NULL;
    IAudioCaptureClient *pCaptureClient = NULL;
    IAudioRenderClient *pRenderClient = NULL;
    BOOL bDone = FALSE;
	BOOL replay_started=FALSE;
    UINT32 packetLength = 0;
    BYTE *pData;
    DWORD flags;
	int		totsamp=0;
	CResample irs1,irs2,ors1,ors2;
	int		capture_mode=0,render_mode=0;
	BYTE *tcapbuf=NULL;
	BYTE *trenbuf=NULL;

//	TRACE("Capture: %d samp/sec %d channels\nProcess: %g samp/sec %d channels\nRender: %d samp/sec %d channels\n",
//		m_capture_wvf.Format.nSamplesPerSec,m_capture_wvf.Format.nChannels,
//		srate,nchan,
//		m_render_wvf.Format.nSamplesPerSec,m_render_wvf.Format.nChannels);

	// input to process resampling
	irs1.init(srate/(double)m_capture_wvf.Format.nSamplesPerSec);
	irs2.init(srate/(double)m_capture_wvf.Format.nSamplesPerSec);

	// process to output resampling
	ors1.init((double)m_render_wvf.Format.nSamplesPerSec/srate);
	ors2.init((double)m_render_wvf.Format.nSamplesPerSec/srate);

	// get buffer space factor
	double spcfactor=1;
	if (srate > m_capture_wvf.Format.nSamplesPerSec) spcfactor=srate/m_capture_wvf.Format.nSamplesPerSec;

	hr = m_pMMCaptureDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioCaptureClient);
	if (hr!=0) {
//		TRACE("Failed to activate audio client\n");
		return(hr);
	}

//	hnsRequestedDuration = 5.0*buffersize*REFTIMES_PER_SEC/srate;
	REFERENCE_TIME p1,p2;
	pAudioCaptureClient->GetDevicePeriod(&p1,&p2);
//	TRACE("Process: capture device period = %d,%d\n",(int)p1,(int)p2);
//	TRACE("Process: requested duration = %d\n",(int)hnsRequestedDuration);

	if (m_capture_exmodebits &&
		((hr = pAudioCaptureClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,hnsRequestedDuration,hnsRequestedDuration, (WAVEFORMATEX *)&m_capture_wvf_ex, NULL))==0)) {
//		TRACE("Process: Opened capture in EXCLUSIVE mode\n");
		capture_mode=1;
	}
	else {
		m_capture_exmodebits=0;
//		TRACE("Process: Capture Exclusive returns %X mode_bits=%d\n",hr,m_capture_exmodebits);
		hr = pAudioCaptureClient->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,hnsRequestedDuration, 0, (WAVEFORMATEX *)&m_capture_wvf, NULL);
//		TRACE("Process: Opened capture in SHARED mode\n");
		capture_mode=0;
	}
	if ((hr!=0)&&(hr!=AUDCLNT_E_ALREADY_INITIALIZED)) {
//		TRACE("Failed to initialise audio client, hr=%X\n",hr);
		return(hr);
	}

	hr = pAudioCaptureClient->GetBufferSize(&ibufferFrameCount);
//	TRACE("Process: Capture bufferSize=%d frames\n",ibufferFrameCount);
	HANDLE hHaveDataEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = pAudioCaptureClient->SetEventHandle(hHaveDataEvent);
	hr = pAudioCaptureClient->GetService(__uuidof(IAudioCaptureClient),(void**)&pCaptureClient);

    hnsActualDuration = (REFERENCE_TIME)((double)REFTIMES_PER_SEC * ibufferFrameCount / m_capture_wvf.Format.nSamplesPerSec);

	hr = m_pMMRenderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioRenderClient);
	if (hr!=0) {
//		TRACE("Process: Failed to activate audio client\n");
		return(hr);
	}

	pAudioRenderClient->GetDevicePeriod(&p1,&p2);
//	TRACE("Process: render device period = %d,%d\n",(int)p1,(int)p2);

	if (m_render_exmodebits &&
		((hr = pAudioRenderClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,p2,p2, (WAVEFORMATEX *)&m_render_wvf_ex, NULL))==0)) {
//		TRACE("Process: Opened render in EXCLUSIVE mode\n");
		render_mode=1;
	}
	else {
		m_render_exmodebits=0;
		hr = pAudioRenderClient->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,hnsRequestedDuration, 0, (WAVEFORMATEX *)&m_render_wvf, NULL);
//		TRACE("Process: Opened render in SHARED mode\n");
		render_mode=0;
	}
	if ((hr!=0)&&(hr!=AUDCLNT_E_ALREADY_INITIALIZED)) {
//		TRACE("Process: Failed to initialise audio client, hr=%X\n",hr);
		return(hr);
	}

	// get some buffers
	hr = pAudioRenderClient->GetBufferSize(&obufferFrameCount);
//	TRACE("Process: Render bufferSize=%d frames\n",obufferFrameCount);

	float *wbuf = (float *)calloc((int)(20*spcfactor*obufferFrameCount*nchan),sizeof(float));
	float *obuf = (float *)calloc((int)(20*spcfactor*obufferFrameCount*m_render_wvf.Format.nChannels),sizeof(float));
	float *tbuf = (float *)calloc((int)(20*spcfactor*obufferFrameCount*m_render_wvf.Format.nChannels),sizeof(float));
	int oleft=0;

	if (capture_mode) tcapbuf = (BYTE *)calloc(ibufferFrameCount,sizeof(float));
	if (render_mode) trenbuf = (BYTE *)calloc(obufferFrameCount,sizeof(float));

	HANDLE hNeedDataEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = pAudioRenderClient->SetEventHandle(hNeedDataEvent);
	hr = pAudioRenderClient->GetService(__uuidof(IAudioRenderClient),(void**)&pRenderClient);

	// register with MMCSS
    DWORD nTaskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristics(_T("Audio"), &nTaskIndex);

    hr = pAudioCaptureClient->Start();  // Start recording.
	if (hr!=0) return(hr);

	// experiment with starting replay immediately, rather than at the start of the first capture block
	hr = pAudioRenderClient->Start();
	replay_started = TRUE;

	// Each loop fills about half of the shared buffer.
    while (bDone == FALSE)
    {
		// wait for data to become available
        DWORD retval = WaitForSingleObject(hHaveDataEvent, 2000);
		if (retval != WAIT_OBJECT_0) {
            // Event handle timed out after a 2-second wait.
//			TRACE("Event timed out\n");
			break;
        }

        hr = pCaptureClient->GetNextPacketSize(&packetLength);

        while ((!bDone) && (packetLength != 0)) {

			// Get the available data in the shared buffer.
			inumFramesAvailable=0;
            hr = pCaptureClient->GetBuffer(&pData,&inumFramesAvailable,&flags, NULL, NULL);
			float *buf=(float *)pData;
			float *rbuf=buf;
			int		nsamp=inumFramesAvailable;

			if (capture_mode) {
				convertBuffer(buf,tcapbuf,m_capture_wvf_ex.Format.nChannels*nsamp,m_capture_exmodebits,1);
				rbuf=(float *)tcapbuf;
			}

//			TRACE("icnt=%d oleft=%d\n",nsamp,oleft);

			if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {

				// map number of channels to requirements
				if (m_capture_wvf.Format.nChannels != nchan) {
					if ((m_capture_wvf.Format.nChannels==1) && (nchan==2)) {
						rbuf=(float *)calloc(2*nsamp,sizeof(float));
						for (int i=0;i<nsamp;i++) {
							rbuf[2*i] = buf[i];
							rbuf[2*i+1] = buf[i];
						}
					}
					else if ((m_capture_wvf.Format.nChannels==2) && (nchan==1)) {
						rbuf=(float *)calloc(nsamp,sizeof(float));
						for (int i=0;i<nsamp;i++) {
							rbuf[i] = (buf[2*i]+buf[2*i+1])/2;
						}
					}
				}

				// perform resampling if required
				if ((int)srate != m_capture_wvf.Format.nSamplesPerSec) {
					double factor = srate/m_capture_wvf.Format.nSamplesPerSec;
					int ncnt = (int)(0.5+nsamp*factor);
					float *tbuf=(float *)calloc(100+ncnt*nchan,sizeof(float));
					if (nchan==2) {
						uninterleave(rbuf-1,2*nsamp);
						int	nused;
						int ocnt=irs1.process(rbuf,nsamp,0,&nused,tbuf,ncnt);
						irs2.process(rbuf+nsamp,nsamp,0,&nused,tbuf+ocnt,ncnt);
						interleave(tbuf-1,2*ocnt);
						nsamp=ocnt;
						if (rbuf!=buf) free(rbuf);
						rbuf=tbuf;
					}
					else {
						int	nused;
						int ocnt=irs1.process(rbuf,nsamp,0,&nused,tbuf,ncnt);
						nsamp=ocnt;
						if (rbuf!=buf) free(rbuf);
						rbuf=tbuf;
					}
				}

				// Allow caller to process the block
				int n = (*ProcessData)(context,rbuf,wbuf,nsamp,nchan);
				if ((rbuf!=buf)&&(rbuf!=(float *)tcapbuf)) free(rbuf);
				if (n < 0) { bDone=TRUE; break; }
				float *wobuf=wbuf;
				int   nocnt=nsamp;

				// perform resampling if required to match render
				if ((int)srate != m_render_wvf.Format.nSamplesPerSec) {
					double factor = m_render_wvf.Format.nSamplesPerSec/srate;
					int ncnt = (int)(0.5+nocnt*factor);
					float *otbuf=(float *)calloc(100+ncnt*nchan,sizeof(float));
					if (nchan==2) {
						uninterleave(wobuf-1,2*nocnt);
						int	nused;
						int ocnt=ors1.process(wobuf,nocnt,0,&nused,otbuf,ncnt);
						ors2.process(wobuf+nocnt,nocnt,0,&nused,otbuf+ocnt,ncnt);
						interleave(otbuf-1,2*ocnt);
						nocnt=ocnt;
						wobuf=otbuf;
					}
					else {
						int	nused;
						int ocnt=ors1.process(wobuf,nocnt,0,&nused,otbuf,ncnt);
						nocnt=ocnt;
						wobuf=otbuf;
					}
				}

				// add processed buffer to output buffer
				for (int i=0;i<nocnt;i++) {
					if (oleft+i >= (int)(2*obufferFrameCount)) {
						// internal buffer full - drop half of it
						memmove(obuf,obuf+obufferFrameCount*m_render_wvf.Format.nChannels,obufferFrameCount*m_render_wvf.Format.nChannels*sizeof(float));
						oleft -= obufferFrameCount;
					}
					if ((m_render_wvf.Format.nChannels==2)&&(nchan==1)) {
						obuf[2*(oleft+i)]=wobuf[i];
						obuf[2*(oleft+i)+1]=wobuf[i];
					}
					else if ((m_render_wvf.Format.nChannels==1) && (nchan==2)) {
						obuf[oleft+i] = (wobuf[2*i]+wobuf[2*i+1])/2;
					}
					else if (m_render_wvf.Format.nChannels==2) {
						obuf[2*(oleft+i)] = wobuf[2*i];
						obuf[2*(oleft+i)+1] = wobuf[2*i+1];
					}
					else {
						obuf[oleft+i] = wobuf[i];
					}
				}
				oleft+=nocnt;
				if (wobuf!=wbuf) free(wobuf);

		        // ask for enough space for Grab all the available space in the shared buffer.
				if (replay_started) {
					UINT32 numFramesPadding;
			        hr = pAudioRenderClient->GetCurrentPadding(&numFramesPadding);
					onumFramesAvailable = obufferFrameCount - numFramesPadding;
				} else {
					onumFramesAvailable = obufferFrameCount - totsamp;
				}
				if ((int)onumFramesAvailable > oleft) onumFramesAvailable=oleft;
//				TRACE("GetBuffer(%d)\n",numFramesAvailable);
				hr = pRenderClient->GetBuffer(onumFramesAvailable, &pData);
				if (hr!=0) {
//					TRACE("RenderClient GetBuffer() fails\n");
					bDone=TRUE;
					break;
				}
				buf=(render_mode) ? (float *)trenbuf : (float *)pData;

				int ocnt=0;
				for (int i=0;(i<(int)onumFramesAvailable)&&(i<oleft);i++) {
					for (int j=0;j<m_render_wvf.Format.nChannels;j++) {
						buf[m_render_wvf.Format.nChannels*i+j] = obuf[m_render_wvf.Format.nChannels*i+j];
					}
					ocnt++;
				}
				if (render_mode) convertBuffer(buf,pData,m_render_wvf_ex.Format.nChannels*ocnt,m_render_exmodebits,0);
				hr = pRenderClient->ReleaseBuffer(ocnt, 0);

//				TRACE("totsamp=%d ocnt=%d oleft=%d\n",totsamp,ocnt,oleft);
				// shift any remaining samples for next time
				memmove(obuf,obuf+ocnt*m_render_wvf.Format.nChannels,(oleft-ocnt)*m_render_wvf.Format.nChannels*sizeof(float));
				oleft -= ocnt;
				totsamp += ocnt;

				if (!replay_started && (totsamp > buffersize)) {
					// Start playing.
//					TRACE("Start play, totsamp=%d\n",totsamp);
				    hr = pAudioRenderClient->Start();
					replay_started = TRUE;
				}
			}

            hr = pCaptureClient->ReleaseBuffer(inumFramesAvailable);
            hr = pCaptureClient->GetNextPacketSize(&packetLength);
        }
    }

    pAudioCaptureClient->Stop();  // Stop recording.
	pCaptureClient->Release();
	CloseHandle(hHaveDataEvent);
	pAudioCaptureClient->Release();

	// Wait for last data in buffer to play before stopping.
    Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC));
	AvRevertMmThreadCharacteristics(hTask);

    pAudioRenderClient->Stop();
	pRenderClient->Release();
	CloseHandle(hNeedDataEvent);
	pAudioRenderClient->Release();

	free(wbuf);
	free(obuf);
	free(tbuf);
	if (tcapbuf) free(tcapbuf);
	if (trenbuf) free(trenbuf);

	irs1.close();
	irs2.close();
	ors1.close();
	ors2.close();

	return 0;
}

int CCoreAudio::patch(int (*ProcessData)(void *,float *,int,int),void *context)
{
	HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC/100;
    REFERENCE_TIME hnsActualDuration;
    UINT32 ibufferFrameCount,obufferFrameCount;
    UINT32 inumFramesAvailable,onumFramesAvailable;
	IAudioClient *pAudioCaptureClient = NULL;
	IAudioClient *pAudioRenderClient = NULL;
    IAudioCaptureClient *pCaptureClient = NULL;
    IAudioRenderClient *pRenderClient = NULL;
    BOOL bDone = FALSE;
	BOOL replay_started=FALSE;
    UINT32 packetLength = 0;
    BYTE *pData;
    DWORD flags;
	int		totsamp=0;
	CResample rs1,rs2;
	int		capture_mode=0,render_mode=0;
	BYTE *tcapbuf=NULL;
	BYTE *trenbuf=NULL;

	double factor = (double)m_render_wvf.Format.nSamplesPerSec/(double)m_capture_wvf.Format.nSamplesPerSec;
	rs1.init(factor);
	rs2.init(factor);

//	TRACE("PATCH started %d -> %d samples/sec %d -> %d channels\n",
//		m_capture_wvf.Format.nSamplesPerSec,
//		m_render_wvf.Format.nSamplesPerSec,
//		m_capture_wvf.Format.nChannels,
//		m_render_wvf.Format.nChannels);

	hr = m_pMMCaptureDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioCaptureClient);
	if (hr!=0) {
//		TRACE("Failed to activate audio client\n");
		return(hr);
	}

//	hnsRequestedDuration = 5.0*buffersize*REFTIMES_PER_SEC/srate;
	REFERENCE_TIME p1,p2;
	pAudioCaptureClient->GetDevicePeriod(&p1,&p2);
//	TRACE("Patch: capture device period = %d,%d\n",(int)p1,(int)p2);
//	TRACE("Patch: requested duration = %d\n",(int)hnsRequestedDuration);

	if (m_capture_exmodebits &&
		((hr = pAudioCaptureClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,p2,p2, (WAVEFORMATEX *)&m_capture_wvf_ex, NULL))==0)) {
//		TRACE("Patch: Opened capture in EXCLUSIVE mode\n");
		capture_mode=1;
	}
	else {
		m_capture_exmodebits=0;
		hr = pAudioCaptureClient->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,hnsRequestedDuration, 0, (WAVEFORMATEX *)&m_capture_wvf, NULL);
//		TRACE("Patch: Opened capture in SHARED mode\n",hr);
		capture_mode=0;
	}
	if ((hr!=0)&&(hr!=AUDCLNT_E_ALREADY_INITIALIZED)) {
//		TRACE("Failed to initialise audio client\n");
		return(hr);
	}

	hr = pAudioCaptureClient->GetBufferSize(&ibufferFrameCount);
//	TRACE("Patch: iBufferSize=%d frames\n",ibufferFrameCount);
	HANDLE hHaveDataEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = pAudioCaptureClient->SetEventHandle(hHaveDataEvent);
	hr = pAudioCaptureClient->GetService(__uuidof(IAudioCaptureClient),(void**)&pCaptureClient);

    hnsActualDuration = (REFERENCE_TIME)((double)REFTIMES_PER_SEC * ibufferFrameCount / m_capture_wvf.Format.nSamplesPerSec);

	hr = m_pMMRenderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,NULL, (void**)&pAudioRenderClient);
	if (hr!=0) {
//		TRACE("Failed to activate audio client\n");
		return(hr);
	}

	if (m_render_exmodebits &&
		((hr = pAudioRenderClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,p2,p2, (WAVEFORMATEX *)&m_render_wvf_ex, NULL))==0)) {
//		TRACE("Patch: Opened render in EXCLUSIVE mode\n");
		render_mode=1;
	}
	else {
		m_render_exmodebits=0;
		hr = pAudioRenderClient->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,hnsRequestedDuration, 0, (WAVEFORMATEX *)&m_render_wvf, NULL);
//		TRACE("Patch: Opened render in SHARED mode\n");
		render_mode=0;
	}
	if ((hr!=0)&&(hr!=AUDCLNT_E_ALREADY_INITIALIZED)) {
//		TRACE("Failed to initialise audio client\n");
		return(hr);
	}

	obufferFrameCount = (int)(0.5+factor*ibufferFrameCount);
//	TRACE("Patch: oBufferSize=%d frames\n",obufferFrameCount);
	float *obuf = (float *)calloc(2*obufferFrameCount*m_render_wvf.Format.nChannels,sizeof(float));
	int oleft=0;

	if (capture_mode) tcapbuf = (BYTE *)calloc(ibufferFrameCount,sizeof(float));
	if (render_mode) trenbuf = (BYTE *)calloc(obufferFrameCount,sizeof(float));

	HANDLE hNeedDataEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = pAudioRenderClient->SetEventHandle(hNeedDataEvent);
	hr = pAudioRenderClient->GetService(__uuidof(IAudioRenderClient),(void**)&pRenderClient);

	// register with MMCSS
    DWORD nTaskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristics(_T("Audio"), &nTaskIndex);

	// start recording
    hr = pAudioCaptureClient->Start();
	if (hr!=0) return(hr);

	// start replaying
//	hr = pAudioRenderClient->Start();
//	if (hr!=0) return(hr);
//	replay_started = TRUE;

	// Each loop fills about half of the shared buffer.
    while (bDone == FALSE)
    {
		// wait for data to become available
        DWORD retval = WaitForSingleObject(hHaveDataEvent, 2000);
		if (retval != WAIT_OBJECT_0) {
            // Event handle timed out after a 2-second wait.
//			TRACE("Event timed out\n");
			break;
        }

        hr = pCaptureClient->GetNextPacketSize(&packetLength);

        while ((!bDone) && (packetLength != 0)) {

			// Get the available data in the shared buffer.
			inumFramesAvailable=0;
            hr = pCaptureClient->GetBuffer(&pData,&inumFramesAvailable,&flags, NULL, NULL);
//			if (hr!=0) TRACE("GetBuffer returns %X numf=%d flags=%d\n",hr,inumFramesAvailable,flags);

//AUDCLNT_S_BUFFER_EMPTY
//AUDCLNT_E_BUFFER_ERROR
//AUDCLNT_E_OUT_OF_ORDER
//AUDCLNT_E_DEVICE_INVALIDATED
//AUDCLNT_E_BUFFER_OPERATION_PENDING
//AUDCLNT_E_SERVICE_NOT_RUNNING

			float *buf=(float *)pData;
			float *rbuf=buf;
			int		nsamp=inumFramesAvailable;

			if (capture_mode) {
				// convert to floats
				convertBuffer(buf,tcapbuf,m_capture_wvf_ex.Format.nChannels*nsamp,m_capture_exmodebits,1);
				rbuf=(float *)tcapbuf;
			}

//			TRACE("icnt=%d oleft=%d\n",nsamp,oleft);

			if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {

				// Allow caller to process the block
				int n = (*ProcessData)(context,rbuf,nsamp,m_capture_wvf_ex.Format.nChannels);
				if (n < 0) { bDone=TRUE; break; }

				// perform resampling if required
				if (m_render_wvf.Format.nSamplesPerSec != m_capture_wvf.Format.nSamplesPerSec) {
					double factor = (double)m_render_wvf.Format.nSamplesPerSec/(double)m_capture_wvf.Format.nSamplesPerSec;
					int ncnt = (int)(0.5+nsamp*factor);
					float *tbuf=(float *)calloc(100+ncnt*2,sizeof(float));
					if (m_capture_wvf.Format.nChannels==2) {
						uninterleave(rbuf-1,2*nsamp);
						int	nused;
						int ocnt=rs1.process(rbuf,nsamp,0,&nused,tbuf,ncnt);
						rs2.process(rbuf+nsamp,nsamp,0,&nused,tbuf+ocnt,ncnt);
						interleave(tbuf-1,2*ocnt);
						nsamp=ocnt;
						rbuf=tbuf;
					}
					else {
						int	nused;
						int ocnt=rs1.process(rbuf,nsamp,0,&nused,tbuf,ncnt);
//						TRACE("resamp: nsamp=%d nused=%d ncnt=%d ocnt=%d\n",nsamp,nused,ncnt,ocnt);
						nsamp=ocnt;
						rbuf=tbuf;
					}
				}

				// add processed buffer to output buffer
				for (int i=0;i<nsamp;i++) {
					if (oleft+i >= (int)(2*obufferFrameCount)) {
						// internal buffer full - drop half of it
						memmove(obuf,obuf+obufferFrameCount*m_render_wvf.Format.nChannels,obufferFrameCount*m_render_wvf.Format.nChannels*sizeof(float));
						oleft -= obufferFrameCount;
					}
					if ((m_render_wvf.Format.nChannels==2)&&(m_capture_wvf.Format.nChannels==1)) {
						obuf[2*(oleft+i)]=rbuf[i];
						obuf[2*(oleft+i)+1]=rbuf[i];
					}
					else if ((m_render_wvf.Format.nChannels==1) && (m_capture_wvf.Format.nChannels==2)) {
						obuf[oleft+i] = (rbuf[2*i]+rbuf[2*i+1])/2;
					}
					else if (m_render_wvf.Format.nChannels==2) {
						obuf[2*(oleft+i)] = rbuf[2*i];
						obuf[2*(oleft+i)+1] = rbuf[2*i+1];
					}
					else {
						obuf[oleft+i] = rbuf[i];
					}
				}
				oleft+=nsamp;

				// free read buffer if required
				if ((rbuf!=buf)&&(rbuf!=(float *)tcapbuf)) free(rbuf);

		        // ask for enough space for Grab all the available space in the shared buffer.
				if (replay_started) {
					UINT32 numFramesPadding;
			        hr = pAudioRenderClient->GetCurrentPadding(&numFramesPadding);
					onumFramesAvailable = obufferFrameCount - numFramesPadding;
				} else {
					onumFramesAvailable = obufferFrameCount - totsamp;
				}
				if ((int)onumFramesAvailable > oleft) onumFramesAvailable=oleft;
//				TRACE("GetBuffer(%d)\n",numFramesAvailable);
				hr = pRenderClient->GetBuffer(onumFramesAvailable, &pData);
				if (hr!=0) {
//					TRACE("RenderClient GetBuffer() fails\n");
					bDone=TRUE;
					break;
				}
				float *wbuf=(render_mode) ? (float *)trenbuf : (float *)pData;

				int ocnt=0;
				for (int i=0;(i<(int)onumFramesAvailable)&&(i<oleft);i++) {
					for (int j=0;j<m_render_wvf.Format.nChannels;j++) {
						wbuf[m_render_wvf.Format.nChannels*i+j] = obuf[m_render_wvf.Format.nChannels*i+j];
					}
					ocnt++;
				}
				if (render_mode) {
					convertBuffer(wbuf,pData,m_render_wvf_ex.Format.nChannels*ocnt,m_render_exmodebits,0);
				}
				hr = pRenderClient->ReleaseBuffer(ocnt, 0);

//				TRACE("totsamp=%d ocnt=%d oleft=%d\n",totsamp,ocnt,oleft);
				// shift any remaining samples for next time
				memmove(obuf,obuf+ocnt*m_render_wvf.Format.nChannels,(oleft-ocnt)*m_render_wvf.Format.nChannels*sizeof(float));
				oleft -= ocnt;
				totsamp += ocnt;

				if (!replay_started) {
					// Start playing.
//					TRACE("Start play, totsamp=%d\n",totsamp);
				    hr = pAudioRenderClient->Start();
					replay_started = TRUE;
				}


			}

            hr = pCaptureClient->ReleaseBuffer(inumFramesAvailable);
            hr = pCaptureClient->GetNextPacketSize(&packetLength);
        }
    }

    pAudioCaptureClient->Stop();  // Stop recording.
	pCaptureClient->Release();
	CloseHandle(hHaveDataEvent);
	pAudioCaptureClient->Release();

	// Wait for last data in buffer to play before stopping.
    Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC));
	AvRevertMmThreadCharacteristics(hTask);

    pAudioRenderClient->Stop();
	pRenderClient->Release();
	CloseHandle(hNeedDataEvent);
	pAudioRenderClient->Release();

	free(obuf);
	if (tcapbuf) free(tcapbuf);
	if (trenbuf) free(trenbuf);

	return 0;
}

double CCoreAudio::getCaptureVolume(void)
{
	IAudioEndpointVolume *pVolume = NULL;
	HRESULT hr = m_pMMCaptureDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL,NULL, (void**)&pVolume);
	if (hr!=0) {
//		TRACE("Failed to activate endpoint volume client\n");
		return(0.0);
	}
	float vol1,vol2;
	if (m_capture_wvf.Format.nChannels==2) {
		hr = pVolume->GetChannelVolumeLevelScalar(0,&vol1);
		hr = pVolume->GetChannelVolumeLevelScalar(1,&vol2);
	}
	else {
		hr = pVolume->GetChannelVolumeLevelScalar(0,&vol1);
		vol2=vol1;
	}
//	TRACE("Get volume as a scalar is: %f,%f\n", vol1,vol2);
	pVolume->Release();
	return (vol1+vol2)/2;
}


double CCoreAudio::setCaptureVolume(float level)
{
	IAudioEndpointVolume *pVolume = NULL;
	HRESULT hr = m_pMMCaptureDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL,NULL, (void**)&pVolume);
	if (hr!=0) {
//		TRACE("Failed to activate endpoint volume client\n");
		return(0.0);
	}
	hr = pVolume->SetChannelVolumeLevelScalar(0,level,NULL);
	if (m_capture_wvf.Format.nChannels==2)
		hr = pVolume->SetChannelVolumeLevelScalar(1,level,NULL);
//	TRACE("Set volume as a scalar is: %f\n", level);
	pVolume->Release();
	return level;
}
