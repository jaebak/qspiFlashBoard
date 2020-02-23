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

int main(int argc, char const *argv[]) {
  // Settings
  bool verbose = false;
  enum Protocol {spi, qspi};
  Protocol readProtocol = qspi;

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
      cout<<"Open a FT4222 device failed!"<<endl;
      return 0;
  }

  // Set ft4222 as spi master. Default system clock is 60 MHz.
  ftStatus = FT4222_SPIMaster_Init(ftHandle, SPI_IO_SINGLE, CLK_DIV_4, CLK_IDLE_LOW, CLK_LEADING, 0x01);
  if (FT_OK != ftStatus) {
      cout<<"Init FT4222 as SPI master device failed!"<<endl;;
      return 0;
  }

	// Read flash
  uint32 startAddress = 0x000000;
	unsigned readDataSize = 200;
  std::vector<unsigned char> readData;
	if (readProtocol == qspi) Mt25Flash::qspiFlashRead(ftHandle, startAddress, readDataSize, readData);
	else if (readProtocol == spi) Mt25Flash::spiFlashRead(ftHandle, startAddress, readDataSize, readData);

	// Print data
  for (int i=0; i<readData.size(); ++i) printf("%c", readData[i]);
	std::cout<<std::endl;
}
