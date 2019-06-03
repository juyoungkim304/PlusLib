/*=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/

#include "PlusConfigure.h"

#include "DeckLink.h"

#include <math.h>
#include <string>

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL: }

//----------------------------------------------------------------------------
DeckLink::DeckLink()
{
  IDeckLinkIterator *deckLinkIterator = NULL;
  CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_
  IDeckLinkIterator, (void**)&deckLinkIterator);
  
  IDeckLinkInput::GetDisplayModelIterator();
  
  CoInitialize();
}

DeckLink::~DeckLink()
{
  Disconnect();
  CoUninitialize();
}

PlusStatus DeckLink::Connect()
{
  IDeckLinkInput::EnableVideoInput();
  return PLUS_SUCCESS;
}

void DeckLink::Disconnect()
{
  IDeckLinkInput::DisableVideoInput();
}

void DeckLink::StartRecording()
{
  IDeckLinkInput::SetCallback();
  IDeckLinkInput::StartStreams();
}

void DeckLink::StopRecording()
{
  IDeckLinkInput::StopStreams();

}

bool DeckLink::CheckProbe()
{
	/*Write this function eventually*/
	return true;
}

unsigned char* DeckLink::CaptureFrame()
{
}