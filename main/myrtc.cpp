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

uint8_t started = 0;
uint8_t stopLogging = 0;

int now[6];
int startTime[6];
int endTime[6];
int resetTime[6];

void initRTC(char* time){
    convertDates(time, now);
    rtc.begin();
    rtc.setTime(now[3], now[4], now[5]);
    rtc.setDate(now[2], now[1], now[0]);
}

int getStopLogging(){
    return stopLogging;
}
int getStartLogging(){
    return started;
}


void setLoggingStart(char* time){
    convertDates(time, startTime);
}

void setLoggingEnd(char* time){
    convertDates(time, endTime);
}

void setNextReset(char* time){
    convertDates(time, resetTime);
}

void alarmMatch(){
    rtc.detachInterrupt();
    Serial.println("alarm");
    if(started){
        stopLogging = 1;        
    } else {
        started = 1;
        mySetAlarm(endTime);
    } 
}

void convertDates(char* datestring, int out[6]){
    out[0] = ((int)datestring[0]-48)*1000 + ((int)datestring[1]-48)*100 + ((int)datestring[2]-48)*10 + ((int)datestring[3]-48);
    out[1] = ((int)datestring[4]-48)*10 + ((int)datestring[5]-48);
    out[2] = ((int)datestring[6]-48)*10 + ((int)datestring[7]-48);
    out[3] = ((int)datestring[8]-48)*10 + ((int)datestring[9]-48);
    out[4] = ((int)datestring[10]-48)*10 + ((int)datestring[11]-48);
    out[5] = ((int)datestring[12]-48)*10 + ((int)datestring[13]-48);
}

void RTCStartAlarms(){
    mySetAlarm(startTime);
}

void mySetAlarm(int alarm[6]){
    rtc.setAlarmHours(alarm[3]);
    rtc.setAlarmMinutes(alarm[4]);
    rtc.setAlarmSeconds(alarm[5]);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.attachInterrupt(alarmMatch);
}

void waitForReset(){
    mySetAlarm(resetTime);
    rtc.standbyMode();
    NVIC_SystemReset();
}