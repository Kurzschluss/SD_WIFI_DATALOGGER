/********************************************************************************************
/*	Sketch.cpp
	Created:		05.10.2019
	Author:			Julian Peters
	Last Modified:	02.01.2020
	Modified by:	Julian Peters
	
	Description:
	
	Sketch.cpp (Arduino-Standard) is the entry-point for the whole application. 
	Functions "setup" and "loop" are a legacy from the Arduino Import which comes with the 
	use of some Arduino Libraries.
	"setup" is nested at the beginning of the c-standard main-function and is called once at 
	the beginning (1. function of Datalogger Project to be executed)
	"loop" is also nested in the cpp-standard main-function in a infinity-loop
	
	The main-function can be found in the file main.cpp which is in the ArduinoCore at 
	"src/core/main.cpp".
	The initialization of the clocks is in startup.c in the ArduinoCore at 
	"src/core/startup.c". This file was modified by the author to achieve a more accurate 
	clock frequency. See also annotations in the startup.c file
	
	The application works like a combination of a "State-Machine" in the main-loop with 
	Timer-Interrupts and External-Interrupts.
	
	The External-Interrupts (Buttons) start and stop the measurement sessions by setting 
	the start- and stopSession flags in the EIC_Handler function. 
	These flags are checked in the state-machine main-loop where the startSession and 
	stopSession functions are called and processed.
	The startSession function resets counters, initializes the SD-Card, opens a new file on 
	it and starts the Timer-Interrupts.
	The stopSession function stops the Timer-Interrupts and closes the file on the SD-Card.
	
	The Timer-Interrupts (TC3_Handler) take new samples from the sensors periodically. 
	One collection of sample data from the various sensors is called dataset or record
	and combined in a struct data_t (see sketch.h). 
	The data_t structs are collected in so-called "blocks" of size 512 B. When one block is
	full with records, it is pushed in a circular-buffer (ring). 
	The main-loop checks if this buffer has entries (blocks), pulls them out and writes them to 
	the SD-Card using a low-level writing method.
	
******************************************************************************************/



// ------------------------------------------------------------------------------------
// Includes

#include "Arduino.h"
#include "Sketch.h"

#define CIRCULAR_BUFFER_INT_SAFE
#include "CircularBuffer.h"

#include "TimerSetup.h"
#include "Configuration.h"
#include "BoardSetup.h"
#include "AdcSetup.h"
#include "I2cSetup.h"
#include "SpiSetup.h"
#include "RtcSetup.h"
#include "ImuLsmSetup.h"
#include "ImuBmiSetup.h"
#include "SdCardExport.h"



// ------------------------------------------------------------------------------------
// local function prototypes

int16_t checkSum(void);
void startSession(void);
void closeSession(void);
void toggleSession(void);
void testSpeed(void);

// Timer Interrupt callback function
void TC3_Handler();
// Button Interrupt callback function
void EIC_Handler();

// ------------------------------------------------------------------------------------
// local variables

// flag for the main-loop to close the session, set in button interrupt
volatile bool stopSessionFlag = false;
// flag for the main-loop to start the session, set in button interrupt
volatile bool startSessionFlag = false;
// flag for the main loop to switch in Serial Command Mode
volatile bool serialCmdModeFlag = false;

// Ring Buffer variables
uint8_t maxRingSize = 0;
block_t freshBlock;
CircularBuffer<block_t, 20> ring;

// counter for number of data_sets (struct datastore) in "block"
// used to keep track when "block" is full and push to ring has to be initiated
uint16_t bufferCounter = 0;
// counter for number of written data_t's to SD
// used to verify the amount of written data in function checkSum()
uint32_t writeCounter = 0;

uint32_t startTimeMillis = 0;
uint32_t endTimeMillis = 0;
uint32_t startTimeMicros = 0;
uint32_t endTimeMicros = 0;
uint32_t startTimeRtcSecs = 0;
uint32_t endTimeRtcSecs = 0;

// maximum elapsed time for SD write of one buffer
uint32_t sdMaxWriteTime = 0;
// medium elapsed time for SD write of one buffer
uint32_t sdMedianWriteTime = 0;

