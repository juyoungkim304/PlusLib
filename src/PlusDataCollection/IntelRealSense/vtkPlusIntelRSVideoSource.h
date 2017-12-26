/*=Plus=header=begin======================================================
  Progra  : Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/

#ifndef __vtkPlusIntelRSVideoSource_h
#define __vtkPlusIntelRSVideoSource_h

#include "vtkPlusDataCollectionExport.h"
#include "vtkPlusDevice.h"

/*!
  \class vtkPlusIntelRSVideoSource
  \brief Interface class to Intel RealSense cameras
  \ingroup PlusLibDataCollection
*/
class vtkPlusDataCollectionExport vtkPlusIntelRSVideoSource : public vtkPlusDevice
{
public:
  /*! Defines whether or not depth stream is used. */
  enum OUTPUT_TYPE
  {
    OPTICAL,
    OPTICAL_AND_DEPTH
  };

  static vtkPlusIntelRSVideoSource *New();
  vtkTypeMacro(vtkPlusIntelRSVideoSource,vtkPlusDevice);

  /*! Hardware device SDK version. */
  virtual std::string GetSdkVersion(); 
 
  virtual bool IsTracker() const { return true; }

  /*!
    Probe to see if the tracking system is present.
  */
  PlusStatus Probe();

  /*!
    Get an update from the tracking system and push the new transforms
    to the tools.  This should only be used within vtkTracker.cxx.
  */
  PlusStatus InternalUpdate(); 
 
  /*! Read IntelRealSenseVideoSource configuration and update the tracker settings accordingly */
  virtual PlusStatus ReadConfiguration( vtkXMLDataElement* config );

  /*! Write current IntelRealSenseVideoSource configuration settings to XML */
  virtual PlusStatus WriteConfiguration(vtkXMLDataElement* rootConfigElement);

  /*! Connect to the tracker hardware */
  PlusStatus InternalConnect();
  /*! Disconnect from the tracker hardware */
  PlusStatus InternalDisconnect();

protected:
  vtkPlusIntelRSVideoSource();
  ~vtkPlusIntelRSVideoSource();

  class vtkInternal;
  vtkInternal* Internal;

  /*!
    Start the tracking system.  The tracking system is brought from
    its ground state into full tracking mode.  The POLARIS will
    only be reset if communication cannot be established without
    a reset.
  */
  PlusStatus InternalStartRecording();

  /*! Stop the tracking system and bring it back to its initial state. */
  PlusStatus InternalStopRecording();


private:
  vtkPlusIntelRSVideoSource(const vtkPlusIntelRSVideoSource&);
  void operator=(const vtkPlusIntelRSVideoSource&);
  
  OUTPUT_TYPE OutputType;

};

#endif
