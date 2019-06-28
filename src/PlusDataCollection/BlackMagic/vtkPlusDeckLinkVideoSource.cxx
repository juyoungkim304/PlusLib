/*=Plus=header=begin======================================================
Program: Plus
Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
See License.txt for details.
=========================================================Plus=header=end*/

#include "PlusConfigure.h"

// Local includes
#include "vtkPlusDeckLinkVideoSource.h"
#include "vtkPlusDataSource.h"
#include "vtkPlusChannel.h"
#include "vtkPlusUsImagingParameters.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkImageImport.h>
#include <vtkObjectFactory.h>

// System includes
#include <string>

//----------------------------------------------------------------------------
// vtkPlusDeckLinkVideoSource::vtkInternal
//----------------------------------------------------------------------------

class vtkPlusDeckLinkVideoSource::vtkInternal
{
public:
  vtkPlusDeckLinkVideoSource* External;

  vtkInternal(vtkPlusDeckLinkVideoSource* external)
    : External(external)
  {
  }

  virtual ~vtkInternal()
  {
  }

};

//----------------------------------------------------------------------------
// vtkPlusDeckLinkVideoSource
//----------------------------------------------------------------------------

vtkStandardNewMacro(vtkPlusDeckLinkVideoSource);

//----------------------------------------------------------------------------
vtkPlusDeckLinkVideoSource::vtkPlusDeckLinkVideoSource()
  : vtkPlusDevice()
  , Internal(new vtkInternal(this)), refCount(0)
{
  LOG_TRACE("vtkPlusDeckLinkVideoSource::vtkPlusDeckLinkVideoSource()");

	//setting up deckLinkIterator to access API
	IDeckLinkIterator* deckLinkIterator;
	result = CoInitialize(NULL);
	result = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)& deckLinkIterator);

	//iterator creates object representing DeckLink device and assigns address to deckLink
	if (deckLinkIterator->Next(&deckLink) == S_OK)
	{
		//deckLink queried to obtain input interface
		result = deckLink->QueryInterface(IID_IDeckLinkInput, (void**)& deckLinkInput);
		if (result != S_OK)
		{
			fprintf(stderr, "Failed to find input.\n");
		}
	}

	//commented out delegate class
	//test = new deckLinkDelegate(deckLink);

  this->FrameNumber = 0;
  this->StartThreadForInternalUpdates = true;
  this->InternalUpdateRate = 30;
	myFrame = new void* ();

}

//----------------------------------------------------------------------------
vtkPlusDeckLinkVideoSource::~vtkPlusDeckLinkVideoSource()
{
  LOG_TRACE("vtkPlusDeckLinkVideoSource::~vtkPlusDeckLinkVideoSource()");
  delete Internal;
  Internal = nullptr;
}

