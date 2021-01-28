/******************************************************************************************
/*	TimerSetup.h
	Created: 30.08.2019
	Author: Julian Peters
	Last Modified: 18.12.2019
	By: Julian Peters
 
	Definitions of TimerSetup.cpp that are needed by other files are written here.
******************************************************************************************/



#ifndef _TIMERSETUP_
#define _TIMERSETUP_


// ------------------------------------------------------------------------------------
// Definitions

#define CPU_HZ 48000000


// ------------------------------------------------------------------------------------
// Function prototypes

/**************************************************************************************
/*	ts_init
	
	@brief	Sets the clock and compare values of the timer TC3 to generate timer
			interrupts. The frequency is configured according to symbols from
			configuration.h
			
	@preconditions	System Clock should be set before (F_CPU)
	
	@return always 0, no error code usable
***************************************************************************************/
int ts_init();

/**************************************************************************************
/*	ts_startTimer
	
	@brief	Starts the timer. The interrupts will be generated periodically with the
			in ts_init set frequency. Also sets priority of interrupts.
			
	@preconditions	ts_init()
***************************************************************************************/
void ts_startTimer(void);

/**************************************************************************************
/*	ts_startTimer
	
	@brief	Stops the timer. The interrupts will cease.
***************************************************************************************/
void ts_stopTimer(void);

/**************************************************************************************
/*	ts_getTimerRunning
	
	@brief	Returns the status of the timer (running: interrupts generation or stopped)
	
	@return	Status of timer: Running or Stopped
***************************************************************************************/
bool ts_getTimerRunning(void);



#endif // _TIMERSETUP_