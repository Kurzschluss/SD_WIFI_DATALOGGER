/******************************************************************************************
/*	myrtc.h
	Created: 13.01.2021
	Author: Simon Wilkes
	Last Modified: 13.01.2021
	By: Simon Wilkes
 
	Definitions of myrtc.cpp that are needed by other files are written here.
******************************************************************************************/



#ifndef _MYRTC_
#define _MYRTC_


// ------------------------------------------------------------------------------------
// Definitions


// ------------------------------------------------------------------------------------
// Function prototypes

/**************************************************************************************
/*	initRTC
	
	@brief	this is a stub
			
			
	
	@return always 0, no error code usable
***************************************************************************************/
void initRTC(char* time);

int getStopLogging();
int getStartLogging();


void setLoggingStart(char* time);

void setLoggingEnd(char* time);

void setNextReset(char* time);

void alarmMatch();

void convertDates(char* datestring, int out[6]);

void mySetAlarm(int alarm[6]);

void waitForReset();
void RTCStartAlarms();

#endif // _MYRTC_