// initialize a meta object
meta_t meta;

// Button Debouncing
#ifdef SOFTWARE_BUTTON_DEBOUNCE
uint32_t lastExtIntTime = 0;
const uint32_t debounceDelay = 300;
#endif



// ------------------------------------------------------------------------------------
// Function implementations

/**************************************************************************************
/*	setup (Arduino legacy function)
	
	@brief	Checks if the desired sampleFrequency is not too high for the activated IMU 
			parts. 
			Calls all the necessary init functions of board-hardware, i2c, rtc, 
			imu, adc, timer. In case of error in one of these functions an error is 
			indicated by blinking red led, the program does not commence in this case.
			Reason of error can be read if connected to serial USB. Clear out the error 
			and restart application.
			If all init functions executed without error, the green LED is turned on to 
			visually indicate readiness for logging.
***************************************************************************************/
void setup() 
{  
	// Turn off LEDs at the beginning
	digitalWrite(LedIntGreen_Arduino, LOW);
	digitalWrite(LedExtGreen_Arduino, LOW);
	digitalWrite(LedIntRed_Arduino, LOW);
	digitalWrite(LedExtRed_Arduino, LOW);
	
	
	// setup serial communciation for USB
	Serial.setTimeout(SERIAL_TIMEOUT);
	Serial.begin(SERIAL_BAUD);
	
	// Wait for USB Serial
	#if WAIT_FOR_SERIAL
	
	while (!Serial)	delay(10);
	delay(10);
	Serial.println("Serial USB connected");
	
	#endif // WAIT_FOR_SERIAL
	
	if(BLOCK_SIZE != 512)
	{
		Serial.println("Invalid Blocksize != 512");
		fatalBlink();
	}
	
  
	delay(500);
	
	// Init board hardware such as LED, ADC, EIC
	bs_init();
	
	// Init I2C-Bus
	if(i2c_init() < 0)
	{
		#ifdef SERIAL_DEBUG_MESSAGES
		Serial.println("i2c_init failed");
		#endif
		fatalBlink();
	}
	
	// Init RealTimeClock
	if(rtc_init() < 0)
	{
		#ifdef SERIAL_DEBUG_MESSAGES
		Serial.println("rtc_init failed");
		#endif
		fatalBlink();
	} 
	else 
	{
		// print current date-time to Serial for observation
		rtc_printNow(&Serial);
		Serial.println();
	}
	
	// init SPI-bus
	if(spi_init() < 0)
	{
		#ifdef SERIAL_DEBUG_MESSAGES
		Serial.println("spi_init failed");
		#endif
		fatalBlink();
	}
	
	// init SD-Card
	if(sd_init() < 0)
	{
		#ifdef SERIAL_DEBUG_MESSAGES
		Serial.println("sd_init failed");
		#endif
		fatalBlink();
	}
	
	// only initialize LSM9DS1 if Magnetometer is needed
	if (sampleMag)
	{
		if(lsm_init() < 0)
		{
			#ifdef SERIAL_DEBUG_MESSAGES
			Serial.println("lsm_init failed");
			#endif
			fatalBlink();
		}
	}
	
	// only initialize BMI160 if either Accelerometer or Gyrometer is needed
	if (sampleAccel | sampleGyro)
	{
		if(bmi_init() < 0)
		{
			#ifdef SERIAL_DEBUG_MESSAGES
			Serial.println("bmi_init failed");
			#endif
			fatalBlink();
		}
		else {
			#ifdef SERIAL_DEBUG_MESSAGES
			Serial.println("bmi_init successful");
			#endif
		}
	}
	
	// Init Analog-Digital-Converter
	if(adc_init() < 0)
	{
		#ifdef SERIAL_DEBUG_MESSAGES
		Serial.println("adc_init failed");
		#endif
		fatalBlink();
	}

	// Init Timer/Counter (Interrupts)
	if(ts_init() < 0)
	{
		#ifdef SERIAL_DEBUG_MESSAGES
		Serial.println("ts_init failed");
		#endif
		fatalBlink();
	}
	
	#ifdef TEST_SPEED
	// Print out elapsed time for various measuring functions
	testSpeed();
	#endif
	
	// Enable External-Interrupts, from this moment the buttons are active
	bs_enableExtIntPins();

	delay(500);
  
	#ifdef SERIAL_DEBUG_MESSAGES
	Serial.printf("sizeof(data_t) = %u\n", DATA_SIZE);
	Serial.printf("Rec/Block(=DataDim) = %lu\n", DATA_DIM);
	Serial.printf("FillDim = %u\n", FILL_DIM);
	Serial.printf("sizeof(block_t) = %lu\n", BLOCK_SIZE);
	#endif
	
	// Indicate ready for start with green led
	digitalWrite(LedIntGreen_Arduino, HIGH);
	digitalWrite(LedExtGreen_Arduino, HIGH);
}


