/**
 * @file mySD.h
 * @author Simon Wilkes (simonwilkes@hotmail.de)
 * @brief provides class for controlling SdFat 
 * @version 1.0
 * @date 2021-01-31 
 */
#ifndef _MYSD_
#define _MYSD_

#include "stdint.h"
#include "stddef.h"
#include "SdFat.h"
#include "BufferedPrint.h"
#include "FreeStack.h"

#include "myrtc.h"
#include "tcp.h"
#include "config.h"



constexpr int evallength(){
    return sizeof(LOG_FILE_NAME);
}
// Temporary log file.  Will be deleted if a reset or power failure occurs.
#define TMP_FILE_NAME "tmp_adc.bin" 



//------------------------------------------------------------------------------
// File definitions.

//due to using FAT32
typedef File32 file_t; 

// Maximum file size in bytes.
// The program creates a contiguous file with MAX_FILE_SIZE_MiB bytes.
// The file will be truncated if logging is stopped early.
const uint32_t MAX_FILE_SIZE_MiB = 100;  // 100 MiB file.
const uint32_t MAX_FILE_SIZE = MAX_FILE_SIZE_MiB << 20;


const size_t BLOCK_SIZE = 64;

//------------------------------------------------------------------------------
// FIFO size definition. Use a multiple of 512 bytes for best performance.
const size_t FIFO_SIZE_BYTES = 4*512;



//------------------------------------------------------------------------------
// First block of file. Work not finished. Could be bullshit data. But surfs the pupose of reserving space
const size_t PIN_NUM_DIM = BLOCK_SIZE - 3*sizeof(uint32_t) - 2*sizeof(uint8_t);
struct metadata_t {
    uint32_t adcFrequency;           // ADC clock frequency
    uint32_t cpuFrequency;           // CPU clock frequency
    uint32_t sampleInterval;         // Sample interval in CPU cycles.
    uint8_t recordEightBits;         // Size of ADC values, nonzero for 8-bits.
    uint8_t pinCount;                // Number of analog pins in a sample.
    uint8_t pinNumber[PIN_NUM_DIM];  // List of pin numbers in a sample.
};

// Defindes the memory of fifo Blocks
const size_t DATA_DIM16 = (BLOCK_SIZE - 2*sizeof(uint16_t))/sizeof(uint16_t);
struct block_t {
    unsigned short count;    // count of data values
    unsigned short overrun;  // count of overruns since last block
    unsigned short data[DATA_DIM16];
};

// Size of FIFO in blocks.
size_t const FIFO_DIM = FIFO_SIZE_BYTES/sizeof(block_t);


//------------------------------------------------------------------------------
// Error messages stored in flash.
#define error(msg) (Serial.println(F(msg)),errorHalt())
#define assert(e) ((e) ? (void)0 : error("assert: " #e))


/**
 * @brief provides means for writing from and to the SD
 * 
 */
class MySD{
    public:
        MySD();
        void initSD();
        void binaryToCsv();
        void csvOverTcp(MyWifi* wifi);
        void createBinFile();
        bool createCsvFile();
        void bufferData(uint16_t d);
        void logData(MyRTC* rtc);

    private:
        void errorHalt();

        SdFat32 sd;

        // name of File on SD
        char binName[evallength()] = LOG_FILE_NAME;
        // Maximum length name including zero byte. 13 is added because that is the minimum number 
        // of bytes required for getting the filename
        const size_t NAME_DIM = evallength()+13; 

        
        // SD chip select pin.
        const uint8_t SD_CS_PIN = CS_SD; 

        file_t binFile;
        file_t csvFile;

        block_t* fifoData; 
        // volatile - shared, ISR and background.
        volatile size_t fifoCount = 0; 
        // Only accessed by ISR during logging.
        size_t fifoHead = 0;  
        // Only accessed by writer during logging.
        size_t fifoTail = 0;          
        // Pointer to current buffer.        
        block_t* isrBuf = nullptr;  
        // overrun count
        uint16_t isrOver = 0;   
};


#endif // _MYSD_
