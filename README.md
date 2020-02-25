Software for writing and reading a QSPI flash (MT25QU128ABA8ESF-0SIT) using
a USB FTDI development module (UMFT4222EV), which has FT4222H.

# Description

Computer software is required to communicate with the UMFT4222EV 
through a USB connection. With a board that connects UMFT4222EV 
to MT25QU128ABA8ESF-0SIT, one can then communicate with the MT25QU128ABA8ESF-0SIT
through the USB connection.
This repository provides the software required for commuicating with the UMFT4222EV,
which then communicates with MT25QU128ABA8ESF-0SIT through the QSPI interface.

# Installation for mac

## Step 1. Install the FT4222H library and dependencies

### Step 1.1 Download the FT4222H library

Download the Mac OSX FT4222H library, which can be 
found at https://www.ftdichip.com/Products/ICs/FT4222H.html. 
Move relevent files to appropriate folders

~~~~bash
mkdir downloads; cd downloads
wget https://www.ftdichip.com/Support/SoftwareExamples/LibFT4222-mac-v1.4.2.184.zip
unzip LibFT4222-mac-v1.4.2.184.zip
hdiutil attach libft4222.1.4.2.184.dmg
cd ../
mkdir lib
cp /Volumes/ft422/libft4222.1.4.2.184.dylib lib
mkdir include #if not already there
cp /Volumes/ft422/\*.h include
hdiutil detach /Volumes/ft422
~~~~

### Step 1.2 Install the boost library (v1.66) that is requried for the FT4222H library

~~~~bash
cd downloads
wget -O boost_1_66_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.66.0/boost_1_66_0.tar.gz/download
tar -xvf boost_1_66_0.tar.gz
cd boost_1_66_0
./bootstrap.sh --prefix=$(pwd)/../../boost-1.66.0
./b2
./b2 install
~~~~

# Step 2. Setup the environment and make the software

~~~~bash
export DYLD_LIBRARY_PATH=$PWD/lib/:$PWD/boost-1.66.0/lib${DYLD_LIBRARY_PATH:+:${DYLD_LIBRARY_PATH}}
make
~~~~

# Step 3. Run software

Below is a list of the software
- FT4222H status checker: readFt4222Status
- MT25QU128ABA8ESF status checker: readMt25StatusChecker

- MT25QU128ABA8ESF writer : mt25Writer
- MT25QU128ABA8ESF reader : mt25Reader
- MT25QU128ABA8ESF write and read test : mt25Tester

TODO
+Test board out+
+Try saving latex file+
+Fix make file to include header file changes+
+Add c++11+
+Writing in qspi is not working sometimes. Needed to add more time..+
+Add system clock modifier+
+Make test script test with more data+
+Fix read and write when data is large+
+spi, qspi failed (Let's find why qspi read failed)+
+qspi, qspi tested+
+Bug fix for read and write when data is large+
+qspi read was sped up+
+Sped up qspi write+


Make read structure more like write... to make it easier for computer to read stuff.
Make tester print diff stuff in case things are not working
Change things to inline to make it quicker
Add reading flash start address (Test)
Make verbose flag for operations


Find why dummy is strange
fix warnings
Make command for previous cmdFastQuadRead
Answer quesiton on mt25Flash.h line 21

Unmark * in readme
