/*=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/

#ifndef DeckLink_h
#define DeckLink_h

#include "igsioCommon.h"

#ifdef _WIN32
#include "DeckLinkAPI_h.h"
#else
#include "DeckLinkAPI.h"
#endif

// STL includes
#include <vector>

class DeckLink : public IDeckLinkInputCallback
{
public:
  DeckLink();
  virtual ~DeckLink();
  
  PlusStatus Connect();
  void Disconnect();
  
  void StartRecording(unsigned int videoModeIndex);
  void StopRecording();

  bool CheckProbe();
  ///unsigned char* CaptureFrame();

private:
  HRESULT result;
  IDeckLink* deckLink;
  IDeckLinkIterator* deckLinkIterator;
  IDeckLinkInput* deckLinkInput;
  IDeckLinkDisplayModeIterator* displayModeIterator;
  std::vector<IDeckLinkDisplayMode*> modeList;

 };
#endif //DeckLink