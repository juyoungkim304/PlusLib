/*=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/

#ifndef __vtkPlusAtracsysCommand_h
#define __vtkPlusAtracsysCommand_h

#include "vtkPlusServerExport.h"
#include "vtkPlusCommand.h"

class vtkPlusUsDevice;

/*!
  \class vtkPlusAtracsysCommand
  \brief This command allows OpenIGTLink commands to configure specific functionalities
  in the Atracsys device. See the documentation for command specifics.
  \ingroup PlusLibPlusServer
 */
class vtkPlusServerExport vtkPlusAtracsysCommand : public vtkPlusCommand
{
public:

  static vtkPlusAtracsysCommand* New();
  vtkTypeMacro(vtkPlusAtracsysCommand, vtkPlusCommand);
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  virtual vtkPlusCommand* Clone() { return New(); }

  /*! Executes the command  */
  virtual PlusStatus Execute();

  /*! Read command parameters from XML */
  virtual PlusStatus ReadConfiguration(vtkXMLDataElement* aConfig);

  /*! Write command parameters to XML */
  virtual PlusStatus WriteConfiguration(vtkXMLDataElement* aConfig);

  /*! Get all the command names that this class can execute */
  virtual void GetCommandNames(std::list<std::string>& cmdNames);

  /*! Gets the description for the specified command name. */
  virtual std::string GetDescription(const std::string& commandName);

  /*! Id of the ultrasound device to change the parameters of at the next Execute */
  vtkGetStdStringMacro(AtracsysDeviceId);
  vtkSetStdStringMacro(AtracsysDeviceId);

  void SetNameToSetUsParameter();

protected:
  vtkPlusDevice* GetAtracsysDevice();

  vtkPlusAtracsysCommand();
  virtual ~vtkPlusAtracsysCommand();

protected:
  std::string AtracsysDeviceId;

  /*!
     List of commands to execute.
  */
  std::map<std::string, std::string> CommandList;

  vtkPlusAtracsysCommand(const vtkPlusAtracsysCommand&);
  void operator=(const vtkPlusAtracsysCommand&);
};

#endif
