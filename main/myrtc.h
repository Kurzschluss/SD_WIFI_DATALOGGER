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
void initRTC();

int getStopLogging();

void sleepFor(int minutes, int seconds);

void setLoggingTime(int minutes, int seconds);

void alarmMatch();





#endif // _MYRTC_