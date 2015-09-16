#pragma once

#include <iostream>
#include <string>

#include "VisemeMixture.h"

#include "liblipsync.h"
#include "libtranscribe.h"
#include "libarticulator.h"

////////////////////////////////////////////////////////////////////////////
// Name: realtime_phoneme_recognizer
/**@ingroup realtime_console
   @brief 
   This class demonstrates how to interface with IRealtimePhnRecognizer.
   
   This class is initialized with arguments from the application
   It is initialized with a model, and a bunch of optional parameters.
   After intialization, the recognizer is told to start. It will either
   start a recording session or start a playback session.
  
   It will enter a loop which will pull CSyncMarker records
   from the recognizer object at the specified frame rate. It will then
   process these markers through the articulator, and generate the results
   the results will be printed to std::out.

   The purpose is to create a relatively simple class which implements
   a realtime recognizer, without a lot of ui or graphics code to
   clutter things up. The loop is usually going to be implemented in 
   a worker thread, but possibly a service routine would work
  
   The class inherits from two sink types need to get audio data from
   the wave_player and the wave_recorder.
*/

class CAnnoSoftRec
{
public:
	CAnnoSoftRec(void);
	~CAnnoSoftRec(void);

	bool Create(LPCTSTR datadir,int gender);

	// Name: init
	/**brief Initialize the realtime phoneme recognizer
	  
	   This method is called before "loop" it initializes
	   the realtime phoneme recognizer with the necessary information
	   
	  	@param pHMM - [in] HMM model to use
	    @param strAudioFile - [in] for wave realtime playback, the audio file
	  		empty for recording mode. If empty, it is initialized in record mode
	  	@param frameRate - [in] milliseconds per frame. How often phonemes will
	  		be recognized. 
	    @param max_phone - [in] -1 for default. Otherwise, it is the number of phonemes
	  					allowed per mixture per frame. Bitmapped graphics should
	  					set this to 1. otherwise 2 or more if animation platform
	  					can handle CPhonemeMixtureArticulation records
	    @param latency - [in] the amount of latency allowed in the system.
	  		this is a critical variable for microphone recording. It will be in the
			range of 40-80. .
	    @param window_size - [in] the size in ms of the articulation window. This
		effects the smoothness of the results. more = smoother, but perhaps too smooth.
	  	the realtime phoneme recognizer will use this to control how
	    much smoothing. A larger number means more smoothing, a smaller
	    number means less smoothing
    */
	serror Initialise();

	// Name: loop
	/**@brief this method enters a loop pulling phoneme data from the
	   realtime recognizer. 
	   
       Most applications would implement the actual
	   loop in either a service routine, but for utmost simplicity
	   it is done in the main thread.
    */
	serror Process(long mstime,CArray<CVisemeMixture,CVisemeMixture &> &list);

	// Name: stop
	/**@brief this method stops the recognizer and destroys data structures.
	 
        After stop, init must be called again
    */
	void Close();

	/** @brief IWaveRecorderHook::OnRecord override
	
       This method is an override for IWaveRecorderHook::OnRecord
	   This method will be called by the wave_recorder when new audio
	   data has arrived. This method will then queue it up into the recognizer
       @param pData - [in] PCM audio data
       @param nBytes - [in] size in bytes of the pcm data.
    */
	virtual void OnRecord(byte *pData, ulong nBytes);	


protected:

    /// latency of the audio, as specified in init (milliseconds)
	long	m_latency; 
    /// frame rate (milliseconds per frame) as specified in init
	long	m_frameRate; 
    /// number of articulation entries be frame as specified in init
	long	m_max_phone; 
    /** the lookback/lookahead window for the phn recognizer. Produces different
        levels of smoothness. (milliseconds) */
	long	m_window;	 					 
	// HMMs
	CAccousticHMM * m_pHMM;
	/// the recognizer						.
	IRealtimePhnRecognizer* m_pRecog; 
    /// the articulator
	IPhnMixtureArticulator* m_pArt;	  
    /// results collection
	ISyncResultsCollection* m_pRecognizedPhonemes; 

};

