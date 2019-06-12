/*=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/

#include "PlusConfigure.h"

#include "DeckLink.h"

#include <math.h>
#include <string>
#include <comutil.h>

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL: }

//----------------------------------------------------------------------------

deckLinkDelegate::deckLinkDelegate(IDeckLink* device)
{
	myDevice = device;
	refCount = 1;

}

HRESULT	STDMETHODCALLTYPE deckLinkDelegate::QueryInterface(REFIID iid, LPVOID* ppv)
{
	HRESULT			result = E_NOINTERFACE;

	if (ppv == NULL)
		return E_INVALIDARG;

	// Initialise the return result
	*ppv = NULL;

	// Obtain the IUnknown interface and compare it the provided REFIID
	if (iid == IID_IUnknown)
	{
		*ppv = this;
		AddRef();
		result = S_OK;
	}
	else if (iid == IID_IDeckLinkInputCallback)
	{
		*ppv = (IDeckLinkInputCallback*)this;
		AddRef();
		result = S_OK;
	}
	else if (iid == IID_IDeckLinkNotificationCallback)
	{
		*ppv = (IDeckLinkNotificationCallback*)this;
		AddRef();
		result = S_OK;
	}

	return result;
}

ULONG STDMETHODCALLTYPE deckLinkDelegate::AddRef(void)
{
	return InterlockedIncrement((LONG*)& refCount);
}

ULONG STDMETHODCALLTYPE deckLinkDelegate::Release(void)
{
	int		newRefValue;

	newRefValue = InterlockedDecrement((LONG*)& refCount);
	if (newRefValue == 0)
	{
		delete this;
		return 0;
	}

	return newRefValue;
}

HRESULT		deckLinkDelegate::VideoInputFormatChanged(/* in */ BMDVideoInputFormatChangedEvents notificationEvents, /* in */ IDeckLinkDisplayMode* newMode, /* in */ BMDDetectedVideoInputFormatFlags detectedSignalFlags)
{
	return S_OK;
}

HRESULT 	deckLinkDelegate::VideoInputFrameArrived(/* in */ IDeckLinkVideoInputFrame* videoFrame, /* in */ IDeckLinkAudioInputPacket* audioPacket)
{
	//CHECK THIS CLASS
	return S_OK;
}