/**************************************************************************************
	loop (Arduino legacy function)
	
	@brief	State-machine like infinity loop. Nested in the main-function.
			The ring-buffer is checked, if there are entries (!ring.isEmpty) 
			the oldest entry is pulled out and written to SD.
			StopSessionFlag initiates stopping of logging procedure if logging is active
			and closing of SD-File.
			StartSessionFlag initiates starting of logging procedure and opening of new
			SD-File.
***************************************************************************************/
void loop() 
{	
	// if there are any block_t's in the ring..
	if(!ring.isEmpty())
	{
		writeCounter++;
		
		// keep track of the maximal size of the ring-buffer
		// for observation and optimization purposes
		if (ring.size() > maxRingSize)
		{
			maxRingSize = ring.size();
		}
		
		// keep track of starting time of SD-write
		uint32_t sdTempWriteTime = micros();
		
		// remove the oldest entry (the head) from ring and ...
		block_t toWrite = ring.shift();
		
		// ... write it to SD-Card
		if(!sd_write(&toWrite))
		{
			// File full or other error
			Serial.println("sd write error");
			// stop logging in this case
			stopSessionFlag = true;
		}
		
		// Measure elapsed time for a sd block write
		sdTempWriteTime = micros() - sdTempWriteTime;
		if (sdTempWriteTime > sdMaxWriteTime)
		{
			sdMaxWriteTime = sdTempWriteTime;
		}
		
		//sdMedianWriteTime = (sdMedianWriteTime * (writeCounter - 1) + sdTempWriteTime) / writeCounter;
	}
	
	// Button -> closeSessionFlag -> stop Timer immediatley
	// But only if it is running
	if (stopSessionFlag && ts_getTimerRunning())
	{	
		// stop the timer interrupts, no new records will be produced
		ts_stopTimer();
		
		// save end times from fine to coarse
		endTimeMicros = micros();
		endTimeMillis = millis();
		endTimeRtcSecs = rtc_getSeconds();
		
		// Calculate durations
		//durationMicros = endTimeMicros - startTimeMicros;
		meta.durationMillis = endTimeMillis - startTimeMillis;
		meta.durationRtcSecs = endTimeRtcSecs - startTimeRtcSecs;
		
		// Put last freshBlock (not completely full) in ring
		if(!ring.push(freshBlock))
		{
			#ifdef SERIAL_DEBUG_MESSAGES
			Serial.println("Overrun pushing last block in ring");
			#endif
		}
	}
	
	// Only close session if the Flag is true (-> Button) AND all Data in Ring is written
	if(stopSessionFlag && ring.isEmpty())
	{
		stopSessionFlag = false;
		closeSession();
	}

	// start a new session if button was pressed
	if(startSessionFlag) 
	{
		startSessionFlag = false;
		startSession();
	}
	
	if(serialCmdModeFlag)
	{
		//Serial.println("Entered Serial Command Mode, press i, t, d");
		
		// loop to trap the program flow in here
		while (serialCmdModeFlag)
		{
			char cmd;
			
			if (Serial.available())
			{
				bs_turnOffLedExtGreen();
				bs_turnOffLedIntGreen();
				
				cmd = Serial.read();
				
				switch (cmd)
				{
				case 't':	// transfer
					sd_usbTransferAll();
					break;
					
				case 'i':	// info
					sd_infoFilesOnCard();
					break;
					
				case 'd':	// delete
					sd_deleteAllFiles();
					break;
					
				case 'r':	// set RTC
					rtc_set();
					break;
				
				default:
					break;
				}
				
			}
			
			// delay for stability
			delay(200);
			
			// blink green LED for visual indication
			bs_toggleLedIntGreen();
			bs_toggleLedExtGreen();
		}
		
		bs_turnOnLedIntGreen();
		bs_turnOnLedExtGreen();
	}
	
	// stop at predefined log time
	// only integrated if the corresponding symbol in configuration.h is defined
	#ifdef PREDEFINED_LOGTIME
	if((ts_getTimerRunning()) && ((millis() - startTimeMillis) > PREDEFINED_LOGTIME))
	{
		stopSessionFlag = true;
	}
	#endif
}



