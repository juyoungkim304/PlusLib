/*=Plus=header=begin======================================================
Program: Plus
Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
See License.txt for details.
=========================================================Plus=header=end*/
#include "PlusConfigure.h"
#include "vtkPlusIntelRSVideoSource.h"

#include "vtkMath.h"
#include "vtkMatrix4x4.h"

#include "librealsense2/rs.hpp"

#define DEFAULT_FRAME_RATE            60
#define SR300_MAX_FRAME_RATE          30    // for optical && depth
#define SR300_MAX_OPTICAL_WIDTH       1920
#define SR300_MAX_OPTICAL_HEIGHT      1080
#define SR300_DEFAULT_OPTICAL_WIDTH   640
#define SR300_DEFAULT_OPTICAL_HEIGHT  480
#define SR300_MAX_DEPTH_WIDTH         640
#define SR300_MAX_DEPTH_HEIGHT        480
#define SR300_DEFAULT_DEPTH_WIDTH     640
#define SR300_DEFAULT_DEPTH_HEIGHT    480

vtkStandardNewMacro(vtkPlusIntelRSVideoSource);

//----------------------------------------------------------------------------
vtkPlusIntelRSVideoSource::vtkPlusIntelRSVideoSource()
{
  this->RequireImageOrientationInConfiguration = true;

  // for accurate timing
  this->FrameNumber = 0;
  
  // No callback function provided by the device, so the data capture thread will be used to poll the hardware and add new items to the buffer
  this->StartThreadForInternalUpdates=true;
  this->AcquisitionRate = 30;
}

//----------------------------------------------------------------------------
vtkPlusIntelRSVideoSource::~vtkPlusIntelRSVideoSource()
{

}

//----------------------------------------------------------------------------
std::string vtkPlusIntelRSVideoSource::GetSdkVersion()
{
	return "";
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusIntelRSVideoSource::Probe()
{  
  return PLUS_SUCCESS;
} 

//----------------------------------------------------------------------------
PlusStatus vtkPlusIntelRSVideoSource::ReadConfiguration( vtkXMLDataElement* rootConfigElement )
{
  XML_FIND_DEVICE_ELEMENT_REQUIRED_FOR_READING(deviceConfig, rootConfigElement);
  XML_FIND_NESTED_ELEMENT_REQUIRED(dataSourcesElement, deviceConfig, "DataSources");

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusIntelRSVideoSource::WriteConfiguration(vtkXMLDataElement* rootConfigElement)
{
  XML_FIND_DEVICE_ELEMENT_REQUIRED_FOR_WRITING(trackerConfig, rootConfigElement);
  //TODO: This.
  return PLUS_SUCCESS;
} 

//----------------------------------------------------------------------------
PlusStatus vtkPlusIntelRSVideoSource::InternalConnect()
{ 
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusIntelRSVideoSource::InternalDisconnect()
{ 
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusIntelRSVideoSource::InternalStartRecording()
{
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusIntelRSVideoSource::InternalStopRecording()
{
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusIntelRSVideoSource::InternalUpdate()
{
  FrameNumber++;
  LOG_INFO(this->FrameNumber);
  return PLUS_SUCCESS;
}