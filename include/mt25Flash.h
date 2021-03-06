#ifndef MT25Flash
#define MT25Flash

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

#include "ftd2xx.h"
#include "libft4222.h"

#define SPI_FLASH_MAX_WRITE_SIZE 256
#define FT4222_MAX_READ_SIZE     65535

#define CmdReadStatusRegister   0x05
#define CmdWriteEnable          0x06
#define CmdSectorErase          0xD8
#define CmdQuadFastProgram      0x38
#define CmdPageProgram          0x02
#define CmdQuadFastRead         0x6B
#define CmdRead                 0x03

namespace Mt25Flash {

  unsigned char readStatusRegister(FT_HANDLE ftHandle) {
      std::vector<unsigned char> writeBuffer;
      writeBuffer.push_back(CmdReadStatusRegister);
      writeBuffer.push_back(0xFF);

      std::vector<unsigned char> readBuffer;
      readBuffer.resize(2);

      uint16 sizeTransferred;
      FT4222_STATUS ftStatus;

      // FT4222_SPIMaster_SingleReadWrite(ftHandle, readBuffer, writeBuffer, bufferSize, sizeTransferred, isEndTransaction)
      ftStatus = FT4222_SPIMaster_SingleReadWrite(ftHandle, &readBuffer[0], &writeBuffer[0], writeBuffer.size(), &sizeTransferred, true);
      if((ftStatus!=FT4222_OK) ||  (sizeTransferred!=writeBuffer.size())) {
        std::cout<<"readStatusRegister failed"<<std::endl;
        return 0xff;
      }

      return readBuffer[1];
  }

  bool waitForFlashReady(FT_HANDLE ftHandle, bool verbose = false) {
    const int WAIT_FLASH_READY_TIMES = 500;

    int waitTime = 0;
    for(int i=0; i<WAIT_FLASH_READY_TIMES; ++i) {
      std::vector<unsigned char> writeBuffer;
      writeBuffer.push_back(CmdReadStatusRegister);
      writeBuffer.push_back(0xFF);

      std::vector<unsigned char> readBuffer;
      readBuffer.resize(2);

      uint16 sizeTransferred;
      FT4222_STATUS ftStatus;

      // FT4222_SPIMaster_SingleReadWrite(ftHandle, readBuffer, writeBuffer, bufferSize, sizeTransferred, isEndTransaction)
      ftStatus = FT4222_SPIMaster_SingleReadWrite(ftHandle, &readBuffer[0], &writeBuffer[0], writeBuffer.size(), &sizeTransferred, true);
      if((ftStatus!=FT4222_OK) ||  (sizeTransferred!=writeBuffer.size())) {
        std::cout<<"waitForFlashReady failed"<<std::endl;
        return false;
      }

      if (verbose) printf("[0] %#08x [1] %#08x\n", readBuffer[0], readBuffer[1]);
      if ((readBuffer[1] & 0x01) == 0x00) { // not in write operation 
        if(verbose && waitTime!= 0) std::cout<<"Waiting for flash for "<<waitTime<<" milliseconds"<<std::endl;
        return true;
      }

      waitTime++;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }


    std::cout<<"waitForFlashReady failed"<<std::endl;
    return false;
  }

  bool runFlashCommand(FT_HANDLE & ftHandle, unsigned char command, unsigned nBytes) {
    // Set to spi mode
    FT4222_STATUS ftStatus;
    ftStatus = FT4222_SPIMaster_SetLines(ftHandle,SPI_IO_SINGLE);
    if (FT4222_OK != ftStatus) {
      std::cout<<"FT4222_SPIMaster_SetLines failed"<<std::endl;
      std::cout<<"runFlashCommand failed"<<std::endl;
      return 0;
    }
    if(!waitForFlashReady(ftHandle)) {
      return false;
      std::cout<<"runFlashCommand failed"<<std::endl;
    }

    // Set write and read buffers
    std::vector<unsigned char> writeBuffer;
    writeBuffer.push_back(command);
    for (int i=0; i<nBytes; ++i) writeBuffer.push_back(0xFF);
    std::vector<unsigned char> readBuffer;
    readBuffer.resize(nBytes+1);

    // Execute command
    uint16 sizeTransferred;
    // FT4222_SPIMaster_SingleReadWrite(ftHandle, readBuffer, writeBuffer, bufferSize, sizeTransferred, isEndTransaction)
    ftStatus = FT4222_SPIMaster_SingleReadWrite(ftHandle, &readBuffer[0], &writeBuffer[0], writeBuffer.size(), &sizeTransferred, true);

    if (ftStatus!=FT4222_OK) {
      std::cout<<"runFlashCommand failed"<<std::endl;
      printf("command: %#04x\n", command);
      return false;
    }
    else {
      printf("Ran command: %#04x\n", command);
      for (int i = 1; i < readBuffer.size(); ++i) printf("Data[%i]: %#04x\n", i, readBuffer[i]);
    }

    if(!waitForFlashReady(ftHandle)) {
      return false;
      std::cout<<"runFlashCommand failed"<<std::endl;
    }

    return true;
  } 