/**************************************************************************************
/*	TC3_Handler (Timer Interrupt Routine)
	
	@brief	Fills up a "block" with new values of ADC and IMU,
			saves time stamps if activated in configuration.h.
			If a block is full it is pushed in the ring-buffer
***************************************************************************************/
void TC3_Handler() 
{
	// Check if this interrupt is due to the compare register matching the timer count
	// Disabled for speed reasons
	// If more than 1 Compare Value for Timer 3 is used reactivate it
  
	//if (TC3->COUNT16.INTFLAG.bit.MC0 == 1)
	//{
		// -----------------------------------------------------------------
		// User Callback code
		
		// For measuring of TC_Frequency with oscilloscope
		#if TC_FREQUENCY_ON_PIN
		bs_toggleLedExtIo4();
		//bs_toggleLedExtRed();
		//PORT->Group[LedExtRed_Port].OUTTGL.reg = LedExtRed_Mask;
		#endif
	
		// pointer to latest (freshest) element in ring
		//block_t* ptr_firstRing = ring[ring.size()-1];

		// if a block_t is full with new records...
		if(bufferCounter == DATA_DIM)
		//if(freshBlock.count == DATA_DIM)
		{
			// ...push the freshly filled up block in the Ring-Buffer
			if(!ring.push(freshBlock))
			{
				// oldest entry in Ring was overwritten -> Overrun
				#ifdef SERIAL_DEBUG_MESSAGES
				Serial.println("Overrun pushing block in ring in TC_Handler");
				#endif
			}
		
			// reset bufferCounter
			bufferCounter = 0;
			//freshBlock.count = 0;
		
		}
	
		// read fresh data record (row) from various sensors
	
		#if WRITE_TIME_TO_SD
		uint32_t stampTimeMillis = millis()-startTimeMillis;
		//uint32_t stampTimeMicros = micros()-startTimeMicros;
		#endif
		
		// Battery Current
		uint16_t cur = adc_getValue(BatC_Arduino) * adcSensitivity;
		// Battery Voltage
		uint16_t volt = adc_getValue(BatV_Arduino) * adcSensitivity;
		// Sound (Microphone)
		uint16_t mic = adc_getValue(Mic_Arduino) * adcSensitivity;
	
		// only accelerometer of BMI160
		#if (SAMPLE_ACCEL & (! SAMPLE_GYRO))
		//#ifdef SAMPLE_ACCEL
		int16_t ax, ay, az;
		//lsm_getAccelValues(&ax, &ay, &az);
		bmi_getAccel(&ax, &ay, &az);
		#endif
		
		// only gyrometer of BMI160
		#if ((! SAMPLE_ACCEL) & SAMPLE_GYRO)
		//#ifdef SAMPLE_GYRO
		int16_t gx, gy, gz;
		//imu_getGyroValues(&gx, &gy, &gz);
		bmi_getGyro(&gx, &gy, &gz);
		#endif
		
		// Both accelerometer and gyrometer of BMI160
		#if (SAMPLE_ACCEL & SAMPLE_GYRO)
		int16_t ax, ay, az, gx, gy, gz;
		bmi_getAccelGyro(&ax, &ay, &az, &gx, &gy, &gz);
		#endif
		
		// Magnetometer of LSM9DS1
		#if SAMPLE_MAG
		int16_t mx, my, mz;
		lsm_getMagValues(&mx, &my, &mz);
		//mx = my = mz = bufferCounter;
		//mz = bufferCounter;
		#endif
		
		// put this data formatted as data_t struct in block_t struct
		freshBlock.data[bufferCounter] = {
			#if WRITE_TIME_TO_SD 
			stampTimeMillis, 
			#endif
			volt, cur, mic 
			#if SAMPLE_ACCEL 
			,ax, ay, az 
			#endif
			#if SAMPLE_GYRO 
			,gx, gy, gz 
			#endif
			#if SAMPLE_MAG 
			,mx, my, mz 
			#endif 
		};
		//freshBlock.data[freshBlock.count] = {...};
	
		// increment bufferCounter
		bufferCounter++;
		//freshBlock.count++;
	
	//} //endif (TC->INTFLAG.bit.MC0 == 1)

	// Acknowledge the interrupt (clear MC0 interrupt flag to re-arm)
	TC3->COUNT16.INTFLAG.reg |= TC_INTFLAG_MC0; // Match Compare Register 0
  
} // TC3_Handler()


