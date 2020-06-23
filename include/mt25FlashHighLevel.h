#ifndef MT25FlashHighLevel
#define MT25FlashHighLevel

#include <string>
#include "mt25FlashMsg.h"
#include "ft4222CustomLibrary.h"

using std::vector;
using std::cout;
using std::endl;
using std::string;

namespace Mt25FlashHighLevel {
  int setupFt4222(FT_HANDLE & ftHandle, std::string & failMessage) {
    // Get ft4222 device
    vector< FT_DEVICE_LIST_INFO_NODE > ft4222DeviceList;
    Ft4222CustomLibrary::getFtUsbDevices(ft4222DeviceList);
    if (ft4222DeviceList.empty()) {
      failMessage = "No FT4222 device was found";
      return 0;
    }

    // Open ft4222 device.
    FT_STATUS ftStatus;
    ftStatus = FT_OpenEx((PVOID)ft4222DeviceList[0].Description, FT_OPEN_BY_DESCRIPTION, &ftHandle);
    if (FT4222_OK != ftStatus) {
        failMessage = "FT_OpenEx failed";
        return 0;
    }

    // Set ft4222 system clock. (24, 48, 60 (default), 80 MHz)
    ftStatus = FT4222_SetClock(ftHandle, SYS_CLK_60);
    if (FT4222_OK != ftStatus) {
	  	failMessage = "FT4222_SetClock failed";
      return 0; 
    }

    // Set ft4222 as spi master. (CLK_DIV_2, CLK_DIV_4, ..., CLK_DIV_512)
    ftStatus = FT4222_SPIMaster_Init(ftHandle, SPI_IO_SINGLE, CLK_DIV_4, CLK_IDLE_LOW, CLK_LEADING, 0x01);
    if (FT4222_OK != ftStatus) {
        failMessage = "FT4222_SPIMaster_Init failed";
        return 0;
    }

    return 1;
  }

  // Return: 1: success, 0: Failed
  int eraseProm(std::string & failMessage) {
    int status = 1;
    FT_HANDLE ftHandle = NULL;

    status = setupFt4222(ftHandle, failMessage);
    if (!status) {
      failMessage = "[eraseProm] failed 1\n"+failMessage;
    } else {
      status = Mt25FlashMsg::eraseFlash(ftHandle, 0, 16777216, failMessage);
      if (!status) {
        failMessage = "[eraseProm] failed 2\n"+failMessage;
      }
    }

    FT_STATUS ftStatus;
    ftStatus = FT_Close(ftHandle);
    if (FT4222_OK != ftStatus) {
      if (failMessage.size() == 0) failMessage = "[eraseProm] FT_Close failed";
      else failMessage += "\n[eraseProm] FT_Close failed";
      return 0;
    }

    return status;
  }

  // Return: 1: success, 0: Failed
  int readProm(std::vector<unsigned char> & readData, std::string & failMessage) {
    int status = 1;
    FT_HANDLE ftHandle = NULL;

    status = setupFt4222(ftHandle, failMessage);
    if (!status) {
      failMessage = "[readProm] failed 1\n"+failMessage;
    } else {
      status = Mt25FlashMsg::qspiFlashRead(ftHandle, 0, 16777216, readData, failMessage);
      if (!status) {
        failMessage = "[readProm] failed 2\n"+failMessage;
      }
    }

    FT_STATUS ftStatus;
    ftStatus = FT_Close(ftHandle);
    if (FT4222_OK != ftStatus) {
      if (failMessage.size() == 0) failMessage = "[readProm] FT_Close failed";
      else failMessage += "\n[readProm] FT_Close failed";
      return 0;
    }

    return status;
  }

  // Return: 1: success, 0: Failed
  int rereadProm(std::vector<unsigned> & addresses, std::vector<unsigned char> & readData, std::string & failMessage) {
    int status = 1;
    FT_HANDLE ftHandle = NULL;
    readData.clear();

    status = setupFt4222(ftHandle, failMessage);
    if (!status) {
      failMessage = "[readProm] failed 1\n"+failMessage;
    } else {
      for (unsigned iAddress = 0; iAddress < addresses.size(); ++iAddress) {
        vector<unsigned char> t_readData;
        status = Mt25FlashMsg::qspiFlashRead(ftHandle, addresses[iAddress], 1, t_readData, failMessage);
        if (!status) {
          failMessage = "[readProm] failed 2\n"+failMessage;
          break;
        } else {
          readData.push_back(t_readData[0]);
        }
      }
    }

    FT_STATUS ftStatus;
    ftStatus = FT_Close(ftHandle);
    if (FT4222_OK != ftStatus) {
      if (failMessage.size() == 0) failMessage = "[readProm] FT_Close failed";
      else failMessage += "\n[readProm] FT_Close failed";
      return 0;
    }

    return status;
  }