//----------------------------------------------------------------------------
void vtkPlusDeckLinkVideoSource::PrintSelf(ostream& os, vtkIndent indent)
{
  LOG_TRACE("vtkPlusDeckLinkVideoSource::PrintSelf(ostream& os, vtkIndent indent)");
  this -> Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDeckLinkVideoSource::ReadConfiguration(vtkXMLDataElement* rootConfigElement)
{
  LOG_TRACE("vtkPlusDeckLinkVideoSource::ReadConfiguration");

  XML_FIND_DEVICE_ELEMENT_REQUIRED_FOR_READING(deviceConfig, rootConfigElement);

  XML_FIND_NESTED_ELEMENT_REQUIRED(dataSourcesElement, deviceConfig, "DataSources");
  
  for (int nestedElementIndex = 0; nestedElementIndex < dataSourcesElement->GetNumberOfNestedElements(); nestedElementIndex++)
  {
    vtkXMLDataElement* toolDataElement = dataSourcesElement->GetNestedElement(nestedElementIndex);
    if (STRCASECMP(toolDataElement->GetName(), "DataSource") != 0)
    {
      // if this is not a data source element, skip it
      continue;
    }
    if (toolDataElement->GetAttribute("Type") != NULL && STRCASECMP(toolDataElement->GetAttribute("Type"), "Video") != 0)
    {
      // if this is not a Tool element, skip it
      continue;
    }
    
  }

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDeckLinkVideoSource::WriteConfiguration(vtkXMLDataElement* rootConfigElement)
{
  LOG_TRACE("vtkPlusDeckLinkVideoSource::WriteConfiguration");
  XML_FIND_DEVICE_ELEMENT_REQUIRED_FOR_WRITING(deviceConfig, rootConfigElement);
  
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDeckLinkVideoSource::InternalConnect()
{
  LOG_TRACE("vtkPlusDeckLinkVideoSource::InternalConnect");
	//deckLinkInput->SetScreenPreviewCallback(screenPreviewCallback);

	//deckLinkInput registers callback that receives video frames in push model
	deckLinkInput->SetCallback(this);

	//starts input and streams with preset video settings and no audio
	result = deckLinkInput->EnableVideoInput(bmdModeHD1080i5994, bmdFormat8BitYUV, bmdVideoInputEnableFormatDetection);
	result = deckLinkInput->DisableAudioInput();
	result = deckLinkInput->StartStreams();
  if (result != S_OK)
  {
    LOG_ERROR("vtkPlusDeckLinkVideoSource device initialization failed");
    this->ConnectedToDevice = false;
    return PLUS_FAIL;
  }
  this->ConnectedToDevice = true;

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDeckLinkVideoSource::InternalDisconnect()
{
  LOG_TRACE("vtkPlusDeckLinkVideoSource::InternalDisconnect");
  LOG_DEBUG("Disconnect from DeckLink");

	//stops stream and ends callback
	deckLinkInput->StopStreams();
	//deckLinkInput->SetScreenPreviewCallback(NULL);
	deckLinkInput->SetCallback(NULL);
  this->ConnectedToDevice = false;
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDeckLinkVideoSource::InternalStartRecording()
{
  LOG_TRACE("vtkPlusDeckLinkVideoSource::InternalStartRecording");

	//starts stream only
	result = deckLinkInput->StartStreams();
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDeckLinkVideoSource::InternalStopRecording()
{
  LOG_TRACE("vtkPlusDeckLinkVideoSource::InternalStopRecording");

	//ends stream only
	deckLinkInput->StopStreams();
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDeckLinkVideoSource::Probe()
{
  LOG_TRACE("vtkPlusDeckLinkVideoSource::Probe");
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDeckLinkVideoSource::InternalUpdate()
{
  LOG_TRACE("vtkPlusDeckLinkVideoSource::InternalUpdate");

	//see VideoInputFrameArrived for update
  return PLUS_SUCCESS;

}

HRESULT	STDMETHODCALLTYPE vtkPlusDeckLinkVideoSource::QueryInterface(REFIID iid, LPVOID* ppv)
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

ULONG STDMETHODCALLTYPE vtkPlusDeckLinkVideoSource::AddRef(void)
{
	return InterlockedIncrement((LONG*)& refCount);
}

ULONG STDMETHODCALLTYPE vtkPlusDeckLinkVideoSource::Release(void)
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

HRESULT		vtkPlusDeckLinkVideoSource::VideoInputFormatChanged(/* in */ BMDVideoInputFormatChangedEvents notificationEvents, /* in */ IDeckLinkDisplayMode* newMode, /* in */ BMDDetectedVideoInputFormatFlags detectedSignalFlags)
{
	return S_OK;
}

//This function runs automatically for each frame as callback is active
//code for update function is currently here as videoFrame is not held beyond current scope
HRESULT 	vtkPlusDeckLinkVideoSource::VideoInputFrameArrived(/* in */ IDeckLinkVideoInputFrame* videoFrame, /* in */ IDeckLinkAudioInputPacket* audioPacket)
{

	const double toolTimestamp = vtkIGSIOAccurateTimer::GetSystemTime(); // unfiltered timestamp
	const double filteredTimestamp = toolTimestamp;

	//CHECK THIS CLASS
	if (videoFrame == NULL)
		return S_OK;

	this->FrameNumber++;

	FrameSizeType frameSizeInPix = { videoFrame->GetWidth(), videoFrame->GetHeight(), 1 };

	vtkPlusDataSource* aSource = NULL;

	if (this->GetFirstActiveOutputVideoSource(aSource) != PLUS_SUCCESS)

	{
		LOG_ERROR("Unable to retrieve the video source in the Black Magic device.");
		return PLUS_FAIL;
	}

	if (aSource->GetNumberOfItems() == 0)
	{
		LOG_DEBUG("Set up image buffer for Deck Link");
		aSource->SetPixelType(VTK_UNSIGNED_CHAR);
		aSource->SetImageType(US_IMG_BRIGHTNESS);
		aSource->SetInputFrameSize(frameSizeInPix);

		LOG_DEBUG("Frame size: " << frameSizeInPix[0] << "x" << frameSizeInPix[1]
			<< ", pixel type: " << vtkImageScalarTypeNameMacro(aSource->GetPixelType())
			<< ", buffer image orientation: " << igsioVideoFrame::GetStringFromUsImageOrientation(aSource->GetInputImageOrientation()));
	}

	videoFrame->GetBytes(myFrame);
	PlusStatus status = aSource->AddItem(*myFrame, frameSizeInPix, VTK_UNSIGNED_CHAR, US_IMG_BRIGHTNESS, this->FrameNumber, toolTimestamp, filteredTimestamp);
	this->Modified();

	//test: no error thrown
	if (!aSource->GetLatestItemHasValidFieldData())
	{
		LOG_ERROR("NO FIELD DATA");
		return PLUS_FAIL;
	}

	//test: error thrown- no valid transform data
	/*
	if (!aSource->GetLatestItemHasValidTransformData())
	{
		LOG_ERROR("NO TRANSFORM DATA");
		return PLUS_FAIL;
	}
	*/

	//test: no error thrown
	if (!aSource->GetLatestItemHasValidVideoData())
	{
		LOG_ERROR("NO VIDEO DATA");
		return PLUS_FAIL;
	}
	return S_OK;
}
