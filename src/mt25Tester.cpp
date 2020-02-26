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
  bool verbose = false;
  enum Protocol {spi, qspi};
  Protocol writeProtocol = qspi;
  Protocol readProtocol = qspi;
  uint32 sendStartAddress = 0x000010;
  uint32 readStartAddress = 0x000010;
	// Max size: 16777216 (255 bytes per write, 65535 bytes per read)
  //uint32 sendDataSize = 16777216; //bytes
  //uint32 readDataSize = 16777216; //bytes
  uint32 sendDataSize = 65536*2; //bytes
  uint32 readDataSize = 65536*2; //bytes

	// Set data
	string sendData;
  sendData = "QSPI test board is ready for radiation testing!";
  sendData = Mt25Flash::makeTestData(sendDataSize);
  cout<<"To send data size: "<<sendDataSize<<endl;

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

  // Make data
  std::vector<unsigned char> v_sendData;
  for (int i=0; i<sendDataSize;i++) {
    if (i>=sendData.size()) v_sendData.push_back(0);
    else v_sendData.push_back(sendData[i]);
  }

  // Write to flash
  if (writeProtocol == qspi) Mt25Flash::qspiFlashWrite(ftHandle, sendStartAddress, v_sendData);
  else if (writeProtocol == spi) Mt25Flash::spiFlashWrite(ftHandle, sendStartAddress, v_sendData);

	// Read flash
  std::vector<unsigned char> readData;
	if (readProtocol == qspi) Mt25Flash::qspiFlashRead(ftHandle, readStartAddress, readDataSize, readData);
	else if (readProtocol == spi) Mt25Flash::spiFlashRead(ftHandle, readStartAddress, readDataSize, readData);

  //for (int i=0; i<readData.size(); ++i) printf("read data[%i] %#04x\n", i, readData[i]);

	// Compare write and read data
	if( 0 != memcmp(&v_sendData[0], &readData[0], readDataSize)) {
		std::cout<<"[Error] Write data and read data is different"<<std::endl;
		for (unsigned i = 0;  i < readDataSize; ++i ) {
			if (v_sendData[i] != readData[i]) {
        //std::cout<<"[Error] Data starts to become different from below"<<std::endl;
				if (i>0) printf("[Before] SendData[%i]: %#04x, ReadData[%i]: %#04x\n", i-1, v_sendData[i-1], i-1, readData[i-1]);
				printf("SendData[%i]: %#04x, ReadData[%i]: %#04x\n", i, v_sendData[i], i, readData[i]);
				//if (i<readDataSize-1) printf("[After] SendData[%i]: %#04x, ReadData[%i]: %#04x\n", i+1, v_sendData[i+1], i+1, readData[i+1]);
			}
		}
		return 0;
	} else {
		std::cout<<"[Success] Data is equal"<<std::endl;
	}

  double seconds = (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - beginTime)).count();
  std::cout<<"Program took "<<seconds<<" seconds"<<std::endl;
}