/**************************************************************************************
/*  startSession
	@brief	A session is sampling of data.
			Sets the LEDs to indicate start of session.
			Resets counters.
			Calls the sd_openLogfile function.
			Saves start times.
			Turns on red LED to indicate sampling has started.
			Calls the timer start function.
**************************************************************************************/
void startSession(void) 
{
	digitalWrite(LedIntGreen_Arduino, LOW);
	digitalWrite(LedExtGreen_Arduino, LOW);
	
	// reset counters
	writeCounter = 0;
	bufferCounter = 0;
	//freshBlock.count = 0;
	sdMaxWriteTime = 0;
	sdMedianWriteTime = 0;
	
	// write the date-time to the meta struct
	rtc_getNowChar(meta.rtcStamp);	// array name without [] is a pointer to first element
	
	// Reset Ring Buffer
	ring.clear();
	
	// initialize SD-Card  
	if(sd_openLogfile() < 0)
	{
		Serial.println("Error in sdInitialize");
		fatalBlink();
		return;
	}
	
	// Indicate with LEDs that timer is started
	digitalWrite(LedIntRed_Arduino, HIGH);
	digitalWrite(LedExtRed_Arduino, HIGH);
	
	// save start times of session
	startTimeRtcSecs = rtc_getSeconds();
	startTimeMillis = millis();
	startTimeMicros = micros();
	
	// Start the timer and the interrupts
	// Measuring will commence immediately
	ts_startTimer();
	
	// Do not call anything time consuming here, because the sampling has started

}



/**************************************************************************************
/*  stopSession
	@brief	A session is sampling of data.
			Calls the ts_stopTimer function to stop the Timer-Interrupts.
			Saves endtimes of the session and calculates the duration.
			Turns off red led indicating timer stopped.
			Calls checkSum function.
			Closes SdCard file.
			Turns on green LED to indicate ready for new session.
**************************************************************************************/
void closeSession(void) 
{
	// Timer already disabled in main loop, here only for completeness
	if (ts_getTimerRunning())
	{
		ts_stopTimer();
		
		endTimeMicros = micros();
		endTimeMillis = millis();
		endTimeRtcSecs = rtc_getSeconds();
		
		// Calculate durations
		//meta.durationMicros = endTimeMicros - startTimeMicros;
		meta.durationMillis = endTimeMillis - startTimeMillis;
		meta.durationRtcSecs = endTimeRtcSecs - startTimeRtcSecs;
		
	}
	
	digitalWrite(LedIntRed_Arduino, LOW);
	digitalWrite(LedExtRed_Arduino, LOW);
	
	// Call function to check if actually written datasets matches expected datasets
	checkSum();
	
	// close the logfile
	sd_closeLogFile();
	
	// write the meta file with duration, sampleFrequency, etc
	sd_writeMetaFile(&meta);
	
	#ifdef SERIAL_DEBUG_MESSAGES
	Serial.println("File closed");
	Serial.println();
	#endif
	
	// Indicate through green LED readiness for new session
	digitalWrite(LedIntGreen_Arduino, HIGH);
	digitalWrite(LedExtGreen_Arduino, HIGH);
}



