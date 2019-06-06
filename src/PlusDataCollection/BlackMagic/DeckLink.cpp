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
DeckLink::DeckLink() : result(NULL), deckLinkIterator(NULL), deckLinkInput(NULL), displayModeIterator(NULL)
{
  result = CoInitialize(NULL);
  result = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)& deckLinkIterator);
  result = deckLinkInput -> GetDisplayModeIterator(&displayModeIterator);
}

DeckLink::~DeckLink()
{
  Disconnect();
  CoUninitialize();
}

PlusStatus DeckLink::Connect()
{
  deckLinkInput->SetCallback(this);
  return PLUS_SUCCESS;
}

void DeckLink::Disconnect()
{
  result = deckLinkInput -> DisableVideoInput();
}

void DeckLink::StartRecording(unsigned int videoModeIndex)
{
  BMDVideoInputFlags videoInputFlags = bmdVideoInputFlagDefault;
  result = deckLinkInput->EnableVideoInput(modeList[videoModeIndex]->GetDisplayMode(), bmdFormat8BitYUV, videoInputFlags);
  deckLinkInput -> StartStreams();
}

void DeckLink::StopRecording()
{
  result = deckLinkInput -> StopStreams();
  deckLinkInput->SetScreenPreviewCallback(NULL);
  deckLinkInput->SetCallback(NULL);

}

bool DeckLink::CheckProbe()
{
	/*Write this function eventually*/
	return true;
}

/*
unsigned char* DeckLink::CaptureFrame()
{
}
*/