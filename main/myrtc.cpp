/******************************************************************************************
/*	MyRTC.cpp
	Created: 13.01.2021
	Author: Simon Wilkes
	Last Modified: 13.01.2021
	By: Simon Wilkes
 
	implements used functions of rtc
******************************************************************************************/
#include "MyRTC.h"
#include "RTCZero.h"

RTCZero rtc;

// Outside of class
MyRTC *pointerToMyRTC; // declare a pointer to testLib class

static void outsideInterruptHandler(void) { // define global handler
  pointerToMyRTC->alarmMatch(); // calls class member handler
}


MyRTC::MyRTC(){
    pointerToMyRTC = this;
}

void MyRTC::init(char* time){
    convertDates(time, now);
    rtc.begin();
    rtc.setTime(now[3], now[4], now[5]);
    rtc.setDate(now[2], now[1], now[0]);
}

bool MyRTC::getStopLogging(){
    return stopLogging;
}
bool MyRTC::getStartLogging(){
    return started;
}


void MyRTC::setLoggingStart(char* time){
    convertDates(time, startTime);
}

void MyRTC::setLoggingEnd(char* time){
    convertDates(time, endTime);
}

void MyRTC::setNextReset(char* time){
    convertDates(time, resetTime);
}

void MyRTC::alarmMatch(){
    rtc.detachInterrupt();
    Serial.println("alarm");
    if(started){
        stopLogging = 1;        
    } else {
        started = 1;
        setAlarm(endTime);
    } 
}

void MyRTC::convertDates(char* datestring, int out[6]){
    out[0] = ((int)datestring[0]-48)*1000 + ((int)datestring[1]-48)*100 + ((int)datestring[2]-48)*10 + ((int)datestring[3]-48);
    out[1] = ((int)datestring[4]-48)*10 + ((int)datestring[5]-48);
    out[2] = ((int)datestring[6]-48)*10 + ((int)datestring[7]-48);
    out[3] = ((int)datestring[8]-48)*10 + ((int)datestring[9]-48);
    out[4] = ((int)datestring[10]-48)*10 + ((int)datestring[11]-48);
    out[5] = ((int)datestring[12]-48)*10 + ((int)datestring[13]-48);
}

void MyRTC::startAlarms(){
    setAlarm(startTime);
}

void MyRTC::setAlarm(int alarm[6]){
    rtc.setAlarmHours(alarm[3]);
    rtc.setAlarmMinutes(alarm[4]);
    rtc.setAlarmSeconds(alarm[5]);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.attachInterrupt(outsideInterruptHandler);
}

void MyRTC::waitForReset(){
    setAlarm(resetTime);
    rtc.standbyMode();
    NVIC_SystemReset();
}