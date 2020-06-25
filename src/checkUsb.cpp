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

  string counterDataFilename = "data/counter.dat";
  string failMessage;
  string outMessage;
	int status;

  logMessage(outFilename, "[INFO] "+getTime()+" [Start] Get USB status");
  unsigned usbStatus = 0;
  status = Mt25FlashHighLevel::getUSBStatus(usbStatus, failMessage);
  logMessage(outFilename, "[INFO] "+getTime()+" USB status: "+std::to_string(usbStatus));
  if (!status) logMessage(outFilename, "[ERROR] "+getTime()+" checkUsb\n"+failMessage);
  logMessage(outFilename, "[INFO] "+getTime()+" [End  ] Get USB status");

  // status 1: success, 0: fail
  logMessage(outFilename, "[INFO] "+getTime()+" checkUsb !return: "+std::to_string(status));

  return !status;
}
