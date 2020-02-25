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
  // Set timer
  std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();

  // Settings
  bool verbose = false;
  enum Protocol {spi, qspi};
  Protocol readProtocol = qspi;
  uint32 readStartAddress = 0x000000;
	// Max size: 16777216 (65535 bytes per read)
  //uint32 readDataSize = 8388608; //bytes
  uint32 readDataSize = 16777216; //bytes
  //uint32 readDataSize = 65536*8; //bytes

  // Get ft4222 devie
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

	// Read flash
  std::vector<unsigned char> readData;
	if (readProtocol == qspi) Mt25Flash::qspiFlashRead(ftHandle, readStartAddress, readDataSize, readData);
	else if (readProtocol == spi) Mt25Flash::spiFlashRead(ftHandle, readStartAddress, readDataSize, readData);

	// Print data
  //for (int i=0; i<readData.size(); ++i) printf("%c", readData[i]);
	//std::cout<<std::endl;
  //for (int i=0; i<readData.size(); ++i) printf("read data[%i] %#04x\n", i, readData[i]);

	// For debugging purpose
	std::string sendData;
  sendData = Mt25Flash::makeTestData(readDataSize);
  std::vector<unsigned char> v_sendData;
  for (int i=0; i<readDataSize;i++) {
    if (i>=sendData.size()) v_sendData.push_back(0);
    else v_sendData.push_back((uint8)sendData[i]);
  }
  bool dataSame = true;
	for (unsigned i = 0;  i < readDataSize; ++i ) {
		if (v_sendData[i] != readData[i]) {
      dataSame = false;
      std::cout<<"[Error] Data starts to become different from below"<<std::endl;
			printf("SendData[%i]: %#04x, ReadData[%i]: %#04x\n", i, v_sendData[i], i, readData[i]);
			break;
		}
	}
  if (dataSame) std::cout<<"[Success] "<<readDataSize<<" bytes of data are as expected"<<endl;

  double seconds = (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - beginTime)).count();
  std::cout<<"Program took "<<seconds<<" seconds"<<std::endl;
}
