#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <fstream>
#include <ctime>

#include "mt25FlashHighLevel.h"

using std::vector;
using std::cout;
using std::endl;
using std::string;
using std::bitset;
using Mt25FlashHighLevel::logMessage;
using Mt25FlashHighLevel::getTime;

int main(int argc, char * argv[]) {
  string pid;
  if (argc!=2) pid = "current";
  else pid = argv[1];

	string outFilename = pid+".out";
	//string errorFilename = pid+".error";
	//string statusFilename = pid+".status";
  //std::ofstream statusFile;
	//statusFile.open(statusFilename);

  string counterDataFilename = "data/counter.dat";
  string failMessage;
	int status;
  vector<unsigned char> expectedData;
	Mt25FlashHighLevel::readFile(counterDataFilename, expectedData);
  vector<unsigned char> readData;
  vector<unsigned> errorAddresses;

	while(1) {
		// Read data
    logMessage(outFilename, "[INFO] "+getTime()+" [Start] read PROM");
    status = Mt25FlashHighLevel::readProm(readData, failMessage);
    if (!status) logMessage(outFilename, "[ERROR] "+getTime()+" continuousReadAndValidate\n"+failMessage);
    logMessage(outFilename, "[INFO] "+getTime()+" [End  ] read PROM");

		// Validate data
    Mt25FlashHighLevel::validateData(readData, expectedData, errorAddresses);
    logMessage(outFilename, "[INFO] "+getTime()+" Number of errors: "+std::to_string(errorAddresses.size()));
    unsigned maxError = 256;
    for (unsigned iAddress = 0; iAddress < errorAddresses.size(); ++iAddress) {
      //logMessage(outFilename, "[INFO] "+getTime()+" Error address: "+std::to_string(errorAddresses[iAddress]));
      unsigned errorAddress = errorAddresses[iAddress];
      char expected[10]; 
      sprintf(expected, "%#04x", expectedData[errorAddress]);
      char read[10];
      sprintf(read, "%#04x", readData[errorAddress]);
      logMessage(outFilename, "[INFO] "+getTime()+" Error address: "+std::to_string(errorAddress)+" expected: "+expected+" read: "+read);
      if (iAddress>=maxError) {
        logMessage(outFilename, "[INFO] "+getTime()+" More than "+std::to_string(maxError)+" error address");
        break;
      }
    }
	}

  return !status;
}
