#include <iostream>
#include <vector>
#include <string>

#include "ftd2xx.h"
#include "libft4222.h"

#include "ft4222CustomLibrary.h"
#include "mt25Flash.h"

using std::vector;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char const *argv[]) {
  // Set timer
  std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();

  // Settings
  bool verbose = true;
  enum Protocol {spi, qspi};
  Protocol writeProtocol = qspi;
  // Max startAddress: 0xFFFFFF
  uint32 startAddress = 0x00000;
  // Max dataSize: 16777216 (255 bytes per write, 65535 bytes per read)
  uint32 dataSize = 16777216; //bytes

  // Get ft4222 device
  vector< FT_DEVICE_LIST_INFO_NODE > ft4222DeviceList;
  Ft4222CustomLibrary::getFtUsbDevices(ft4222DeviceList);
  if (verbose) Ft4222CustomLibrary::printFtUsbDevices(ft4222DeviceList);
  if (ft4222DeviceList.empty()) {
    cout<<"No FT4222 device was found"<<endl;
    return 0;
  }

  // Open ft4222 device.
  FT_HANDLE ftHandle = NULL;
  FT_STATUS ftStatus;
  ftStatus = FT_OpenEx((PVOID)ft4222DeviceList[0].Description, FT_OPEN_BY_DESCRIPTION, &ftHandle);
  if (FT_OK != ftStatus) {
      cout<<"FT_OpenEx failed"<<endl;
      return 0;
  }

  // Set ft4222 system clock. (24, 48, 60 (default), 80 MHz)
  ftStatus = FT4222_SetClock(ftHandle, SYS_CLK_60);
  if (FT4222_OK != ftStatus) {
		cout<<"FT4222_SetClock failed"<<endl;
    return 0; 
  }

  // Set ft4222 as spi master. (CLK_DIV_2, CLK_DIV_4, ..., CLK_DIV_512)
  ftStatus = FT4222_SPIMaster_Init(ftHandle, SPI_IO_SINGLE, CLK_DIV_4, CLK_IDLE_LOW, CLK_LEADING, 0x01);
  if (FT_OK != ftStatus) {
      cout<<"FT4222_SPIMaster_Init failed"<<endl;;
      return 0;
  }

  // Erase flash
  if(!Mt25Flash::eraseFlash(ftHandle, startAddress, startAddress + dataSize)) {
    std::cout<<"spiFlashWrite failed"<<std::endl;
    return 0;
  } else {
    std::cout<<"Flash was erased"<<std::endl;
  }

  double seconds = (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - beginTime)).count();
  std::cout<<"Program took "<<seconds<<" seconds"<<std::endl;
}
