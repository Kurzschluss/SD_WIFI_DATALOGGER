/**
 * @file myrtc.h
 * @author Simon Wilkes (simonwilkes@hotmail.de)
 * @brief 
 * @version 0.1
 * @date 2021-01-30
 * 
 * 
 */



#ifndef _MYRTC_
#define _MYRTC_

// ------------------------------------------------------------------------------------
// standart includes
#include "stdint.h"
#include "stddef.h"
// ------------------------------------------------------------------------------------
// specific includes



// ------------------------------------------------------------------------------------
// Definitions

#define seconds_default 0
#define minutes_default 0
#define hours_default 0
#define day_default 11
#define month_default 1
#define year_default 21

// ===================================================================================
// Class definition

class MyRTC{
	public:
		MyRTC();
		void init(char* time);
		bool getStopLogging();
		bool getStartLogging();
		void setLoggingStart(char* time);
		void setLoggingEnd(char* time);
		void setNextReset(char* time);
		void startAlarms();
		void alarmMatch();
		void waitForReset();

	private:
		void convertDates(char* datestring, int out[6]);
		void setAlarm(int alarm[6]);


// ------------------------------------------------------------------------------------
// variables
	private:
		uint32_t MessureBegin = 0;

		bool started = 0;
		bool stopLogging = 0;

		int now[6];
		int startTime[6];
		int endTime[6];
		int resetTime[6];

};

#endif // _MYRTC_