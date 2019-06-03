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

class DeckLink
{
public:
  DeckLink();
  virtual ~DeckLink();
  
  PlusStatus Connect();
  void Disconnect();
  
  void StartRecording();
  void StopRecording();

  bool CheckProbe();
  unsigned char* CaptureFrame();

 };
#endif //DeckLink