/**************************************************************************************
/*  toggleSession
	
	@brief	Calls ts_getTimerRunning to get status of Timer-Interrupts
			If timer is running, the closeSessionFlag is set and vice versa.
			These flags are processed in the main loop
**************************************************************************************/
void toggleSession(void) 
{
	if(ts_getTimerRunning()) 
	{
		startSessionFlag = false;
		stopSessionFlag = true;	
	} else {
		stopSessionFlag = false;
		startSessionFlag = true;	
	}
}


/**************************************************************************************
/*  toggleSerialCmdMode
	
	@brief	toggles the serialCmdModeFlag
**************************************************************************************/
void toggleSerialCmdMode(void) 
{
	if(serialCmdModeFlag == true) 
	{
		serialCmdModeFlag = false;	
	} else {
		serialCmdModeFlag = true;
	}
}



/**************************************************************************************
/*  checkSum
	
	@brief	Checks if the amount of expected datasets matches the amount of actually- 
			written-data.
			The actually-written-datasets (records) value is calculated with the 
			writeCounter. 
			The expected amount of datasets is calculated with duration in rtc seconds
			or SysTick milliseconds and sample frequency.
			Dataset or record is one entry of the struct data_t array which equals a 
			row in the csv file.
			Deviation between expected and actually written data is calculated for each
			the expected millis and RTC values.
			Deviation can occur due to lags of the SD-Write if the uC is overbusy 
			because of a too high SampleFrequency
			DeviationRtc is often only due to coarse resolution of the rtc seconds. 
			Therefore a tolerance is applied. If deviation is more than the tolerance, 
			the most common reason is that the sampling was too slow due to too high 
			sampleFreq.
	
			@return int16_t deviation of expectedRtc and actually written data
					positive value: more datasets written than expected
					negative value: less datasets written than expected
**************************************************************************************/
int16_t checkSum(void) 
{	
	Serial.println();
	Serial.printf("writeCounter: %lu\n", writeCounter);
	//Serial.printf("LastBlockCount: %lu\n", freshBlock.count);
	Serial.printf("LastBlockCount: %lu\n", bufferCounter);
	Serial.printf("sampleFreq: %lu\n", meta.sampleFrequency);
	Serial.printf("logTimeRtcSecs: %lu\n", meta.durationRtcSecs);
	Serial.printf("logTimeMillis: %lu\n", meta.durationMillis);
	Serial.printf("maxSdWriteTimeMicros: %lu\n", sdMaxWriteTime);
	//Serial.printf("medianSdWriteTimeMicros: %lu\n", sdMedianWriteTime);
	Serial.printf("maxRingSize: %lu\n", maxRingSize);
	
	// amount of records (rows) that were written
	// bufferCounter has the amount of Values from the latest block (not yet full)
	meta.recordsWritten = (writeCounter - 1) * DATA_DIM + bufferCounter;
	//meta.recordsWritten = (writeCounter - 1) * DATA_DIM + freshBlock.count;
	
	// amount of records (rows) that are expected
	// durationMillis is in milliseconds, but seconds are needed -> /1000
	meta.recordsExpectedRtc = meta.sampleFrequency * meta.durationRtcSecs;
	meta.recordsExpectedMillis = (meta.sampleFrequency * meta.durationMillis)/1000;
	
	Serial.println();
	Serial.println("recordsWritten = (writeCounter-1)*recorsPerBlock + lastBlockCount: ");
	Serial.println(meta.recordsWritten);
	Serial.println("recExpItc = meta.sampleFreq * meta.durationRtcMillis / 1000: ");
	Serial.println(meta.recordsExpectedMillis);
	Serial.println("recExpdRtc = sampleFreq * logTimeRtcSecs: ");
	Serial.println(meta.recordsExpectedRtc);
	
	// calculate deviation and save it in meta
	meta.deviationMillis = meta.recordsWritten - meta.recordsExpectedMillis;
	Serial.println("Deviation (RecWritten - RecExpItcMillis): ");
	Serial.println(meta.deviationMillis);
	
	meta.deviationRtc = meta.recordsWritten - meta.recordsExpectedRtc;
	Serial.println("Deviation (RecWritten - RecExpRtc): ");
	Serial.println(meta.deviationRtc);
	
	// tolerance for deviationRtc due to coarse resolution of seconds
	Serial.println("Tolerance (0.9 * sampleFreq): ");
	meta.tolerance = 0.9 * meta.sampleFrequency;
	Serial.println(meta.tolerance);
	
	if (abs(meta.deviationRtc) <= meta.tolerance)
	{
		Serial.println("Checksums match within tolerance");
	}
	else
	{
		Serial.println("Checksums do NOT match");
	}
	
	return meta.deviationRtc;
}


