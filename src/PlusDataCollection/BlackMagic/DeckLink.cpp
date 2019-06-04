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
DeckLink::DeckLink() : result(NULL), deckLinkIterator(NULL), deckLinkInput(NULL)
{
  result = CoInitialize(NULL);
  result = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)& deckLinkIterator);
  result = deckLinkInput -> IDeckLinkInput::GetDisplayModeIterator();
}

DeckLink::~DeckLink()
{
  Disconnect();
  CoUninitialize();
}

PlusStatus DeckLink::Connect()
{
  result = deckLinkInput -> IDeckLinkInput::EnableVideoInput();
  return PLUS_SUCCESS;
}

void DeckLink::Disconnect()
{
  result = deckLinkInput -> IDeckLinkInput::DisableVideoInput();
}

void DeckLink::StartRecording()
{
  result = deckLinkInput -> IDeckLinkInput::SetCallback();
  result = deckLinkInput -> IDeckLinkInput::StartStreams();
}

void DeckLink::StopRecording()
{
  result = deckLinkInput -> IDeckLinkInput::StopStreams();

}

bool DeckLink::CheckProbe()
{
	/*Write this function eventually*/
	return true;
}

unsigned char* DeckLink::CaptureFrame()
{
}