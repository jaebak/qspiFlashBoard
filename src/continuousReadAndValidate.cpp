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
	string errorFilename = pid+".error";
	//string statusFilename = pid+".status";
  //std::ofstream statusFile;
	//statusFile.open(statusFilename);

  string counterDataFilename = "data/counter.dat";
  string failMessage;
	int status;
  vector<unsigned char> expectedData;
	Mt25FlashHighLevel::readFile(counterDataFilename, expectedData);
  vector<unsigned char> readData;
  vector<unsigned> errorAddress;

	while(1) {
		// Read data
    logMessage(outFilename, "[INFO] "+getTime()+" [Start] read PROM");
    status = Mt25FlashHighLevel::readProm(readData, failMessage);
    if (!status) logMessage(errorFilename, failMessage);
    logMessage(outFilename, "[INFO] "+getTime()+" [End  ] read PROM");

		// Validate data
    Mt25FlashHighLevel::validateData(readData, expectedData, errorAddress);
    logMessage(outFilename, "[INFO] "+getTime()+" Number of errors: "+std::to_string(errorAddress.size()));
	}

  return !status;
}
