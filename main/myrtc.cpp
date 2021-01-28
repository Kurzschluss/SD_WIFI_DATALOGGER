/******************************************************************************************
/*	myrtc.cpp
	Created: 13.01.2021
	Author: Simon Wilkes
	Last Modified: 13.01.2021
	By: Simon Wilkes
 
	implements used functions of rtc
******************************************************************************************/
#include "myrtc.h"
#include "RTCZero.h"

RTCZero rtc;

const byte seconds_begin = 0;
const byte minutes_begin = 0;
const byte hours_begin = 0;
const byte day_begin = 11;
const byte month_begin = 1;
const byte year_begin = 21;

uint32_t MessureBegin;

uint8_t stopLogging = 0;

void initRTC(){
    rtc.begin();
    rtc.setTime(hours_begin, minutes_begin, seconds_begin);
    rtc.setDate(day_begin, month_begin, year_begin);
}

int getStopLogging(){
    return stopLogging;
}

//sleeps for x since last start of meassurement
void sleepFor(int minutes, int seconds){
    int set = minutes * 60 + seconds + MessureBegin;
    rtc.setAlarmEpoch(set);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.standbyMode();
}


void setLoggingTime(int minutes, int seconds){
    stopLogging = 0;
    MessureBegin = rtc.getEpoch();
    int set = minutes * 60 + seconds + MessureBegin;
    Serial.print("Begin: ");
    Serial.println(MessureBegin);
    Serial.print("set: ");
    Serial.println(set);
    rtc.setAlarmEpoch(set);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.attachInterrupt(alarmMatch);
}

void alarmMatch(){
    rtc.detachInterrupt();
    stopLogging = 1;
}