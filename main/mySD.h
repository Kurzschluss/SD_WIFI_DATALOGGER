/******************************************************************************************
/*	mySD.h
	Created: 13.01.2021
	Author: Simon Wilkes
	Last Modified: 13.01.2021
	By: Simon Wilkes
 
	Definitions of mySD.cpp that are needed by other files are written here.
******************************************************************************************/
#ifndef _MYSD_
#include "stdint.h"
#include "stddef.h"
#include "SdFat.h"
#include "BufferedPrint.h"
#include "FreeStack.h"
// ------------------------------------------------------------------------------------
// Definitions

#define ENABLE_DEDICATED_SPI 0
#define CS_SD 18
#define PIN_COUNT 2

//------------------------------------------------------------------------------
// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 1

// Test with reduced SPI speed for breadboards.  SD_SCK_MHZ(4) will select
// the highest speed supported by the board that is not over 4 MHz.
// Change SPI_SPEED to SD_SCK_MHZ(50) for best performance.
#define SPI_SPEED SD_SCK_MHZ(4)


// log file name.  Integer field before dot will be incremented.
#define LOG_FILE_NAME "AvrAdc00.bin"

//------------------------------------------------------------------------------
// File definitions.
//
// Maximum file size in bytes.
// The program creates a contiguous file with MAX_FILE_SIZE_MiB bytes.
// The file will be truncated if logging is stopped early.
const uint32_t MAX_FILE_SIZE_MiB = 100;  // 100 MiB file.

const size_t BLOCK_SIZE = 64;
//------------------------------------------------------------------------------
// First block of file.
const size_t PIN_NUM_DIM = BLOCK_SIZE - 3*sizeof(uint32_t) - 2*sizeof(uint8_t);
struct metadata_t {
  uint32_t adcFrequency;           // ADC clock frequency
  uint32_t cpuFrequency;           // CPU clock frequency
  uint32_t sampleInterval;         // Sample interval in CPU cycles.
  uint8_t recordEightBits;         // Size of ADC values, nonzero for 8-bits.
  uint8_t pinCount;                // Number of analog pins in a sample.
  uint8_t pinNumber[PIN_NUM_DIM];  // List of pin numbers in a sample.
};

const size_t DATA_DIM16 = (BLOCK_SIZE - 2*sizeof(uint16_t))/sizeof(uint16_t);
struct block16_t {
  unsigned short count;    // count of data values
  unsigned short overrun;  // count of overruns since last block
  unsigned short data[DATA_DIM16];
};




// ------------------------------------------------------------------------------------
// Function prototypes

/**************************************************************************************
/*	functionName
	
	@brief	this is a stub
			
			
	
	@return stub
***************************************************************************************/
void initSD();

//------------------------------------------------------------------------------
// Convert binary file to csv file.
void binaryToCsv();
void binaryToTcp();
void csvOverTcp();

//------------------------------------------------------------------------------
void createBinFile();
//------------------------------------------------------------------------------
bool createCsvFile();
//------------------------------------------------------------------------------
void openBinFile();

//------------------------------------------------------------------------------
// Print data file to Serial
void printData();

void bufferData(uint16_t d);

void logData();

void printUnusedStack();

void clearSerialInput();

bool serialReadLine(char* str, size_t size);

void readls();




#endif // _MYSD_
