// CoreAudio - support for Windows Core Audio API
//
// Mark Huckvale - University College London
//
// version 1 - June 2015
//

#pragma once
class CCoreAudio
{
private:
	IMMDeviceCollection *m_pMMRenderDeviceCollection;	// list of output devices
	IMMDeviceCollection *m_pMMCaptureDeviceCollection;	// list of input devices
	IMMDevice *m_pMMDefaultRenderDevice;				// default output device
	IMMDevice *m_pMMDefaultCaptureDevice;				// default input device
	IMMDevice *m_pMMRenderDevice;						// current output device
	IMMDevice *m_pMMCaptureDevice;						// current input device
	WAVEFORMATEXTENSIBLE	m_render_wvf;				// current output format
	WAVEFORMATEXTENSIBLE	m_capture_wvf;				// current input format
	WAVEFORMATEXTENSIBLE	m_render_wvf_ex;			// current output format exclusive mode
	WAVEFORMATEXTENSIBLE	m_capture_wvf_ex;			// current input format exclusive mode
	int	m_render_exmodebits;			// exclusive mode sample bits
	int	m_capture_exmodebits;			// exclusive mode sample bits

public:
	CCoreAudio(void);
	~CCoreAudio(void);
	// init - initialisation must be performed first
	int init(void);
	// get number of output devices
	int getNumRenderDevices();
	// get number of input devices
	int getNumCaptureDevices();
	// get output device name - Unicode (-1=default)
	int getRenderDeviceNameW(int id,wchar_t *buf,int bufsize);
	// get input device name - Unicode (-1=default)
	int getCaptureDeviceNameW(int id,wchar_t *buf,int bufsize);
	// get output device name (-1=default)
	int getRenderDeviceName(int id,char *buf,int bufsize);
	// get input device name (-1=default)
	int getCaptureDeviceName(int id,char *buf,int bufsize);
	// get output device mixer format (-1=default)
	WAVEFORMATEXTENSIBLE * getRenderDeviceFormat(int id);
	// get input device mixer format (-1=default)
	WAVEFORMATEXTENSIBLE * getCaptureDeviceFormat(int id);
	// check output device supports format (-1=default)
	int checkRenderDeviceFormat(int id,int nchan,int nbits,int srate);
	// check input device supports format (-1=default)
	int checkCaptureDeviceFormat(int id,int nchan,int nbits,int srate);
	// select output device (-1=default)
	int setRenderDevice(int id);
	// select input device (-1=default)
	int setCaptureDevice(int id);
	// play memory buffer through current output device
	int playBuffer(float *buf,int nsamp,double srate,int nchan);
	// resample memory buffer
	int resample(float *buf,int nsamp,double srate,int nchan,float *obuf,int osamp,double orate);
	// record through callback function (returns -1 to halt recording)
	int capture(double srate,int nchan,int buffersize,int (*)(void *context,float *buf,int nsamp,int nchan),void *context);
	// play through callback function (returns -1 to halt playback)
	int render(double srate,int nchan,int buffersize,int (*)(void *context,float *buf,int nsamp,int nchan),void *context);
	// record and play through callback function (return -1 to halt)
	int process(double srate,int nchan,int buffersize,int (*)(void *context,float *ibuf,float *obuf,int nsamp,int nchan),void *context);
	// directly patch input device to output device with callback to monitor (returns -1 to halt)
	int patch(int (*)(void *context,float *ibuf,int nsamp,int nchan),void *context);
	// request use of exclusive mode for playback
	int setRenderExclusiveMode(int mode);
	// request use of exclusive mode for recording
	int setCaptureExclusiveMode(int mode);
	// get status of exclusive mode for playback
	int getRenderExclusiveMode();
	// get status of exclusive mode for recording
	int getCaptureExclusiveMode();
	// convert buffer across sample formats
	void convertBuffer(void *src,void *dst,int nsamp,int nbits,int dir);
	// get record volume
	double getCaptureVolume(void);
	// set record volume
	double setCaptureVolume(float level);
	// simple get output sample rate
	int getRenderSampleRate() { return m_render_wvf.Format.nSamplesPerSec; }
	// simple get output channels
	int getRenderNumChannels()  { return m_render_wvf.Format.nChannels; }
	// simple get input sample rate
	int getCaptureSampleRate()  { return m_capture_wvf.Format.nSamplesPerSec; }
	// simple get inpt channels
	int getCaptureNumChannels()  { return m_capture_wvf.Format.nChannels; }
protected:
	// reverse part of a buffer
	void reverse(float *buf,int spos,int len);
	// interleave a buffer
	void interleave(float *buf,int len);
	// uninterleave a buffer
	void uninterleave(float *buf,int len);
};

