/*=Plus=header=begin======================================================
Program: Plus
Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
See License.txt for details.
=========================================================Plus=header=end*/

// Local includes
#include "PlusConfigure.h"
#include "vtkPlusIntelRSVideoSource.h"
#include "PixelCodec.h"
#include "vtkPlusDataSource.h"


// VTK includes
#include "vtkMath.h"
#include "vtkMatrix4x4.h"

// STL includes
#include<vector>

// Intel RealSense includes
#include "librealsense2/rs.hpp"

//Todo: For testing
#include <opencv2/opencv.hpp>


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

namespace
{
  class Stream
  {
  public:
    enum STREAM_TYPE
    {
      STREAM_COLOR,
      STREAM_DEPTH,
      STREAM_INFRARED
    };

    Stream(STREAM_TYPE st, std::string sid, int fs[], int fr)
      : StreamType(st)
      , SourceId(sid)
      , FrameRate(fr)
    {
      FrameSize[0] = fs[0];
      FrameSize[1] = fs[1];
      FrameSize[2] = 1;
    };

    STREAM_TYPE StreamType;
    std::string SourceId;
    FrameSizeType FrameSize;
    int FrameRate;

    // Handle for stream's PlusSource
    vtkPlusDataSource* StreamSource;

    // Frame configured to handle stream's data
    PlusVideoFrame* StreamFrame;
  };
}

//----------------------------------------------------------------------------
class vtkPlusIntelRSVideoSource::vtkInternal
{
public:
  vtkPlusIntelRSVideoSource* External;

  vtkInternal(vtkPlusIntelRSVideoSource* external)
    : External(external)
  {
  }

  virtual ~vtkInternal()
  {
  }

  float get_depth_scale(rs2::device dev)
  {
    // Go over the device's sensors
    for (rs2::sensor& sensor : dev.query_sensors())
    {
      // Check if the sensor if a depth sensor
      if (rs2::depth_sensor dpt = sensor.as<rs2::depth_sensor>())
      {
        return dpt.get_depth_scale();
      }
    }
    throw std::runtime_error("Device does not have a depth sensor");
  }

  rs2_stream find_stream_to_align(const std::vector<rs2::stream_profile>& streams)
  {
    //Given a vector of streams, we try to find a depth stream and another stream to align depth with.
    //We prioritize color streams to make the view look better.
    //If color is not available, we take another stream that (other than depth)
    rs2_stream align_to = RS2_STREAM_ANY;
    bool depth_stream_found = false;
    bool color_stream_found = false;
    for (rs2::stream_profile sp : streams)
    {
      rs2_stream profile_stream = sp.stream_type();
      if (profile_stream != RS2_STREAM_DEPTH)
      {
        if (!color_stream_found)         //Prefer color
          align_to = profile_stream;

        if (profile_stream == RS2_STREAM_COLOR)
        {
          color_stream_found = true;
        }
      }
      else
      {
        depth_stream_found = true;
      }
    }

    if (!depth_stream_found)
      throw std::runtime_error("No Depth stream available");

    if (align_to == RS2_STREAM_ANY)
      throw std::runtime_error("No stream found to align with Depth");

    return align_to;
  }

  // camera handles & intrinsics
  rs2::pipeline Pipe;
  rs2::context Context;
  float DepthScale;

  // streams
  std::vector<Stream> Streams;
  rs2::frameset frame;
  rs2::frame color_frame;
};

