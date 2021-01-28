/******************************************************************************************
/*	TimerSetup.cpp
	Created: 30.08.2019
	Author: Julian Peters
	Last Modified: 18.12.2019
	By: Julian Peters
 
	This file contains the code to setup, initialize and start the timer.
******************************************************************************************/

// ------------------------------------------------------------------------------------
// Includes

#include "Arduino.h"
#include "TimerSetup.h"
//#include "Configuration.h"


// ------------------------------------------------------------------------------------
// Local variables

volatile bool timerRunning = false;


// ------------------------------------------------------------------------------------
// function implementations, description in prototype section above or in header-file

int ts_init(void) 
{
	// System Clock should be set before
	
	/*	Clock Initialization (CPU, AHB, APBx)
		The System RC Oscillator (OSC8M) provides the source for the main clock
		at chip startup. It is set to 1 MHz. Clock Generator 0 is enabled. 
		After initClocks Clock Generator 0 is at 48 MHz from DFLL48M
		and Clock Generator 3 is at 1 MHz from OSC8M
		*/
	
	/* Disable all IRQs until all application initialization completed */
	__disable_irq();
	
	// Disable TC3
	TC3->COUNT16.CTRLA.bit.ENABLE = 0;
	
	/* TC3 Initialization (Generate Compare Interrupts every x mS) */
	
	// Configure asynchronous clock source
	
	GCLK->CLKCTRL.bit.WRTLOCK = 0;							// Ensure the register is not locked
	GCLK->CLKCTRL.bit.GEN = GCLK_CLKCTRL_GEN_GCLK0_Val;		// select source GCLK_GEN[0] i.e. Clock Generator 0
	GCLK->CLKCTRL.bit.ID = GCLK_CLKCTRL_ID_TCC2_TC3_Val;	// select TC3 peripheral channel
	GCLK->CLKCTRL.bit.CLKEN = 1;							// enable generic clock
	/*
	// Alternative writing method to .bit
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TCC2_TC3_Val;		// select TC3 peripheral channel
	GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_GEN_GCLK0;			// select source GCLK_GEN[3] i.e. Clock Generator 3
	GCLK->CLKCTRL.bit.CLKEN = 1;							// enable generic clock
	*/
	
	
	// Configure synchronous bus clock
	// TODO check if TC is running with the right frequency 1 MHz 
	// not the 48 MHz from the Bus (SetupClock)
	PM->APBCSEL.bit.APBCDIV = 0;							// no prescaler
	PM->APBCMASK.bit.TC3_ = 1;								// enable TC3 interface
	
	// Configure Count Mode (16-bit)
	TC3->COUNT16.CTRLA.bit.MODE = 0x0;
	
	// Configure Prescaler for divide (Beware Input Clock from before)
	//TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :1
	//TC3->COUNT16.CTRLA.bit.PRESCALER = 0x1;		// :2
	//TC3->COUNT16.CTRLA.bit.PRESCALER = 0x2;		// :4
	//TC3->COUNT16.CTRLA.bit.PRESCALER = 0x3;		// :8
	//TC3->COUNT16.CTRLA.bit.PRESCALER = 0x4;		// :16	
	//TC3->COUNT16.CTRLA.bit.PRESCALER = 0x5;		// :64
	//TC3->COUNT16.CTRLA.bit.PRESCALER = 0x6;		// :256	
	//TC3->COUNT16.CTRLA.bit.PRESCALER = 0x7;		// :1024 
	//TC3->COUNT16.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV64_Val
	
	// Initialize prescaler and compare value
	// Max Value for count 16: 2^16-1 = 65535
	#define SAMPLE_FREQ 12000
	#if (SAMPLE_FREQ == 50)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x4;		// :16
	TC3->COUNT16.CC[0].reg = 60000;				// 50 Hz for 48MHz SRC and 16 PRSCL
	
	#elif (SAMPLE_FREQ == 100)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x3;		// :8
	TC3->COUNT16.CC[0].reg = 60012;				// 100 Hz for 48MHz SRC and 8 PRSCL
	
	#elif (SAMPLE_FREQ == 200)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x2;		// :4
	TC3->COUNT16.CC[0].reg = 60008;				// 200 Hz for 48MHz SRC and 4 PRSCL
	
	#elif (SAMPLE_FREQ == 250)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x3;		// :8
	TC3->COUNT16.CC[0].reg = 24002;				// 250 Hz for 48MHz SRC and 8 PRSCL
	
	#elif (SAMPLE_FREQ == 500)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x1;		// :2
	TC3->COUNT16.CC[0].reg = 48010;				// 500 Hz for 48MHz SRC and 2 PRSCL
	
	#elif (SAMPLE_FREQ == 600)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x1;		// :2
	TC3->COUNT16.CC[0].reg = 40008;				// 600 Hz for 48MHz SRC and 2 PRSCL
	
	#elif (SAMPLE_FREQ == 750)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x1;		// :2
	TC3->COUNT16.CC[0].reg = 32002;				// 750 Hz for 48MHz SRC and 2 PRSCL
	
	#elif (SAMPLE_FREQ == 800)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :2
	TC3->COUNT16.CC[0].reg = 60002;				// 800 Hz for 48MHz SRC and 2 PRSCL
		
	#elif (SAMPLE_FREQ == 1000)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :1
	TC3->COUNT16.CC[0].reg = 48002;				// 1kHz for 48MHz SRC and 1 PRSCL
	
	#elif (SAMPLE_FREQ == 2000)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :1
	TC3->COUNT16.CC[0].reg = 24003;				// 2 kHz for 48MHz SRC and 1 PRSCL
	
	#elif (SAMPLE_FREQ == 3000)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :1
	TC3->COUNT16.CC[0].reg = 16000;				// 3 kHz for 48MHz SRC and 1 PRSCL
	
	#elif (SAMPLE_FREQ == 4000)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :1
	TC3->COUNT16.CC[0].reg = 12000;				// 4 kHz for 48MHz SRC and 1 PRSCL
	
	#elif (SAMPLE_FREQ == 6000)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :1
	TC3->COUNT16.CC[0].reg = 8000;				// 6 kHz for 48MHz SRC and 1 PRSCL
	
	#elif (SAMPLE_FREQ == 8000)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :1
	TC3->COUNT16.CC[0].reg = 6000;				// 8 kHz for 48MHz SRC and 1 PRSCL
	
	#elif (SAMPLE_FREQ == 12000)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :1
	TC3->COUNT16.CC[0].reg = 3999;				// 12 kHz for 48MHz SRC and 1 PRSCL
	
	#elif (SAMPLE_FREQ == 16000)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :1
	TC3->COUNT16.CC[0].reg = 2997;				// 16 kHz for 48MHz SRC and 1 PRSCL
	
	#elif (SAMPLE_FREQ == 20000)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :1
	TC3->COUNT16.CC[0].reg = 2396;				// 20 kHz for 48MHz SRC and 1 PRSCL
	
	#elif (SAMPLE_FREQ == 24000)
	TC3->COUNT16.CTRLA.bit.PRESCALER = 0x0;		// :1
	TC3->COUNT16.CC[0].reg = 1997;				// 24 kHz for 48MHz SRC and 1 PRSCL
	
	#else
	#error "SampleFreq not valid. Choose from given values!"
	#endif
	
	
	// Prescaler Synchronization: Reload or Reset the counter on next prescaler clock
	TC3->COUNT16.CTRLA.bit.PRESCSYNC = TC_CTRLA_PRESCSYNC_PRESC_Val;
	//TC3->COUNT16.CTRLA.bit.PRESCSYNC = TC_CTRLA_PRESCSYNC_GCLK_Val;
	
	// Configure TC3 Compare Mode for compare channel 0
	TC3->COUNT16.CTRLA.bit.WAVEGEN = 0x1;					// "Match Frequency" operation	
	
	// Enable TC3 compare mode interrupt generation
	TC3->COUNT16.INTENSET.bit.MC0 = 0x1;					// Enable match interrupts on compare channel 0
	
	// Enable TC3
	TC3->COUNT16.CTRLA.bit.ENABLE = 1;
	
	// Wait until TC3 is enabled
	while(TC3->COUNT16.STATUS.bit.SYNCBUSY == 1);
	
	// Enable all IRQs, disabled before
	__enable_irq();
	
	return 0;
}

