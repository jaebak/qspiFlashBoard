#ifndef FT4222CUSTOMLIBRARY
#define FT4222CUSTOMLIBRARY

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cstring>

#include "ftd2xx.h"
#include "libft4222.h"

namespace Ft4222CustomLibrary {

  void getFtUsbDevices(std::vector< FT_DEVICE_LIST_INFO_NODE > & ft4222DeviceList) {
    FT_STATUS ftStatus = 0;
  
    DWORD numOfDevices = 0;
    ftStatus = FT_CreateDeviceInfoList(&numOfDevices);
  
    for(DWORD iDev=0; iDev<numOfDevices; ++iDev) {
      FT_DEVICE_LIST_INFO_NODE devInfo;
      memset(&devInfo, 0, sizeof(devInfo));
  
      ftStatus = FT_GetDeviceInfoDetail(iDev, &devInfo.Flags, &devInfo.Type, &devInfo.ID, &devInfo.LocId,
          devInfo.SerialNumber,
          devInfo.Description,
          &devInfo.ftHandle);
  
      if (FT_OK == ftStatus) {
        const std::string desc = devInfo.Description;
        if(desc == "FT4222" || desc == "FT4222 A") {
          ft4222DeviceList.push_back(devInfo);
        }
      }
    }
  }
  
	std::string deviceFlagToString(DWORD flags) {
		  std::string message;
      message += (flags & 0x1)? "DEVICE_OPEN" : "DEVICE_CLOSED";
      message += ", ";
      message += (flags & 0x2)? "High-speed USB" : "Full-speed USB";
      return message;
  }
  
  void printFtUsbDevices(std::vector< FT_DEVICE_LIST_INFO_NODE > const & ft4222DeviceList) {
    if (ft4222DeviceList.empty()) std::cout<<"No FT4222 device was found"<<std::endl;
    for (unsigned iDevice = 0; iDevice < ft4222DeviceList.size(); ++iDevice) {
      FT_DEVICE_LIST_INFO_NODE const & device = ft4222DeviceList[iDevice];
      printf("Device %d:\n", iDevice);
      printf("  Flags= 0x%x, (%s)\n", device.Flags, deviceFlagToString(device.Flags).c_str());
      printf("  Type= 0x%x\n",        device.Type);
      printf("  ID= 0x%x\n",          device.ID);
      printf("  LocId= 0x%x\n",       device.LocId);
      printf("  SerialNumber= %s\n",  device.SerialNumber);
      printf("  Description= %s\n",   device.Description);
      printf("  ftHandle= %p\n",    device.ftHandle);
    }
	}
}

#endif
