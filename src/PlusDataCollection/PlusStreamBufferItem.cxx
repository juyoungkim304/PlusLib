/*=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/

#include "PlusConfigure.h"
#include "PlusStreamBufferItem.h"
#include "vtkMatrix4x4.h"

//----------------------------------------------------------------------------
//            DataBufferItem
//----------------------------------------------------------------------------
StreamBufferItem::StreamBufferItem()
  : FilteredTimeStamp( 0 )
  , UnfilteredTimeStamp( 0 )
  , Index( 0 )
  , Uid( 0 )
  , ValidTransformData( false )
  , Matrix( vtkSmartPointer<vtkMatrix4x4>::New() )
  , Status( TOOL_OK )
{
}

//----------------------------------------------------------------------------
StreamBufferItem::~StreamBufferItem()
{
}

//----------------------------------------------------------------------------
StreamBufferItem::StreamBufferItem( const StreamBufferItem& dataItem )
{
  this->Matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->Status = TOOL_OK;
  *this = dataItem;
}

//----------------------------------------------------------------------------
StreamBufferItem& StreamBufferItem::operator=( StreamBufferItem const& dataItem )
{
  // Handle self-assignment
  if ( this == &dataItem )
  {
    return *this;
  }

  this->Frame = dataItem.Frame;
  this->PolyData = dataItem.PolyData;
  this->FilteredTimeStamp = dataItem.FilteredTimeStamp;
  this->UnfilteredTimeStamp = dataItem.UnfilteredTimeStamp;
  this->Index = dataItem.Index;
  this->Uid = dataItem.Uid;
  this->CustomFrameFields = dataItem.CustomFrameFields;
  this->Status = dataItem.Status;
  this->Matrix->DeepCopy( dataItem.Matrix );
  this->ValidTransformData = dataItem.ValidTransformData;

  return *this;
}

//----------------------------------------------------------------------------
void StreamBufferItem::SetCustomFrameField( std::string fieldName, std::string fieldValue )
{
  this->CustomFrameFields[fieldName] = fieldValue;
}

//----------------------------------------------------------------------------
PlusStatus StreamBufferItem::DeepCopy( StreamBufferItem* dataItem )
{
  if ( dataItem == NULL )
  {
    LOG_ERROR( "Failed to deep copy data buffer item - buffer item NULL!" );
    return PLUS_FAIL;
  }

  ( *this ) = ( *dataItem );

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus StreamBufferItem::SetMatrix( vtkMatrix4x4* matrix )
{
  if ( matrix == NULL )
  {
    LOG_ERROR( "Failed to set matrix - input matrix is NULL!" );
    return PLUS_FAIL;
  }

  ValidTransformData = true;

  this->Matrix->DeepCopy( matrix );

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus StreamBufferItem::GetMatrix( vtkMatrix4x4* outputMatrix )
{
  if ( outputMatrix == NULL )
  {
    LOG_ERROR( "Failed to copy matrix - output matrix is NULL!" );
    return PLUS_FAIL;
  }

  outputMatrix->DeepCopy( this->Matrix );

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
void StreamBufferItem::SetStatus( ToolStatus status )
{
  this->Status = status;
}

//----------------------------------------------------------------------------
ToolStatus StreamBufferItem::GetStatus() const
{
  return this->Status;
}

//----------------------------------------------------------------------------
void StreamBufferItem::SetPolyData(vtkSmartPointer<vtkPolyData> polyDataPtr)
{
  this->PolyData = polyDataPtr;
}

//----------------------------------------------------------------------------
bool StreamBufferItem::HasValidFieldData() const
{
  return this->CustomFrameFields.size() > 0;
}

bool StreamBufferItem::HasValidPolyData() const
{
  if (this->PolyData != NULL)
  {
    return true;
  }
  return false;
}