#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <ncurses.h>
#include <sqlite3.h>
#include <bitset>
#include <fstream>
#include <ctime>

#include "mt25FlashHighLevel.h"

using std::vector;
using std::cout;
using std::endl;
using std::string;
using std::bitset;

void validateData(std::vector<unsigned char> const & readData, std::vector<unsigned char> const & expectedData, std::vector<unsigned> & errorAddress) {
  errorAddress.clear();
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

void extractData(std::vector<unsigned> const & addresses, std::vector<unsigned char> const & inData, std::vector<unsigned char> & outData) {
  outData.clear();
  for (unsigned iAddress = 0; iAddress < addresses.size(); ++iAddress) {
    outData.push_back(inData[addresses[iAddress]]);
  }
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("----\n");
   return 0;
}

int exec_sql(sqlite3 * database, std::string sql) {
  char * errorMessage = 0;
  int errorStatus = 0;
  errorStatus = sqlite3_exec(database, sql.c_str(), callback, 0, &errorMessage);
  if (errorStatus != SQLITE_OK) {
    cout<<"SQL error: "<<errorMessage<<endl;
    sqlite3_free(errorMessage);
  } else {
    cout<<"SQL was executed."<<endl;
  }
  return errorStatus;
}

//Ref: https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
void logInformation(std::string log, std::string databaseFilename, std::string txtFilename="") {
  // Current time
  //auto time_point = std::chrono::system_clock::now();
  //std::time_t now_c = std::chrono::system_clock::to_time_t(time_point);
  std::time_t now = std::time(NULL);
  char timeBuffer[32];
  std::strftime(timeBuffer, 32, "%Y-%m-%d %H:%M:%S", std::localtime(&now));

  // Log in database
  string tableName = "RADIATION_TEST";
  int status;
  string sql;
  sqlite3 * database;
  // TODO: Open until successfull
  sqlite3_open(databaseFilename.c_str(), &database);
  sql = "CREATE TABLE IF NOT EXISTS "+tableName+"("\
         "ID INTEGER PRIMARY KEY,"\
         "LOGTIME DATETIME NOT NULL,"\
         "LOG TEXT NOT NULL);";
  exec_sql(database, sql);
  sql = "INSERT INTO "+tableName+" (LOGTIME, LOG) "\
         "VALUES ('"+timeBuffer+"', '"+log+"');";
  exec_sql(database, sql);
  //sql = "DELETE from TEST where ID=2;";
  //sql = "SELECT * from TEST";
  //sql = "DROP TABLE "+tableName;
  //sql = "SELECT * from RADIATION_TEST WHERE LOGTIME BETWEEN '2020-06-24 02:17:00' and '2020-06-24 03:00:00'";

  sqlite3_close(database);

  // Log in text file
  if (txtFilename != "") {
    std::ofstream txtFile;
    txtFile.open(txtFilename, std::ios_base::app);
    txtFile<<timeBuffer<<": "<<log<<std::endl;
  }
}

void print_menu(WINDOW *menu_win, int highlight, vector<string> const & selections)
{
	int x, y, i;

	x = 2; // text initial location
	y = 2; // text initial location
	box(menu_win, 0, 0); // numbers are space between things
	for(i = 0; i < selections.size(); ++i)
	{	
    if(highlight == i + 1) /* High light the present choice */
		{	
      wattron(menu_win, A_REVERSE);
			mvwprintw(menu_win, y, x, "%s", selections[i].c_str());
			wattroff(menu_win, A_REVERSE);
		}
		else mvwprintw(menu_win, y, x, "%s", selections[i].c_str());
		++y;
	}
	wrefresh(menu_win);
}

int main()
{
  // Set timer
  std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();

  //initscr();      /* Start curses mode      */
  //printw("Hello World !!!\n");  /* Print Hello World      */
  //mvprintw(0, 0, "Bye World");
  //printw("\nType any string: ");
  //refresh();      /* Print it on to the real screen */
  //char inputChar[80];
  //getstr(inputChar);
  //printw("Pressed keys are: %s ", inputChar);
  //refresh();      /* Print it on to the real screen */
  //getch();      /* Wait for user input */
  //endwin();     /* End curses mode      */

  vector<string> selections = {"Erase PROM", "Write PROM", "Read PROM", "Validate PROM", "Re-read PROM", "Exit"};

  WINDOW *menu_win;
	int highlight = 1;
	int choice = 0;
	int pressedKey;
  int const WIDTH = 30;
  int const HEIGHT = selections.size()+4;

	initscr();
	clear();
	noecho();
	cbreak();	/* Line buffering disabled. pass on everything */
	int startx = 0;
	int starty = 1;

	menu_win = newwin(HEIGHT, WIDTH, starty, startx);
	keypad(menu_win, TRUE);
	mvprintw(0, 0, "Use arrow keys to go up and down, Press enter to select a choice");
	refresh();
	print_menu(menu_win, 1, selections);
  // variables for PROM commands
  int status = 1;
  string failMessage;
  vector<unsigned char> expectedData;
  vector<unsigned char> readData;
  vector<unsigned> errorAddress;
	while(1)
	{	

    // Selections
    pressedKey = wgetch(menu_win);
		switch(pressedKey)
		{	
      case KEY_UP:
				if(highlight == 1)
					highlight = selections.size();
				else
					--highlight;
				break;
			case KEY_DOWN:
				if(highlight == selections.size())
					highlight = 1;
				else
					++highlight;
				break;
			case 10:
				choice = highlight;
				break;
			default:
				//mvprintw(24, 0, "Charcter pressed is = %3d Hopefully it can be printed as '%c'", pressedKey, pressedKey);
	      mvprintw(HEIGHT+1, 0, "Only up/down/enter is allowed.");
				refresh();
				break;
		}
		print_menu(menu_win, highlight, selections);

    // Display updates for chosen command
    if (choice != 0) {
	    mvprintw(HEIGHT+1, 0, "%s was chosen. Waiting for action to finish.\n", selections[choice - 1].c_str());
      move(HEIGHT+3,0);
      clrtobot();
      refresh();
    }

    // Actions
    if (choice != 0) {
		  if(choice == selections.size())	break; // Exit was chosen
      else if (selections[choice-1] == "Erase PROM") {
        status = Mt25FlashHighLevel::eraseProm(failMessage);
        expectedData = std::vector<unsigned char> (16777216, 255);
      } else if (selections[choice-1] == "Write PROM") {
        // Make data
        expectedData.clear();
        for (int i=0; i<16777216;i++) {
          expectedData.push_back((uint8) i);
        }
        // Write to PROM
        status = Mt25FlashHighLevel::writeProm(expectedData, failMessage);
      } else if (selections[choice-1] == "Read PROM") {
        status = Mt25FlashHighLevel::readProm(readData, failMessage);
      } else if (selections[choice-1] == "Validate PROM") {
        validateData(readData, expectedData, errorAddress);
      } else if (selections[choice-1] == "Re-read PROM") {
        status = Mt25FlashHighLevel::rereadProm(errorAddress, readData, failMessage);
        vector<unsigned char> tempData = expectedData;
        extractData(errorAddress, tempData, expectedData);
      }
    }

    // Display updates after action is done
    if (choice != 0) {
      if (selections[choice-1] == "Erase PROM") {
        if (!status) {
          mvprintw(HEIGHT+1, 0, "PROM erase failed.\n");
          mvprintw(HEIGHT+3, 0, failMessage.c_str());
        }
        else {
          mvprintw(HEIGHT+1, 0, "PROM was erased\n");
          move(HEIGHT+3,0);
          clrtobot();
        }
      } else if (selections[choice-1] == "Write PROM") {
        if (!status) {
          mvprintw(HEIGHT+1, 0, "PROM write failed.\n");
          mvprintw(HEIGHT+3, 0, failMessage.c_str());
        }
        else {
          mvprintw(HEIGHT+1, 0, "Wrote to PROM\n");
          move(HEIGHT+3,0);
          clrtobot();
        }
      } else if (selections[choice-1] == "Read PROM") {
        if (!status) {
          mvprintw(HEIGHT+1, 0, "PROM read failed.\n");
          mvprintw(HEIGHT+3, 0, failMessage.c_str());
        }
        else {
          mvprintw(HEIGHT+1, 0, "Read PROM\n");
          move(HEIGHT+3,0);
          clrtobot();
        }
      } else if (selections[choice-1] == "Validate PROM") {
          mvprintw(HEIGHT+1, 0, ("Number of errors: "+std::to_string(errorAddress.size())+"\n").c_str());
      } else if (selections[choice-1] == "Re-read PROM") {
        if (!status) {
          mvprintw(HEIGHT+1, 0, "PROM re-read failed.\n");
          mvprintw(HEIGHT+3, 0, failMessage.c_str());
        }
        else {
          mvprintw(HEIGHT+1, 0, "Re-read PROM\n");
          move(HEIGHT+3,0);
          clrtobot();
        }
      }
      refresh();
      choice = 0;
    }

	}
	clrtoeol();
	refresh();
	endwin();
	return 0;

  // Finish examples
  // Make flow for radiation test

  //// Examples
  //string failMessage;
  //int status = 1;
  //std::vector<unsigned char> writeData;
  //vector<unsigned char> expectedData;
  //std::vector<unsigned char> readData;
  //std::vector<unsigned> errorAddress;
  //std::vector<unsigned char> emptyData(16777216, 255);
  
  // showMessage on ncurses
  // control with ncurses

  //logInformation("Another test", "test.sqlite3", "test.txt");

  //cout<<"[Start] check USB status"<<endl;
  //unsigned usbStatus = 0;
  //status = Mt25FlashHighLevel::getUSBStatus(usbStatus, failMessage);
  //if (!status) cout<<failMessage<<endl;
  //cout<<usbStatus<<endl;
  //cout<<"[End  ] check USB status"<<endl;

  //// checkPromStatus
  //cout<<"[Start] check PROM status"<<endl;
  //vector<unsigned> promStatusRegisters;
  //status = Mt25FlashHighLevel::getPromStatus(promStatusRegisters, failMessage);
  //if (!status) cout<<failMessage<<endl;
  //for(unsigned iStatus = 0; iStatus<promStatusRegisters.size(); ++iStatus) cout<<bitset<8>(promStatusRegisters[iStatus])<<endl;
  //cout<<"[End  ] check PROM status"<<endl;

  //// Erase all
  //cout<<"[Start] erase PROM"<<endl;
  //status = Mt25FlashHighLevel::eraseProm(failMessage);
  //if (!status) cout<<failMessage<<endl;
  //cout<<"[End  ] erase PROM"<<endl;

  //// Read all
  //cout<<"[Start] read PROM"<<endl;
  //status = Mt25FlashHighLevel::readProm(readData, failMessage);
  //if (!status) cout<<failMessage<<endl;
  //cout<<"[End  ] read PROM"<<endl;

  //// Validate
  //expectedData = emptyData;
  //validateData(readData, expectedData, errorAddress);
  //cout<<"Number of errors: "<<errorAddress.size()<<endl;

  ////// Make data
  ////makeRandomData(16777216, 1, 0, writeData);
  ////// Quick data
  ////string sendData = Mt25FlashMsg::makeTestData(16777216);
  ////// convert data to vector
  ////std::vector<unsigned char> v_sendData;
  ////for (int i=0; i<16777216;i++) {
  ////  if (i>=sendData.size()) v_sendData.push_back(0);
  ////  else v_sendData.push_back(sendData[i]);
  ////}
  ////// Test data
  //std::vector<unsigned char> v_sendData;
  //for (int i=0; i<16777216/2;i++) {
  //  v_sendData.push_back(0x00);
  //}
  //for (int i=0; i<16777216/2;i++) {
  //  v_sendData.push_back(0x00);
  //}

  //writeData = v_sendData;
  ////// bit flip Quick data
  ////bitFlipData(writeData);

  //// Write data
  //cout<<"[Start] write PROM"<<endl;
  //status = Mt25FlashHighLevel::writeProm(writeData, failMessage);
  //if (!status) cout<<failMessage<<endl;
  //cout<<"[End  ] write PROM"<<endl;

  //// Read all
  //cout<<"[Start] read PROM"<<endl;
  //status = Mt25FlashHighLevel::readProm(readData, failMessage);
  //if (!status) cout<<failMessage<<endl;
  //cout<<"[End  ] read PROM"<<endl;

  //// Validate
  //expectedData = writeData;
  //validateData(readData, expectedData, errorAddress);
  //cout<<"Number of errors: "<<errorAddress.size()<<endl;

  //// Re-read errors
  //printf("%#04x %#04x\n", writeData[0], writeData[2]);
  //errorAddress.push_back(0);
  //errorAddress.push_back(2);
  //vector<unsigned char> rereadData;
  //cout<<"[Start] reread PROM"<<endl;
  //status = Mt25FlashHighLevel::rereadProm(errorAddress, rereadData, failMessage);
  //if (!status) cout<<failMessage<<endl;
  //cout<<"[End ] reread PROM"<<endl;

  //// Validate
  //extractData(errorAddress, writeData, expectedData);
  //validateData(rereadData, expectedData, errorAddress);
  //cout<<"Number of errors: "<<errorAddress.size()<<endl;


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