/************************************************************************
 * enableTimerInterrupts starts the TC3 TimerCounter
 * Preconditions:	initTimerInterrupts
 ************************************************************************/
void ts_startTimer(void)
{
	/* Configure/Enable Interrupts */
	NVIC_SetPriority(TC3_IRQn, 1);						// Set the interrupt priority 1: High 3: Low
	NVIC_EnableIRQ(TC3_IRQn);							// Enable the interrupt
	
	timerRunning = true;
	
	#ifdef SERIAL_DEBUG_MESSAGES
	Serial.println("Timer started ");
	#endif
}

/************************************************************************
 ts_stopTimer
 disableTimerInterrupts stops the TC3 TimerCounter
 Preconditions:	initTimerInterrupts
 ************************************************************************/
void ts_stopTimer(void)
{
	NVIC_DisableIRQ(TC3_IRQn);
	
	timerRunning = false;
	
	#ifdef SERIAL_DEBUG_MESSAGES
	Serial.println("Timer stopped ");
	#endif
}


bool ts_getTimerRunning(void)
{
	if (timerRunning)
	{
		return true;
	} 
	else
	{
		return false;
	}
}



/*
static void ts_setTimerFrequency_bk(int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TcCount16* TC = (TcCount16*) TC3;
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  //Serial.println(TC->COUNT.reg);
  //Serial.println(TC->CC[0].reg);
  while (TC->STATUS.bit.SYNCBUSY == 1);
}
void ts_timerInitialize_bk(int frequencyHz) {
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3) ;
  while ( GCLK->STATUS.bit.SYNCBUSY == 1 ); // wait for sync
  TcCount16* TC = (TcCount16*) TC3;
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
  // Use the 16-bit timer
  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
  // Use match mode so that the timer counter resets when the count matches the compare register
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
  // Set prescaler to 1024
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
  ts_setTimerFrequency(frequencyHz);
  Serial.print("Set timer frequency: ");
  Serial.print(frequencyHz);
  Serial.print("\n");
  // Enable the compare interrupt
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;
  
  
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
  
}
void ts_startTimer_bk(void) {
  
  //TcCount16* TC = (TcCount16*) TC3;
  NVIC_EnableIRQ(TC3_IRQn);
  Serial.println("Timer started ");
}
void ts_stopTimer_bk(void) {
  
  
  //TcCount16* TC = (TcCount16*) TC3;
  //TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  //while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
  
  NVIC_DisableIRQ(TC3_IRQn);
  Serial.println("Timer stopped ");
}
void ts_resetTimer_bk(void) {
  
  TcCount16* TC = (TcCount16*) TC3;
  TC->CTRLA.reg = TC_CTRLA_SWRST;
  while (TC->STATUS.bit.SYNCBUSY == 1);
  while (TC->CTRLA.bit.SWRST);
}
*/

