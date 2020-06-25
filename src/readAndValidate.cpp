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
  string counterDataFilename;
  string pid;
  if (argc==1) {
    pid == "current";
    counterDataFilename = "data/counter.dat";
  } else if (argc==2) {
    pid = argv[1];
    counterDataFilename = "data/counter.dat";
  } else {
    pid = argv[1];
    counterDataFilename = argv[2];
  }

	string outFilename = pid+".out";
	//string errorFilename = pid+".error";
	//string statusFilename = pid+".status";

  string failMessage;
  string outMessage;
	int status;

  vector<unsigned char> expectedData;
	Mt25FlashHighLevel::readFile(counterDataFilename, expectedData);
  vector<unsigned char> readData;
  vector<unsigned> errorAddress;
  
	// Read data
  logMessage(outFilename, "[INFO] "+getTime()+" [Start] read PROM");
  status = Mt25FlashHighLevel::readProm(readData, failMessage);
  if (!status) logMessage(outFilename, "[ERROR] "+getTime()+" readAndValidate\n"+failMessage);
  logMessage(outFilename, "[INFO] "+getTime()+" [End  ] read PROM");

	// Validate data
  logMessage(outFilename, "[INFO] "+getTime()+" Validate with "+counterDataFilename);
  Mt25FlashHighLevel::validateData(readData, expectedData, errorAddress);
  logMessage(outFilename, "[INFO] "+getTime()+" Number of errors: "+std::to_string(errorAddress.size()));
  unsigned maxError = 256;
  for (unsigned iAddress = 0; iAddress < errorAddress.size(); ++iAddress) {
    logMessage(outFilename, "[INFO] "+getTime()+" Error address: "+std::to_string(errorAddress[iAddress]));
    if (iAddress>=maxError) {
      logMessage(outFilename, "[INFO] "+getTime()+" More than "+std::to_string(maxError)+" error address");
      break;
    }
  }

  // status 1: success, 0: fail
  logMessage(outFilename, "[INFO] "+getTime()+" readAndValidate !return: "+std::to_string(status));

  return !status;
}
