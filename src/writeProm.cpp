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
	string statusFilename = pid+".status";

  string counterDataFilename = "data/counter.dat";
  string failMessage;
  string outMessage;
	int status;

  vector<unsigned char> expectedData;
	Mt25FlashHighLevel::readFile(counterDataFilename, expectedData);
  
  logMessage(outFilename, "[INFO] "+getTime()+" [Start] write PROM");
  status = Mt25FlashHighLevel::writePromMessage(expectedData, failMessage, outFilename);
  if (!status) logMessage(errorFilename, failMessage);
  logMessage(outFilename, "[INFO] "+getTime()+" [End  ] write PROM");

  // status 1: success, 0: fail
  logMessage(statusFilename, std::to_string(status));

  return !status;
}
