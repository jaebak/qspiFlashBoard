#include <iostream>
#include <vector>
#include <fstream>
#include "libft4222.h"

void writeFile(std::string filename, std::vector<unsigned char> const & data) {
  std::ofstream txtFile;
  txtFile.open(filename);
  for (auto item : data) {
    txtFile<<item;
  }
  std::cout<<"filename: "<<filename<<" was made."<<std::endl;
  txtFile.close();
}

void readFile(std::string filename, std::vector<unsigned char> & data) {
  data.clear();
  std::ifstream txtFile;
  txtFile.open(filename);
  char t_char;
  while (txtFile.get(t_char)) {
    data.push_back(t_char);
    //printf("%#04x\n",t_char);
  }
  txtFile.close();
}

void validateData(std::vector<unsigned char> const & readData, std::vector<unsigned char> const & expectedData, std::vector<unsigned> & errorAddress) {
  errorAddress.clear();
  for (unsigned address = 0; address < readData.size(); ++address) {
    //printf("%#04x %#04x\n", readData[address], expectedData[address]);
    if (readData[address] != expectedData[address]) {
      errorAddress.push_back(address);
    }
  }
}

int main() {
  const unsigned dataLength = 16777216;
  std::vector<unsigned char> readData;

  std::string emptyDataFilename = "data/empty.dat";
  std::vector<unsigned char> emptyData(dataLength, 255);
  writeFile(emptyDataFilename, emptyData);

  std::cout<<"Reading "<<emptyDataFilename<<std::endl;
  readFile(emptyDataFilename, readData);
  std::vector<unsigned> errorAddress;
  validateData(emptyData, readData, errorAddress);
  std::cout<<"Validating "<<emptyDataFilename<<std::endl;
  std::cout<<"Errors: "<<errorAddress.size()<<std::endl;

  std::string counterDataFilename = "data/counter.dat";
  std::vector<unsigned char> counterData;
  for (int i=0; i<dataLength;i++) {
    counterData.push_back((uint8) i);
  }
  writeFile(counterDataFilename, counterData);

  std::cout<<"Reading "<<counterDataFilename<<std::endl;
  readFile(counterDataFilename, readData);
  validateData(counterData, readData, errorAddress);
  std::cout<<"Validating "<<counterDataFilename<<std::endl;
  std::cout<<"Errors: "<<errorAddress.size()<<std::endl;

  return 0;
}