/**************************************************************************************
/*	EIC_Handler
	@brief	Button Interrupt Handler.
			Checks which button caused the interrupt and calls the respective
			user function.
**************************************************************************************/
void EIC_Handler(void)
{
	/*Serial.print("EXTINT2 = ");
	Serial.println(EIC->INTFLAG.bit.EXTINT2);
	Serial.print("EXTINT3 = ");
	Serial.println(EIC->INTFLAG.bit.EXTINT3);*/
		
	// If more than 1 Button Interrupt is needed
	// check which button triggered the Interrupt with INTFLAG:
	
	// button right: PB02 -> EXTINT2
	if (EIC->INTFLAG.bit.EXTINT2)
	{
		#ifdef SOFTWARE_BUTTON_DEBOUNCE
			// Don't enter if called too frequently
			if ((millis() - lastExtIntTime) > debounceDelay)
			{
				// Save last time the Interrupt was executed for debouncing
				lastExtIntTime = millis();
			
		#endif
		
		// --------------------------------------------------------------		
		// for button right, add code here
		
		#ifdef SERIAL_DEBUG_MESSAGES
		Serial.println("Button right pressed");
		#endif
		
		// if in serialCmdMode, do nothing
		if (!serialCmdModeFlag)
		{
			toggleSession();
		}
			
		// -------------------------------------------------------------
			
		#ifdef SOFTWARE_BUTTON_DEBOUNCE	
			}
		#endif
		
		// clear Interrupt Flag, has to stay last command
		EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT2;
	} 
	
	// button left: PB03 -> EXTINT3
	else if (EIC->INTFLAG.bit.EXTINT3)
	{
		#ifdef SOFTWARE_BUTTON_DEBOUNCE
			// Don't enter if called too frequently
			if ((millis() - lastExtIntTime) > debounceDelay)
			{
				// Save last time the Interrupt was executed for debouncing
				lastExtIntTime = millis();
			
		#endif
		
		// -------------------------------------------------------------
		// for button left, add code here
		
		#ifdef SERIAL_DEBUG_MESSAGES
		Serial.println("Button left pressed");
		#endif
		
		// if logging runs, do nothing
		if (!ts_getTimerRunning())
		{
			// wait for Serial to connect
			//while (!Serial)	delay(500);
			delay(200);
			
			toggleSerialCmdMode();
		}
			
		// -------------------------------------------------------------
			
		#ifdef SOFTWARE_BUTTON_DEBOUNCE
			}
		#endif
		
		// clear Interrupt Flag, has to stay last command
		EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT3;
	}
	
}


/**************************************************************************************
/*	fatalBlink	
	
	@brief	Blinks the red LED to indicate Error.
			This Method will never return,
**************************************************************************************/
void fatalBlink(void)
{
	//Serial.println("Entered fatal blink");
	while (true)
	{
		bs_togglePin(LedIntRed_Arduino);
		bs_togglePin(LedExtRed_Arduino);
		delay(200);
	}
}

