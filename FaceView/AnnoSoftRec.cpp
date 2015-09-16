#include "StdAfx.h"
#include "AnnoSoftRec.h"

#pragma comment( lib, "liblipsync_rt.lib" )

CAnnoSoftRec::CAnnoSoftRec(void)
{
	m_latency = 50;
	m_frameRate = 25;
	m_max_phone = 2; // -1 = use articulator default;
	m_window = 250;	  // window size for smoothing ??
	m_pRecog = NULL;
	m_pArt = NULL;
	m_pRecognizedPhonemes = NULL;
	m_pHMM=NULL;
}


CAnnoSoftRec::~CAnnoSoftRec(void)
{
	if (m_pRecog) Close();
	if (m_pHMM) DestroyAccHMM(m_pHMM);
}

// create the recogniser
bool CAnnoSoftRec::Create(LPCTSTR datadir,int gender)
{
	CFile	fp;
	CString	hmmname;

	switch (gender) {
	case 0:
		hmmname.Format("%s\\HiQ40_20_38male.hmm",datadir);
		break;
	case 1:
		hmmname.Format("%s\\HiQ40_20_38female.hmm",datadir);
		break;
	default:
		hmmname.Format("%s\\HiQ40_20_38generic.hmm",datadir);
		break;
	}

	if (!fp.Open((LPCTSTR)hmmname,CFile::modeRead|CFile::typeBinary)) {
		CString msg;
		msg.Format("Unable to open '%s'",(LPCTSTR)hmmname);
		AfxMessageBox((LPCTSTR)msg,MB_ICONEXCLAMATION);
		exit(1);
	}

	int	hmmsize=(int)fp.GetLength();
	char *buf=(char *)malloc(hmmsize);
	fp.Read(buf,hmmsize);
	fp.Close();

	// load the HMM
	if (m_pHMM) DestroyAccHMM(m_pHMM);
	m_pHMM = NULL;
	serror err;
	char szError[256];
	err = CreateAccHMM(buf,hmmsize, szError, &m_pHMM);
	if (err != kNoError) {
		CString msg;
		msg.Format("Unable to load '%s'",(LPCTSTR)hmmname);
		AfxMessageBox((LPCTSTR)msg,MB_ICONEXCLAMATION);
		exit(1);
	}
	free(buf);

	CString	trigname;

	trigname.Format("%s\\anno.trig",datadir);
	err = ::LoadTrigramFileIntoHMM(m_pHMM, (LPCTSTR)trigname, false);
	if (err != kNoError) {
		CString msg;
		msg.Format("Unable to load '%s'",(LPCTSTR)trigname);
		AfxMessageBox((LPCTSTR)msg,MB_ICONEXCLAMATION);
		exit(1);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
serror CAnnoSoftRec::Initialise()
{
	
	serror err = kNoError;
	try
	{
		err = CreateSyncResultsCollection(ISyncResultsCollection::opt_intensity_as_morph, 
			&m_pRecognizedPhonemes);
		if (err) throw(err);

		err = CreatePhonemeArticulator(&m_pArt, 0);
		if (err) throw(err);

		float latencySeconds = (float)m_latency/1000.f;

		err = CreateRealtimePhnRecognizer( 22050, 16, 1, latencySeconds, m_pHMM, &m_pRecog);
		if (err != kNoError) throw (err);
		m_pRecog->SetMarkerWindow(m_window);
		m_pRecog->Start();								
		err = kNoError;
	}
	catch (serror err)
	{
		CString msg;
		msg.Format("Unable to create real-time recogniser");
		AfxMessageBox((LPCTSTR)msg,MB_ICONEXCLAMATION);
	}
	return (err);
}


////////////////////////////////////////////////////////////////////////////
serror CAnnoSoftRec::Process(long mstime,CArray<CVisemeMixture,CVisemeMixture &> &list)
{
	long ms = -1;
	bool bHasMarkers = false;

	if (m_pRecog==NULL) return(1);

	// clear the phonemes.
	m_pRecognizedPhonemes->clear();
	// simple case - no time keeper. Get the current results.
//	bHasMarkers = m_pRecog->GetMicrophoneLipsync(m_pRecognizedPhonemes, m_latency);
	bHasMarkers = m_pRecog->GetMarkersBeforeTime(m_pRecognizedPhonemes, -1 /* mstime-m_latency */);

	if (bHasMarkers) {
		// We have markers, however results are improved using articulations
		// so what we do here is dump the markers into the articulator
		// then we tell the articulator to generate a new marker representing
		// the stuff we just generated.
		m_pArt->GenerateArticulations(m_pRecognizedPhonemes, NULL);
//		for (CPhonemeMixtureArticulation *p=m_pArt->begin();p!=m_pArt->end();p++) {
//			TRACE("art: start=%d stop=%d n=%d ",p->msStart,p->msEnd,p->m_nConstituents);
//			for (int j=0;j<p->m_nConstituents;j++)
//				TRACE(", %s (%g)",p->m_constituents[j].strPhoneme,p->m_constituents[j].weight);
//			TRACE("\n");
//		}
		CPhonemeMixtureArticulation *pQ = m_pArt->Quantize(
			-1 /* start time */, -1 /* end time */, 
			m_max_phone /* number of phonemes in the resultant articulator
							 set to one to achieve only one phone */
		);
		if (pQ) {
			// use the quantized viseme
			//				_print_articulation_marker(*pQ); 
			for (int i=0;i<pQ->m_nConstituents;i++) {
				CVisemeMixture m(pQ->m_constituents[i].strPhoneme,pQ->m_constituents[i].weight);
				list.Add(m);
			}
		}
		else {
			// just use the phoneme
			//				std::cerr << "failed to get articulation" << std::endl;
			//GenerateLipsImmediate(m_pRecognizedPhonemes->begin(), 
            //                      m_pRecognizedPhonemes->end(), true);
		}
	}

	return (0);
}

////////////////////////////////////////////////////////////////////////////
void CAnnoSoftRec::Close()
{
	// if this were multithreaded, these would need to be in a critical section
	if (m_pRecog) {
		m_pRecog->Stop();
		DestroyRealtimePhnRecognizer(m_pRecog);
	}
	m_pRecog = NULL;
	if (m_pRecognizedPhonemes) DestroySyncResultsCollection(m_pRecognizedPhonemes);
	m_pRecognizedPhonemes=NULL;
	if (m_pArt) DestroyPhonemeArticulator(m_pArt);
	m_pArt=NULL;
}

////////////////////////////////////////////////////////////////////////////
//
void CAnnoSoftRec::OnRecord(byte *pData, ulong nBytes)
{
	// This method queues up the bytes from the wave_recorder
	// into the recognition engine. That is, this method called by
	// the recorder with audio data, then pushed the data on to the
	// recognition engine
	if (m_pRecog)
		m_pRecog->WriteBytes(pData, nBytes);
}

