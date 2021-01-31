/**
 * @file myrtc.cpp
 * @author Simon Wilkes (simonwilkes@hotmail.de)
 * @brief implements used functions of myrtc.h
 * @version 1.0
 * @date 2021-01-31
 * 
 * 
 */
#include "MyRTC.h"
#include "RTCZero.h"

RTCZero rtc;


MyRTC *pointerToMyRTC; // declare a pointer to testLib class must be outside of class for beeing able to be called by interrupt.

/**
 * @brief wired hacky shit for interrupt purposes
 * 
 */
static void outsideInterruptHandler(void) { // must be outside of class for beeing able to be called by interrupt.
  pointerToMyRTC->alarmMatch(); // calls class member handler
}


MyRTC::MyRTC(){
    pointerToMyRTC = this;
}

/**
 * @brief initiates the RTC with the provided time
 * 
 * @param time char[14]
 */
void MyRTC::init(char* time){
    convertDates(time, now);
    rtc.begin();
    rtc.setTime(now[3], now[4], now[5]);
    rtc.setDate(now[2], now[1], now[0]);
}

/**
 * @brief is it time to start Logging?
 * 
 * @return true if it is time
 * @return false if it is not jet time
 */
bool MyRTC::getStartLogging(){
    return started;
}

/**
 * @brief is it time to stop Logging?
 * 
 * @return true if it is time
 * @return false if it is not jet time
 */
bool MyRTC::getStopLogging(){
    return stopLogging;
}

/**
 * @brief saves the time to start Logging
 * 
 * @param time char[14]
 */
void MyRTC::setLoggingStart(char* time){
    convertDates(time, startTime);
}

/**
 * @brief saves the time to stop Logging
 * 
 * @param time char[14]
 */
void MyRTC::setLoggingEnd(char* time){
    convertDates(time, endTime);
}

/**
 * @brief saves the time to reset the µC
 * 
 * @param time char[14]
 */
void MyRTC::setNextReset(char* time){
    convertDates(time, resetTime);
}

/**
 * @brief gets called by RTC interrupt.
 * First time it sets the Start bool.
 * Second time it sets the End bool and sets the reboot time 
 * 
 */
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

/**
 * @brief converts the provided date char array from Matlab to usable int array
 * 
 * @param datestring char[14] input
 * @param out int[4] output
 */
void MyRTC::convertDates(char* datestring, int out[6]){
    out[0] = ((int)datestring[0]-48)*1000 + ((int)datestring[1]-48)*100 + ((int)datestring[2]-48)*10 + ((int)datestring[3]-48);
    out[1] = ((int)datestring[4]-48)*10 + ((int)datestring[5]-48);
    out[2] = ((int)datestring[6]-48)*10 + ((int)datestring[7]-48);
    out[3] = ((int)datestring[8]-48)*10 + ((int)datestring[9]-48);
    out[4] = ((int)datestring[10]-48)*10 + ((int)datestring[11]-48);
    out[5] = ((int)datestring[12]-48)*10 + ((int)datestring[13]-48);
}

/**
 * @brief starts the interrupt sequence with beginning, end and reboot.
 * 
 */
void MyRTC::startAlarms(){
    setAlarm(startTime);
}

/**
 * @brief set the next alarm time
 * 
 * @param alarm int[6]
 */
void MyRTC::setAlarm(int alarm[6]){
    rtc.setAlarmHours(alarm[3]);
    rtc.setAlarmMinutes(alarm[4]);
    rtc.setAlarmSeconds(alarm[5]);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.attachInterrupt(outsideInterruptHandler);
}

/**
 * @brief waits for RTC interrupt and resets afterward. ->Never returns
 * 
 */
void MyRTC::waitForReset(){
    setAlarm(resetTime);
    rtc.standbyMode();
    NVIC_SystemReset();
}