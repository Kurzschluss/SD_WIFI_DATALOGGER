/******************************************************************************************
/*	sketch.h
	Created: 09.11.2019
	Author: Julian Peters
	Last Modified: 02.01.2020
	By: Julian Peters
 
	Definitions of Sketch.cpp that are needed by other files are written here.
******************************************************************************************/ 



#ifndef SKETCH_H_
#define SKETCH_H_


// ------------------------------------------------------------------------------------
// Includes

//#include "Configuration.h"
#include <assert.h>



// ------------------------------------------------------------------------------------
// variable definitions

struct meta_t
{
	//uint32_t durationMicros;
	uint32_t durationMillis;
	uint32_t durationRtcSecs;
	uint32_t recordsWritten;
	uint32_t recordsExpectedRtc;
	uint32_t recordsExpectedMillis;
	int16_t deviationRtc;
	int16_t deviationMillis;
	uint16_t tolerance;
	char rtcStamp[18];	// 17 stamp + 1 \0
	
	// initialize sampleFrequency according to chosen value from configuration.h
	const uint16_t sampleFrequency = SAMPLE_FREQ;
	
};

const int META_COLS = 10;

// Struct to store Sample Values
// used in block_t
struct data_t
{
	#if (WRITE_TIME_TO_SD == 1)
	uint32_t timeStampMillis;
	#endif
	
	uint16_t batV;
	uint16_t batC;
	uint16_t mic;
	
	#if (SAMPLE_ACCEL == 1)
	int16_t accelX;
	int16_t accelY;
	int16_t accelZ;
	#endif
	#if (SAMPLE_GYRO == 1)
	int16_t gyroX;
	int16_t gyroY;
	int16_t gyroZ;
	#endif
	#if (SAMPLE_MAG == 1)
	int16_t magX;
	int16_t magY;
	int16_t magZ;
	#endif
};

const uint16_t DATA_SIZE = sizeof(data_t);

// DATA_DIM: Number of data records in a block.
// 512: Target Block Size
const uint16_t DATA_DIM = 512 / DATA_SIZE;

// Calculate fill so block size is 512 bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = 512 - DATA_DIM * DATA_SIZE;

// Create 512 Byte Blocks
// Important: Keep order of members,
// otherwise size Problems due to memory alignment will occur
// -> sizeof(block_t) > 512 will cause errors
struct block_t
{
	data_t data[DATA_DIM];
	uint8_t fill[FILL_DIM];	//Overhead to fill up the block size to get complete 512 Size
};

// calculated meta_t size
const uint16_t META_SIZE = sizeof(meta_t);
// real calculated block_t size to verify if the target block size (512) is kept
const uint16_t BLOCK_SIZE = sizeof(block_t);


// check if sizeof(block_t) is 512
// if not produce compile-time error
static_assert(BLOCK_SIZE == 512, "sizeof(block_t) is NOT 512. Check the data_t struct.");


// ------------------------------------------------------------------------------------
// function prototypes

/**************************************************************************************
/*	fatalBlink	
	
	@brief	Blinks the red LED to indicate Error.
			This Method will never return, 
**************************************************************************************/
void fatalBlink(void);

bool checkIntArray(uint8_t* array, uint8_t length, uint8_t toCheck);


#endif /* SKETCH_H_ */