//----------------------------------------------------------------------------
vtkPlusIntelRSVideoSource::vtkPlusIntelRSVideoSource()
  : Internal(new vtkInternal(this))
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
  for (int nestedElementIndex = 0; nestedElementIndex < dataSourcesElement->GetNumberOfNestedElements(); nestedElementIndex++)
  {
    vtkXMLDataElement* streamDataElement = dataSourcesElement->GetNestedElement(nestedElementIndex);
    if (STRCASECMP(streamDataElement->GetName(), "DataSource") != 0)
    {
      // if this is not a data source element, skip it
      continue;
    }
    if (streamDataElement->GetAttribute("Type") != NULL && STRCASECMP(streamDataElement->GetAttribute("Type"), "Video") != 0)
    {
      // if this is not a Video element, skip it
      continue;
    }

    const char* streamId = streamDataElement->GetAttribute("Id");
    if (streamId == NULL)
    {
      // tool doesn't have ID needed to generate transform
      LOG_ERROR("Failed to initialize IntelRSVideo stream: DataSource Id is missing");
      continue;
    }

    if (streamDataElement->GetAttribute("StreamType") != NULL && streamDataElement->GetAttribute("FrameSize"))
    {
      // read type of stream requested by the user
      Stream::STREAM_TYPE streamType;
      XML_READ_ENUM3_ATTRIBUTE_NONMEMBER_REQUIRED(StreamType, streamType, streamDataElement, "COLOR", Stream::STREAM_COLOR, "DEPTH", Stream::STREAM_DEPTH, "INFRARED", Stream::STREAM_INFRARED);
      // read frame size requested by user, default of (0, 0) is replaced by camera default in InternalConnect()
      int streamFrameSize[2] = { 0, 0 };
      XML_READ_VECTOR_ATTRIBUTE_NONMEMBER_OPTIONAL(int, 2, FrameSize, streamFrameSize, streamDataElement);
      // read frame rate requested by user, default of 0 is replaced by camera default in InternalConnect()
      int streamFrameRate(0);
      XML_READ_SCALAR_ATTRIBUTE_NONMEMBER_OPTIONAL(int, FrameRate, streamFrameRate, streamDataElement);
      // create new stream and add to requested streams
      Stream newStream(streamType, streamId, streamFrameSize, streamFrameRate);
      this->Internal->Streams.push_back(newStream);
    }
    else
    {
      LOG_ERROR("Incorrectly formatted stream data source.");
    }
  }

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
  // check devices connected to computer

  // select connected device to use

  // check user selected parameters against device intrinsics


  //Create a configuration for configuring the pipeline with a non default profile
  rs2::config cfg;

  //Add desired streams to configuration
  cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_RGB8, 30);
  cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);

  //Instruct pipeline to start streaming with the requested configuration
  this->Internal->Pipe.start(cfg);

  // Camera warmup - dropping several first frames to let auto-exposure stabilize
  rs2::frameset frames;
  for (int i = 0; i < 10; i++)
  {
    //Wait for all configured streams to produce a frame
    frames = this->Internal->Pipe.wait_for_frames();
  }

  for (auto& s : this->Internal->Streams)
  {
    if (s.StreamType == Stream::STREAM_COLOR)
    {
      cout << "C" << endl;
      if (GetVideoSource("RGB", s.StreamSource) != PLUS_SUCCESS)
      {
        LOG_ERROR("Intel RS video source failed");
      }
      s.StreamSource->SetPixelType(VTK_UNSIGNED_CHAR);
      s.StreamSource->SetImageType(US_IMG_RGB_COLOR);
      s.StreamSource->SetNumberOfScalarComponents(3);
      s.StreamSource->SetInputFrameSize(s.FrameSize);

      s.StreamFrame = new PlusVideoFrame();
      s.StreamFrame->SetImageOrientation(s.StreamSource->GetInputImageOrientation());
      s.StreamFrame->SetImageType(US_IMAGE_TYPE::US_IMG_RGB_COLOR);
      s.StreamFrame->AllocateFrame(s.FrameSize, VTK_UNSIGNED_CHAR, 3);
      cout << s.StreamFrame->GetScalarPointer() << endl;
    }
    else if (s.StreamType == Stream::STREAM_DEPTH)
    {
      cout << "D" << endl;
    }
    else if (s.StreamType == Stream::STREAM_INFRARED)
    {
      cout << "I" << endl;
    }
  }

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
  LOG_INFO("vtkPlusIntelRSVideoSource::InternalUpdate()");
  const double unfilteredTimestamp = vtkPlusAccurateTimer::GetSystemTime();

  this->Internal->Pipe.poll_for_frames(&this->Internal->frame);

  //Get each frame
  this->Internal->color_frame = this->Internal->frame.get_color_frame();
  //rs2::frame depth_frame = frame.get_depth_frame();

  // Creating OpenCV matrix from IR image
  //cv::Mat col(cv::Size(640, 480), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

  // Display the image in GUI
  //cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);
  //imshow("Display Image", col); 
  //cv::waitKey(0);
  FrameSizeType fs;
  fs[0] = 640;
  fs[1] = 480;
  fs[2] = 1;



  for (auto s : this->Internal->Streams)
  {
    if (s.StreamType == Stream::STREAM_COLOR)
    {    
      // for testing
      PixelCodec::ConvertToBmp24(PixelCodec::ComponentOrder_RGB, PixelCodec::PixelEncoding::PixelEncoding_RGB24, 640, 480, (unsigned char*) this->Internal->color_frame.get_data(), (unsigned char*)s.StreamFrame->GetScalarPointer());
      s.StreamSource->AddItem(s.StreamFrame, FrameNumber, unfilteredTimestamp);
      cout << "COLOR" << endl;
    }
    else if (s.StreamType == Stream::STREAM_DEPTH)
    {
      cout << "DEPTH" << endl;
    }
    else if (s.StreamType == Stream::STREAM_INFRARED)
    {
      cout << "INFRARED" << endl;
    }
  }



  //colorFrame->AllocateFrame(fs, US_IMG_RGB_COLOR, 3);
  for (auto s : this->Internal->Streams)
  {
    

  }



  /*
  //Get each frame
  rs2::frameset frame = this->Internal->Pipe.wait_for_frames();
  rs2::frame color_frame = frame.get_color_frame();

  // Creating OpenCV Matrix from a color image
  cv::Mat color(cv::Size(640, 480), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

  // Display in a GUI
  cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);
  imshow("Display Image", color);

  cv::waitKey(0);
  */


  //vtkPlusDataSource* aSource(NULL);
  //for (int i = 0; i < this->GetNumberOfVideoSources(); ++i)
  //{
  //  if (this->GetVideoSourceByIndex(i, aSource) != PLUS_SUCCESS)
  //  {
  //    LOG_ERROR("Unable to retrieve the video source in the Epiphan device on channel " << (*this->OutputChannels.begin())->GetChannelId());
  //    return PLUS_FAIL;
  //  }
  //  int numberOfScalarComponents(1);
  //  if (aSource->GetImageType() == US_IMG_RGB_COLOR)
  //  {
  //    numberOfScalarComponents = 3;
  //  }
  //  if (aSource->AddItem(frame->pixbuf, aSource->GetInputImageOrientation(), this->FrameSize, VTK_UNSIGNED_CHAR, numberOfScalarComponents, aSource->GetImageType(), 0, this->FrameNumber) != PLUS_SUCCESS)
  //  {
  //    LOG_ERROR("Error adding item to video source " << aSource->GetId() << " on channel " << (*this->OutputChannels.begin())->GetChannelId());
  //    return PLUS_FAIL;
  //  }
  //  else
  //  {
  //    this->Modified();
  //  }
  //}

  //FrmGrab_Release((FrmGrabber*)this->FrameGrabber, frame);


  FrameNumber++;
  LOG_INFO(this->FrameNumber);
  return PLUS_SUCCESS;
}