  // Return: 1: success, 0: Failed
  int writeProm(std::vector<unsigned char> const & writeData, std::string & failMessage) {
    int status = 1;
    FT_HANDLE ftHandle = NULL;

    status = setupFt4222(ftHandle, failMessage);
    if (!status) {
      failMessage = "[writeProm] failed 1\n"+failMessage;
    } else {
      status = Mt25FlashMsg::qspiFlashWrite(ftHandle, 0, writeData, failMessage);
      if (!status) {
        failMessage = "[writeProm] failed 2\n"+failMessage;
      }
    }

    FT_STATUS ftStatus;
    ftStatus = FT_Close(ftHandle);
    if (FT4222_OK != ftStatus) {
      if (failMessage.size() == 0) failMessage = "[writeProm] FT_Close failed";
      else failMessage += "\n[writeProm] FT_Close failed";
      return 0;
    }

    return status;
  }

  // Return: 1: success, 0: Failed
  int getUSBStatus(unsigned & usbStatus, std::string & failMessage) {
    usbStatus = 0;

    // Get ft4222 device
    vector< FT_DEVICE_LIST_INFO_NODE > ft4222DeviceList;
    Ft4222CustomLibrary::getFtUsbDevices(ft4222DeviceList);
    if (ft4222DeviceList.empty()) {
      failMessage = "No FT4222 device was found";
      return 0;
    }
    // Get status of device
    FT_DEVICE_LIST_INFO_NODE const & device = ft4222DeviceList[0];
    usbStatus = device.Flags;
    return 1;
  }

  // Return: 1: success, 0: Failed
  // promStatusRegisters[0]: status register
  // promStatusRegisters[1]: flag status register
  // promStatusRegisters[2]: nonvolatile configuration register (0)
  // promStatusRegisters[3]: nonvolatile configuration register (1)
  // promStatusRegisters[4]: volatile configuration register
  // promStatusRegisters[5]: enhanced volatile configuration register
  // promStatusRegisters[6]: general purpose register
  int getPromStatus(vector<unsigned> & promStatusRegisters, std::string & failMessage) {
    int status = 1;
    FT_HANDLE ftHandle = NULL;
    promStatusRegisters.clear();
    promStatusRegisters.resize(7);

    status = setupFt4222(ftHandle, failMessage);
    if (!status) {
      failMessage = "[checkPromStatus] failed 1\n"+failMessage;
    } else {
      std::vector<unsigned char> v_promStatus;
      // promStatus should be all 0.
      status = Mt25FlashMsg::runFlashCommand(ftHandle, 0x05, 1, v_promStatus, failMessage);
      promStatusRegisters[0] = v_promStatus[0];
      if (!status) {
        failMessage = "[checkPromStatus] failed 2\n"+failMessage;
      } else {
        status = Mt25FlashMsg::runFlashCommand(ftHandle, 0x70, 1, v_promStatus, failMessage);
        promStatusRegisters[1] = v_promStatus[0];
        if (!status) {
          failMessage = "[checkPromStatus] failed 3\n"+failMessage;
        } else {
          status = Mt25FlashMsg::runFlashCommand(ftHandle, 0xb5, 2, v_promStatus, failMessage);
          promStatusRegisters[2] = v_promStatus[0];
          promStatusRegisters[3] = v_promStatus[1];
          if (!status) {
            failMessage = "[checkPromStatus] failed 4\n"+failMessage;
          } else {
            status = Mt25FlashMsg::runFlashCommand(ftHandle, 0x85, 1, v_promStatus, failMessage);
            promStatusRegisters[4] = v_promStatus[0];
            if (!status) {
              failMessage = "[checkPromStatus] failed 5\n"+failMessage;
            } else {
              status = Mt25FlashMsg::runFlashCommand(ftHandle, 0x65, 1, v_promStatus, failMessage);
              promStatusRegisters[5] = v_promStatus[0];
              if (!status) {
                failMessage = "[checkPromStatus] failed 6\n"+failMessage;
              } else {
                status = Mt25FlashMsg::runFlashCommand(ftHandle, 0x96, 1, v_promStatus, failMessage);
                promStatusRegisters[6] = v_promStatus[0];
                if (!status) {
                  failMessage = "[checkPromStatus] failed 7\n"+failMessage;
                }
              }
            }
          }
        }
      }
    }

    FT_STATUS ftStatus;
    ftStatus = FT_Close(ftHandle);
    if (FT4222_OK != ftStatus) {
      if (failMessage.size() == 0) failMessage = "[eraseProm] FT_Close failed";
      else failMessage += "\n[eraseProm] FT_Close failed";
      return 0;
    }

    return status;
  }

}
#endif