/**************************************************************************************
/*	testSpeed
	
	@brief	Prints out amount of time elapsed for calling of various measuring 
			functions.
**************************************************************************************/
void testSpeed(void)
{
	uint32_t timeSaver;
	uint16_t testVal;
	
	Serial.println();
	
	// Mathematics Speed
	
	Serial.println("Mathematics Speed:");
	timeSaver = micros();
	testVal = (uint16_t) 200*100/16;
	timeSaver = micros() - timeSaver;
	Serial.printf("200*100/16 = %lu in %lu us\n",testVal, timeSaver);
	
	// Test ADC Speed
	
	Serial.println();
	Serial.println("Last adc_getValue(BatV_Arduino) Value:");
	timeSaver = micros();
	testVal = adc_getValue(BatV_Arduino)*adcSensitivity;
	timeSaver = micros() - timeSaver;
	Serial.printf("%lu mV\t in %lu us\n",testVal, timeSaver);
	
	Serial.println("Last adc_getValue(BatC_Arduino) Value:");
	timeSaver = micros();
	testVal = adc_getValue(BatC_Arduino)*adcSensitivity;
	timeSaver = micros() - timeSaver;
	Serial.printf("%lu mV\t in %lu us\n",testVal, timeSaver);
	
	Serial.println("Last adc_getValue(Mic_Arduino) Value:");
	timeSaver = micros();
	testVal = adc_getValue(Mic_Arduino)*adcSensitivity;
	timeSaver = micros() - timeSaver;
	Serial.printf("%lu mV\t in %lu us\n",testVal, timeSaver);
	
	
	// Test RTC Speed
	
	timeSaver = micros();
	uint32_t testRtc = rtc_getSeconds();
	timeSaver = micros() - timeSaver;
	Serial.printf("\nRTCgetValue %lu sec in %u us\n", testRtc, timeSaver);
	
	// Test IMU Speed
	
	#if (SAMPLE_ACCEL == 1)
	// LSM9DS1
	int16_t x, y, z;
	timeSaver = micros();
	lsm_getAccelValues(&x, &y, &z);
	timeSaver = micros() - timeSaver;
	Serial.printf("\nlsmGetAccel\t X:%li\t Y:%li\t Z:%li\t in %u us\n", x, y, z, timeSaver);
	// BMI160
	timeSaver = micros();
	bmi_getAccel(&x, &y, &z);
	timeSaver = micros() - timeSaver;
	Serial.printf("bmiGetAccel\t X:%li\t Y:%li\t Z:%li\t in %u us\n", x, y, z, timeSaver);
	#endif
	
	#if (SAMPLE_GYRO == 1)
	// LSM9DS1
	int16_t roll, pitch, yaw;
	timeSaver = micros();
	lsm_getGyroValues(&roll, &pitch, &yaw);
	timeSaver = micros() - timeSaver;
	Serial.printf("imuGetGyro\t X:%li\t Y:%li\t Z:%li\t in %u us\n", roll, pitch, yaw, timeSaver);
	// BMI160
	timeSaver = micros();
	bmi_getGyro(&roll, &pitch, &yaw);
	timeSaver = micros() - timeSaver;
	Serial.printf("bmiGetGyro\t X:%li\t Y:%li\t Z:%li\t in %u us\n", roll, pitch, yaw, timeSaver);
	#endif
	
	#if (SAMPLE_MAG == 1)
	int16_t magX, magY, magZ;
	timeSaver = micros();
	lsm_getMagValues(&magX, &magY, &magZ);
	timeSaver = micros() - timeSaver;
	Serial.printf("imuGetMag\t X:%li\t Y:%li\t Z:%li\t in %u us\n", magX, magY, magZ, timeSaver);
	#endif
	
	Serial.println();
}


bool checkIntArray(uint8_t* array, uint8_t length, uint8_t toCheck)
{
	for (int i = 0; i < length; i++)
	{
		if (*(array+i) == toCheck) return true;
	}
	
	return false;
}