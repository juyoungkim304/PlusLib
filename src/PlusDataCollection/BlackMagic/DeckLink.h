/*=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/

#ifndef DeckLink_h
#define DeckLink_h

#include "igsioCommon.h"
#include "atlstr.h"

#ifdef _WIN32
#include "DeckLinkAPI_h.h"
#else
#include "DeckLinkAPI.h"
#endif

// STL includes
#include <vector>
#include <string>



class deckLinkDelegate : public IDeckLinkInputCallback
{
private:
	int	refCount;
	IDeckLink* myDevice;

public:
	deckLinkDelegate(IDeckLink* device);
	IDeckLinkVideoInputFrame* myFrame;

	virtual HRESULT STDMETHODCALLTYPE	QueryInterface(REFIID iid, LPVOID* ppv);
	virtual ULONG STDMETHODCALLTYPE		AddRef(void);
	virtual ULONG STDMETHODCALLTYPE		Release(void);

	// IDeckLinkInputCallback interface
	virtual HRESULT STDMETHODCALLTYPE	VideoInputFormatChanged(/* in */ BMDVideoInputFormatChangedEvents notificationEvents, /* in */ IDeckLinkDisplayMode* newDisplayMode, /* in */ BMDDetectedVideoInputFormatFlags detectedSignalFlags);
	virtual HRESULT STDMETHODCALLTYPE	VideoInputFrameArrived(/* in */ IDeckLinkVideoInputFrame* videoFrame, /* in */ IDeckLinkAudioInputPacket* audioPacket);

};
#endif //DeckLink