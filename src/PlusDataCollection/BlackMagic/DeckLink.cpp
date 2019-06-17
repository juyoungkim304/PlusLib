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

typedef struct {
	CString vitcF1Timecode;
	CString vitcF1UserBits;
	CString vitcF2Timecode;
	CString vitcF2UserBits;

	CString rp188vitc1Timecode;
	CString rp188vitc1UserBits;
	CString rp188vitc2Timecode;
	CString rp188vitc2UserBits;
	CString rp188ltcTimecode;
	CString rp188ltcUserBits;
} AncillaryDataStruct;

HRESULT 	deckLinkDelegate::VideoInputFrameArrived(/* in */ IDeckLinkVideoInputFrame* videoFrame, /* in */ IDeckLinkAudioInputPacket* audioPacket)
{
	//CHECK THIS CLASS
	AncillaryDataStruct		ancillaryData;

	if (videoFrame == NULL)
		return S_OK;

	// Get the various timecodes and userbits attached to this frame
	GetAncillaryDataFromFrame(videoFrame, bmdTimecodeVITC, &ancillaryData.vitcF1Timecode, &ancillaryData.vitcF1UserBits);
	GetAncillaryDataFromFrame(videoFrame, bmdTimecodeVITCField2, &ancillaryData.vitcF2Timecode, &ancillaryData.vitcF2UserBits);
	GetAncillaryDataFromFrame(videoFrame, bmdTimecodeRP188VITC1, &ancillaryData.rp188vitc1Timecode, &ancillaryData.rp188vitc1UserBits);
	GetAncillaryDataFromFrame(videoFrame, bmdTimecodeRP188LTC, &ancillaryData.rp188ltcTimecode, &ancillaryData.rp188ltcUserBits);
	GetAncillaryDataFromFrame(videoFrame, bmdTimecodeRP188VITC2, &ancillaryData.rp188vitc2Timecode, &ancillaryData.rp188vitc2UserBits);

	return S_OK;
}

void	deckLinkDelegate::GetAncillaryDataFromFrame(IDeckLinkVideoInputFrame* videoFrame, BMDTimecodeFormat timecodeFormat, CString* timecodeString, CString* userBitsString)
{
	IDeckLinkTimecode* timecode = NULL;
	BSTR					timecodeBstr;
	BMDTimecodeUserBits		userBits = 0;

	if ((videoFrame != NULL) && (timecodeString != NULL) && (userBitsString != NULL)
		&& (videoFrame->GetTimecode(timecodeFormat, &timecode) == S_OK))
	{
		if (timecode->GetString(&timecodeBstr) == S_OK)
		{
			*timecodeString = timecodeBstr;
			SysFreeString(timecodeBstr);
		}
		else
		{
			*timecodeString = _T("");
		}

		timecode->GetTimecodeUserBits(&userBits);
		userBitsString->Format(_T("0x%08X"), userBits);

		timecode->Release();
	}
	else
	{
		*timecodeString = _T("");
		*userBitsString = _T("");
	}


}