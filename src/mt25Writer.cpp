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
  // Settings
  bool verbose = true;
  enum Protocol {spi, qspi};
  Protocol writeProtocol = qspi;
  // Max startAddress: 0xFFFFFF
  uint32 sendStartAddress = 0x00000;
	// Max dataSize: 16777216 (255 bytes per write, 65535 bytes per read)
  uint32 sendDataSize = 256; //bytes

	// Set data
	string sendData;
  //sendData = "QSPI test board is ready for radiation testing!";
  sendData = \
R"(Results are reported from a search for supersymmetric particles in proton-proton collisions
in the final state with a single, high transverse momentum lepton; multiple jets, including at least one b-tagged jet;
and large missing transverse momentum. The search uses a 136~fb$^{-1}$ sample of proton-proton collisions at $\sqrt s=13$ TeV
accumulated by the CMS experiment in 2016, 2017, and 2018.
The search focuses on processes leading to high jet multiplicities, such as gluino pair production with
${\rm \tilde g}\to{\rm t}{\rm \bar t}\,{\tilde\chi^0_1}$. The quantity $M_J$, defined as the sum of the masses
of the large-radius jets in the event, is used in conjunction with other kinematic variables to provide discrimination
between signal and background and as a key part of the background estimation method.
The observed event yields in the signal regions in data are consistent with
those expected for standard model backgrounds, estimated from control regions in
data.  Exclusion limits are obtained for a simplified model corresponding to
gluino pair production with three-body decays into top quarks and
neutralinos. Gluinos with a mass below about 2150~\GeV are excluded at a 95\%
confidence level for scenarios with $m({\tilde\chi^0_1})<700$~\GeV, and
the highest excluded neutralino mass is about 1250~\GeV.})";
  sendDataSize = sendData.size();
  //sendData = Mt25Flash::makeTestData(sendDataSize);
  //sendData = Mt25Flash::makeTestData(sendDataSize, 10);
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
  if (verbose) {
	  for (int i=0; i<v_sendData.size(); ++i) printf("send data[%#08x] %#04x\n", i+sendStartAddress, v_sendData[i]);
	}

  // Write to flash
  if (writeProtocol == qspi) Mt25Flash::qspiFlashWrite(ftHandle, sendStartAddress, v_sendData, verbose);
  else if (writeProtocol == spi) Mt25Flash::spiFlashWrite(ftHandle, sendStartAddress, v_sendData, verbose);

  std::cout<<"[Success] Wrote to flash"<<std::endl;
}
