/**
 * @file config.h
 * @author Simon Wilkes (simonwilkes@hotmail.de)
 * @brief 
 * @version 1.0
 * @date 2021-01-31
 * 
 * 
 */
#ifndef _CONFIG_
#define _CONFIG_

//===================================================================================================
//Wifi parameters 
#define MYSSID "testMe"
#define MYPASS "wasd1234"

//===================================================================================================
//SD parameters 

//chip select Pin for SD use
#define CS_SD 18 
//number of pins used for Metadate coding and decoding
#define PIN_COUNT 2 
// log file name.  Integer field before dot will be incremented.
#define LOG_FILE_NAME "ADC0000.bin" 
// use of Dedicated SPI not possible due to it sharing a line with Wifi
#define ENABLE_DEDICATED_SPI 0  
// deactivated
#define ERROR_PIN -1 



// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 1   

// Test with reduced SPI speed for breadboards.  SD_SCK_MHZ(4) will select
// the highest speed supported by the board that is not over 4 MHz.
// Change SPI_SPEED to SD_SCK_MHZ(50) for best performance. Unsave. Only change with thorough testing
#define SPI_SPEED SD_SCK_MHZ(4)




#endif //_CONFIG_