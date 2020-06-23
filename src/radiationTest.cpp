#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <ncurses.h>
#include <sqlite3.h>

#include "mt25FlashHighLevel.h"

using std::vector;
using std::cout;
using std::endl;
using std::string;

void validateData(std::vector<unsigned char> const & readData, std::vector<unsigned char> const & expectedData, std::vector<unsigned> & errorAddress) {
  for (unsigned address = 0; address < readData.size(); ++address) {
    if (readData[address] != expectedData[address]) errorAddress.push_back(address);
  }
}

void makeRandomData(int size, int seed, bool bitFlip, std::vector<unsigned char> & data){
  data.resize(size);
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> distribution(0,255);
  (uint8)distribution(generator); // Ignore first number
	for (unsigned iData = 0; iData < size; ++iData) {
    int number = (uint8)distribution(generator);
    if (bitFlip) {
      for (int iBit = 0; iBit < 8; ++iBit) number = (number ^ (1<<iBit));
    }
    data[iData] = number;
    //printf("%i %#04x\n", iData, data[iData]);
  }
}

void bitFlipData(std::vector<unsigned char> & data) {
  unsigned size = data.size();
	for (unsigned iData = 0; iData < size; ++iData) {
    int number = data[iData];
    for (int iBit = 0; iBit < 8; ++iBit) number = (number ^ (1<<iBit));
    data[iData] = number;
  }
}

int main()
{
  // Set timer
  std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();

  string failMessage;
  int status = 1;
  std::vector<unsigned char> writeData;
  std::vector<unsigned char> readData;
  std::vector<unsigned> errorAddress;
  std::vector<unsigned char> emptyData(16777216, 255);

  // Examples
  // Erase all
  cout<<"[Start] erase PROM"<<endl;
  status = Mt25FlashHighLevel::eraseProm(failMessage);
  if (!status) cout<<failMessage<<endl;
  cout<<"[End  ] erase PROM"<<endl;

  // Read all
  cout<<"[Start] read PROM"<<endl;
  status = Mt25FlashHighLevel::readProm(readData, failMessage);
  if (!status) cout<<failMessage<<endl;
  cout<<"[End  ] read PROM"<<endl;

  // Validate
  validateData(readData, emptyData, errorAddress);
  cout<<"Number of errors: "<<errorAddress.size()<<endl;

  //// Make random data
  //makeRandomData(16777216, 1, 0, writeData);
  // Quick data
  string sendData = Mt25FlashMsg::makeTestData(16777216);
  std::vector<unsigned char> v_sendData;
  for (int i=0; i<16777216;i++) {
    if (i>=sendData.size()) v_sendData.push_back(0);
    else v_sendData.push_back(sendData[i]);
  }
  writeData = v_sendData;
  //// bit flip Quick data
  //bitFlipData(writeData);

  // Write data
  cout<<"[Start] write PROM"<<endl;
  status = Mt25FlashHighLevel::writeProm(writeData, failMessage);
  if (!status) cout<<failMessage<<endl;
  cout<<"[End  ] write PROM"<<endl;

  // Read all
  cout<<"[Start] read PROM"<<endl;
  status = Mt25FlashHighLevel::readProm(readData, failMessage);
  if (!status) cout<<failMessage<<endl;
  cout<<"[End  ] read PROM"<<endl;

  // Validate
  validateData(readData, writeData, errorAddress);
  cout<<"Number of errors: "<<errorAddress.size()<<endl;

  // Re-read errors
  printf("%#04x %#04x\n", writeData[0], writeData[2]);
  errorAddress.push_back(0);
  errorAddress.push_back(2);

  double seconds = (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - beginTime)).count();
  std::cout<<"Program took "<<seconds<<" seconds"<<std::endl;
  return 0;
}


  // Print methods
  //for (int i=0; i<readData.size(); ++i) printf("read data[%#08x] %#02x\n", i, readData[i]); // by 8 bits
  //for (int i=0; i<(readData.size()/4); ++i) printf("read data[%#08x] %#08x\n", i*4, (readData[4*i+3]<<24)+(readData[4*i+2]<<16)+(readData[4*i+1]<<8)+readData[4*i]); // by 32 bits
  //for (int i=0; i<(readData.size()/8); ++i) { // by 64 bits
  //  printf("read data[%#08x] %#08x", i*8, (readData[8*i+7]<<24)+(readData[8*i+6]<<16)+(readData[8*i+5]<<8)+readData[8*i+4]);
  //  printf(" %08x\n", (readData[8*i+3]<<24)+(readData[8*i+2]<<16)+(readData[8*i+1]<<8)+readData[8*i]);
  //}
  //for (int i=0; i<(readData.size()/16); ++i) { // by 128 bits
  //  printf("read data[%#08x] %#08x", i*16, (readData[16*i+15]<<24)+(readData[16*i+14]<<16)+(readData[16*i+13]<<8)+readData[16*i+12]);
  //  printf(" %08x", (readData[16*i+11]<<24)+(readData[16*i+10]<<16)+(readData[16*i+9]<<8)+readData[16*i+8]);
  //  printf(" %08x", (readData[16*i+7]<<24)+(readData[16*i+6]<<16)+(readData[16*i+5]<<8)+readData[16*i+4]);
  //  printf(" %08x\n", (readData[16*i+3]<<24)+(readData[16*i+2]<<16)+(readData[16*i+1]<<8)+readData[16*i]);
  //}
  //for (int i=0; i<(readData.size()/4); ++i) printf("%08x ", (readData[4*i+3]<<24)+(readData[4*i+2]<<16)+(readData[4*i+1]<<8)+readData[4*i]); // by 32 bits

