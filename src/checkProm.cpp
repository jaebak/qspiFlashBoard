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

  logMessage(outFilename, "[INFO] "+getTime()+" [Start] Get Prom status");
  vector<unsigned> promStatusRegisters;
  status = Mt25FlashHighLevel::getPromStatus(promStatusRegisters, failMessage);
  for(unsigned iStatus = 0; iStatus<promStatusRegisters.size(); ++iStatus) {
    logMessage(outFilename, "[INFO] "+getTime()+" Prom status: "+bitset<8>(promStatusRegisters[iStatus]).to_string());
	}
  if (!status) logMessage(outFilename, "[ERROR] "+getTime()+" checkProm\n"+failMessage);
  logMessage(outFilename, "[INFO] "+getTime()+" [End  ] Get Prom status");

  // status 1: success, 0: fail
  logMessage(outFilename, "[INFO] "+getTime()+" checkProm !return: "+std::to_string(status));

  return !status;
}