  bool writeEnableCommand(FT_HANDLE & ftHandle, bool verbose = false)
  {
    const int WAIT_FLASH_READY_TIMES = 500;

    uint8 outBuffer = CmdWriteEnable;
    uint16 sizeTransferred;
    FT4222_STATUS ftStatus;

    // Fixing bug that write enable is not set
    for(int i=0; i<WAIT_FLASH_READY_TIMES; ++i) {
      ftStatus = FT4222_SPIMaster_SingleWrite(ftHandle, &outBuffer, 1, &sizeTransferred, true);
      if((ftStatus!=FT4222_OK) ||  (sizeTransferred!=1)) {
        std::cout<<"writeEnableCommand failed"<<std::endl;
        return false;
      }

      uint16 statusRegister = readStatusRegister(ftHandle);
      if ((statusRegister & 0x03) == 0x02) {
        return true;
      }
      if (verbose) printf("statusRegister: %#08x Try setting write enable again\n", statusRegister);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout<<"writeEnableCommand failed"<<std::endl;
    return false;
  }

  bool blockEraseCommand(FT_HANDLE ftHandle, uint32 address)
  {
    uint16 sizeTransferred;
    FT4222_STATUS ftStatus;
    std::vector<unsigned char> writeBuffer;

    writeBuffer.push_back(CmdSectorErase);
    writeBuffer.push_back((unsigned char)((address & 0xFF0000) >> 16));
    writeBuffer.push_back((unsigned char)((address & 0x00FF00) >> 8));
    writeBuffer.push_back((unsigned char)( address & 0x0000FF));

    ftStatus = FT4222_SPIMaster_SingleWrite(ftHandle, &writeBuffer[0], writeBuffer.size(), &sizeTransferred, true);
    if((ftStatus!=FT4222_OK) ||  (sizeTransferred != writeBuffer.size())) {
      std::cout<<"blockEraseCommand failed"<<std::endl;
      return false;
    }

    if(!waitForFlashReady(ftHandle)) {
      return false;
      std::cout<<"blockEraseCommand failed"<<std::endl;
    }

    return true;
  }

  bool eraseFlash(FT_HANDLE & ftHandle, uint32 startAddress, uint32 endAddress) {
    FT4222_STATUS ftStatus;

    ftStatus = FT4222_SPIMaster_SetLines(ftHandle,SPI_IO_SINGLE);
    if (FT4222_OK != ftStatus) {
      std::cout<<"set spi single line failed!"<<std::endl;
      return 0;
    }

    if (!waitForFlashReady(ftHandle)) {
      std::cout<<"EraseFlash failed"<<std::endl;
      return false;
    }

    while (startAddress < endAddress) {
      if(!writeEnableCommand(ftHandle)) {
        std::cout<<"EraseFlash failed"<<std::endl;
        return false;
      }

      if(!blockEraseCommand(ftHandle, startAddress)) {
        std::cout<<"EraseFlash failed"<<std::endl;
        return false;
      }

      startAddress += 0x10000;

      if(!waitForFlashReady(ftHandle)) {
        std::cout<<"EraseFlash failed"<<std::endl;
        return false;
      }
      else continue;
    }
    return true;
  }

  bool quadIOPageProgramCommand(FT_HANDLE ftHandle, uint32 address, unsigned char * pData , uint16 size)
  {
    uint32 sizeOfRead;
    FT4222_STATUS ftStatus;
    std::vector<unsigned char> writeBuffer;

    writeBuffer.push_back(CmdQuadFastProgram);
    writeBuffer.push_back((unsigned char)((address & 0xFF0000) >> 16));
    writeBuffer.push_back((unsigned char)((address & 0x00FF00) >> 8));
    writeBuffer.push_back((unsigned char)( address & 0x0000FF));

    writeBuffer.insert(writeBuffer.end(), pData, (pData + size));

    // FT4222_SPIMaster_MultiReadWrite(ftHandle, readBuffer, writeBuffer, singleWriteBytes, multiWriteBytes, sizeOfRead)
    ftStatus = FT4222_SPIMaster_MultiReadWrite(ftHandle, NULL, &writeBuffer[0], 1, 3+size, 0, &sizeOfRead);
    if(ftStatus!=FT4222_OK) {
      std::cout<<"FT4222_SPIMaster_MultiReadWrite failed"<<std::endl;
      std::cout<<"quadIOPageProgramCommand failed"<<std::endl;
      return false;
    }

    return true;
  }

  bool qspiFlashWrite(FT_HANDLE & ftHandle, uint32 startAddress, std::vector<unsigned char> sendData, bool verbose=false) {
    unsigned nBytes = sendData.size();
    FT_STATUS ftStatus;

    // Erase flash
    std::cout<<"Start flash erase"<<std::endl;
    if(!eraseFlash(ftHandle, startAddress, startAddress + nBytes)) {
      std::cout<<"qspiFlashWrite failed"<<std::endl;
      return 0;
    } else {
      std::cout<<"End flash erase"<<std::endl;
    }

    // Write Flash
    uint32 notSentByte = nBytes;
    uint32 sentByte=0;

    int printCounter = 1;
    int counter = 0;
    std::cout<<"Start flash write"<<std::endl;
    while (notSentByte > 0) {
      counter++;
      uint16 data_size = std::min<size_t>(SPI_FLASH_MAX_WRITE_SIZE, notSentByte);
      // Prevent from going over page
      if (counter==1) data_size = data_size - startAddress;

      ftStatus = FT4222_SPIMaster_SetLines(ftHandle,SPI_IO_SINGLE);
      if (FT4222_OK != ftStatus) {
        std::cout<<"set spi single line failed!"<<std::endl;
        std::cout<<"qspiFlashWrite failed"<<std::endl;
        return 0;
      }

      if (!writeEnableCommand(ftHandle)) {
        std::cout<<"qspiFlashWrite failed"<<std::endl;
        return 0;
      }

      ftStatus = FT4222_SPIMaster_SetLines(ftHandle,SPI_IO_QUAD);
      if (FT4222_OK != ftStatus) {
        std::cout<<"set spi quad line failed!"<<std::endl;
        std::cout<<"qspiFlashWrite failed"<<std::endl;
        return 0;
      }

      if(!quadIOPageProgramCommand(ftHandle, startAddress+sentByte, &sendData[sentByte], data_size)) {
        std::cout<<"qspiFlashWrite failed"<<std::endl;
        return 0;
      } else {
        if (verbose && (printCounter == counter)) {
          printf("writing flash start address =[%#08x] %d bytes\n", startAddress+sentByte, data_size);
          //printCounter = printCounter * 2;
          printCounter = printCounter + 1;
        }
      }
			if (counter%100==0) std::cout<<"Wrote up to "<<counter*SPI_FLASH_MAX_WRITE_SIZE<<" bits."<<std::endl;

      notSentByte -= data_size;
      sentByte    += data_size;
    }
    std::cout<<"End flash write"<<std::endl;

    ftStatus = FT4222_SPIMaster_SetLines(ftHandle,SPI_IO_SINGLE);
    if (FT4222_OK != ftStatus) {
      std::cout<<"set spi single line failed!"<<std::endl;
      std::cout<<"qspiFlashWrite failed"<<std::endl;
      return false;
    }
    if(!waitForFlashReady(ftHandle)) {
      std::cout<<"qspiFlashWrite failed"<<std::endl;
      return false;
    }

    return true;
  }

  bool pageProgramCommand(FT_HANDLE ftHandle, uint32 address, unsigned char * pData , uint16 size)
  {
    uint16 sizeTransferred;
    FT4222_STATUS ftStatus;
    std::vector<unsigned char> writeBuffer;

    writeBuffer.push_back(CmdPageProgram);
    writeBuffer.push_back((unsigned char)((address & 0xFF0000) >> 16));
    writeBuffer.push_back((unsigned char)((address & 0x00FF00) >> 8));
    writeBuffer.push_back((unsigned char)( address & 0x0000FF));

    writeBuffer.insert(writeBuffer.end(), pData, (pData + size));

    ftStatus = FT4222_SPIMaster_SingleWrite(ftHandle, &writeBuffer[0], writeBuffer.size(), &sizeTransferred, true);
    if((ftStatus!=FT4222_OK) ||  (sizeTransferred != writeBuffer.size())) {
      std::cout<<"pageProgramCommand failed"<<std::endl;
      return false;
    }

    return true;
  }

  bool spiFlashWrite(FT_HANDLE & ftHandle, uint32 startAddress, std::vector<unsigned char> sendData, bool verbose = false) {
    unsigned nBytes = sendData.size();
    FT_STATUS ftStatus;

    // Erase flash
    if(!eraseFlash(ftHandle, startAddress, startAddress + nBytes)) {
      std::cout<<"spiFlashWrite failed"<<std::endl;
      return 0;
    } else {
      std::cout<<"Flash was erased"<<std::endl;
    }

    // Write flash
    uint32 notSentByte = nBytes;
    uint32 sentByte=0;

    int counter = 0;
    while (notSentByte > 0) {
      counter++;
      uint16 data_size = std::min<size_t>(SPI_FLASH_MAX_WRITE_SIZE, notSentByte);
      // Prevent from going over page
      if (counter==1) data_size = data_size - startAddress;

      if(!waitForFlashReady(ftHandle)) {
        std::cout<<"spiFlashWrite failed"<<std::endl;
        return 0;
      }

      if(!writeEnableCommand(ftHandle)) {
        std::cout<<"spiFlashWrite failed"<<std::endl;
        return 0;
      }

      if(!pageProgramCommand(ftHandle, startAddress+sentByte, &sendData[sentByte], data_size)) {
        std::cout<<"spiFlashWrite failed"<<std::endl;
        return 0;
      } else {
        printf("writing flash start address =[%#08x] %d bytes\n", startAddress+sentByte, data_size);
      }

      notSentByte -= data_size;
      sentByte    += data_size;
    }

    if(!waitForFlashReady(ftHandle)) {
      std::cout<<"spiFlashWrite failed"<<std::endl;
      return 0;
    }

    return true;
  }

	std::string makeTestData(uint32 sendDataSize, int fill=-1) {
		std::string sendData;
		sendData.resize(sendDataSize);
		for (unsigned iData = 0; iData < sendDataSize; ++iData) {
      if (fill == -1) sendData[iData] = (uint8)iData;
      else sendData[iData] = (uint8)fill;
    }
		//// Print test data
    //std::vector<unsigned char> t_sendData;
    //for (int i=0; i<sendDataSize;i++) {
    //  t_sendData.push_back(sendData[i]);
    //}
    //for (int i=0; i<t_sendData.size(); ++i) printf("send data[%i] %#04x\n", i, t_sendData[i]);
		return sendData;
	}

  bool readDataBytesCommand(FT_HANDLE ftHandle, uint32 address, unsigned char * pData , uint32 size)
  {
    FT4222_STATUS ftStatus;
    uint16 sizeTransferred;

    std::vector<unsigned char> writeBuffer;
    writeBuffer.push_back(CmdRead);
    writeBuffer.push_back((unsigned char)((address & 0xFF0000) >> 16));
    writeBuffer.push_back((unsigned char)((address & 0x00FF00) >> 8));
    writeBuffer.push_back((unsigned char)( address & 0x0000FF));

    // FT4222_SPIMaster_SingleWrite(ftHandle, buffer, bufferSize, sizeTransferred, isEndTransaction)
    ftStatus = FT4222_SPIMaster_SingleWrite(ftHandle, &writeBuffer[0], writeBuffer.size(), &sizeTransferred, false);
    if((ftStatus!=FT4222_OK) ||  (sizeTransferred != writeBuffer.size())) {
			std::cout<<"FT4222_SPIMaster_SingleWrite failed"<<std::endl;
			std::cout<<"readDataBytesCommand failed"<<std::endl;
      return false;
    }

    // FT4222_SPIMaster_SingleRead(ftHandle, buffer, bufferSize, sizeOfRead, isEndTransaction)
    ftStatus = FT4222_SPIMaster_SingleRead(ftHandle, pData, size, &sizeTransferred, true);
    if((ftStatus!=FT4222_OK) ||  (sizeTransferred != size)) {
      std::cout<<"FT4222_SPIMaster_SingleRead failed"<<std::endl;
      std::cout<<"readDataBytesCommand failed"<<std::endl;
      return false;
    }

    return true;
  }

  bool spiFlashRead(FT_HANDLE & ftHandle, uint32 startAddress, unsigned nBytes, std::vector<unsigned char> & readData, bool verbose = false) {
    FT4222_STATUS ftStatus;
    readData.resize(nBytes);
		uint32 notReadByte = nBytes;
		uint32 readByte = 0;

		while (notReadByte > 0) {
      uint16 dataSize = std::min<size_t>(FT4222_MAX_READ_SIZE, notReadByte);

      if(!readDataBytesCommand(ftHandle, startAddress+readByte, &readData[readByte], dataSize)) {
        std::cout<<"spiFlashRead failed"<<std::endl;
        return 0;
      } else {
        if (verbose) printf("reading flash start address =[%#08x] %d bytes\n", startAddress+readByte, dataSize);
      }

      notReadByte -= dataSize;
      readByte    += dataSize;
    }

    if(!waitForFlashReady(ftHandle)) {
      std::cout<<"spiFlashRead failed"<<std::endl;
      return false;
    }

    return true;
  }

  bool fastQuadReadCommand(FT_HANDLE ftHandle, uint32 address, unsigned char * pData , uint32 size, int nDummyDivide2=4)
  {
    FT4222_STATUS ftStatus;
    uint32 sizeTransferred;
    std::vector<unsigned char> writeBuffer;
    writeBuffer.push_back(CmdQuadFastRead);
    writeBuffer.push_back((unsigned char)((address & 0xFF0000) >> 16));
    writeBuffer.push_back((unsigned char)((address & 0x00FF00) >> 8));
    writeBuffer.push_back((unsigned char)( address & 0x0000FF));
    for (int i=0; i<nDummyDivide2; ++i) writeBuffer.push_back(0xFF);

    // FT4222_SPIMaster_MultiReadWrite(ftHandle, readBuffer, writeBuffer, singleWriteBytes, multiWriteBytes, sizeOfRead)
    ftStatus = FT4222_SPIMaster_MultiReadWrite(ftHandle, pData, &writeBuffer[0], 1+3, nDummyDivide2, size, &sizeTransferred);
    if((ftStatus!=FT4222_OK) ||  (sizeTransferred != size)) {
      std::cout<<"FT4222_SPIMaster_MultiReadWrite failed"<<std::endl;
      std::cout<<"fastQuadReadCmd failed"<<std::endl;
      return false;
    }

    return true;
  }

  bool qspiFlashRead(FT_HANDLE & ftHandle, uint32 startAddress, unsigned nBytes, std::vector<unsigned char> & readData, bool verbose = false) {
    FT4222_STATUS ftStatus;
    readData.resize(nBytes);
		uint32 notReadByte = nBytes;
		uint32 readByte = 0;

		while (notReadByte > 0) {
      uint16 dataSize = std::min<size_t>(FT4222_MAX_READ_SIZE, notReadByte);

      ftStatus = FT4222_SPIMaster_SetLines(ftHandle,SPI_IO_QUAD);
      if (FT4222_OK != ftStatus) {
        std::cout<<"FT4222_SPIMaster_SetLines failed"<<std::endl;
        std::cout<<"qspiFlashRead failed"<<std::endl;
        return 0;
      }

      // Read flash
      if(!fastQuadReadCommand(ftHandle, startAddress+readByte, &readData[readByte], dataSize)) {
        std::cout<<"qspiFlashRead failed"<<std::endl;
        return 0;
      } else {
        if (verbose) printf("reading flash start address =[%#08x] %d bytes\n", startAddress+readByte, dataSize);
      }

      ftStatus = FT4222_SPIMaster_SetLines(ftHandle,SPI_IO_SINGLE);
      if (FT4222_OK != ftStatus) {
        std::cout<<"FT4222_SPIMaster_SetLines failed"<<std::endl;
        std::cout<<"fastQuadReadCmd failed"<<std::endl;
        return 0;
      }
      waitForFlashReady(ftHandle);

      notReadByte -= dataSize;
      readByte    += dataSize;
    }

    return true;
  }

}
#endif
