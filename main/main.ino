#include <SPI.h>
#include "SdFat.h"
#include <RTCZero.h>
#include <WiFi101.h>
#include "BufferedPrint.h"
#include "FreeStack.h"

#define adcSensitivity 1


#include "TimerSetup.h"
#include "AdcSetup.h"
#include "tcp.h"
#include "myrtc.h"
#include "mySD.h"


MyRTC myRTC;
MyWifi myWifi;
MySD mySD;

/**
 * @brief this function is called by timer to get measurements and write them to Buffer
 * 
 */
void TC3_Handler(){
    mySD.bufferData(adc_getValue(14) * adcSensitivity);
    mySD.bufferData(adc_getValue(15) * adcSensitivity);
    if(myRTC.getStopLogging()){
        ts_stopTimer();
    }
    TC3->COUNT16.INTFLAG.reg |= TC_INTFLAG_MC0; // Match Compare Register 0
}

void setup(){
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    WiFi.setPins(8,7,4,2);
    Serial.begin(9600);
    // while(!Serial){};
    // FillStack();

    myWifi.init();
    mySD.initSD();
    ts_init(); 

    myWifi.waitForClient();

    adc_init();
}


void loop(){
    WifiMeasurement();
}

/**
 * @brief this function is called if a Wifi measurement should be startet
 * 
 */
void WifiMeasurement(){
    initMatlabCommunication();
    myRTC.startAlarms();
    mySD.createBinFile();
    while(true){
        if(myRTC.getStartLogging()){
        mySD.logData(&myRTC);
        mySD.createCsvFile();
        mySD.binaryToCsv();
        mySD.csvOverTcp(&myWifi);
        digitalWrite(LED_BUILTIN, LOW);
        myRTC.waitForReset();
        }
    }
}

/**
 * @brief starts communication with matlab and sets correct times in RTC
 * 
 */
void initMatlabCommunication(){
    char date[15];

    myWifi.flush();
    delay(100);

    char message[] = "start of transmission";
    myWifi.write(message);
    delay(100);

    //just for again flushing the input should read "flush"
    myWifi.read(date, 15);
    delay(100);

    myWifi.read(date, 15);
    myWifi.write(date);
    myRTC.init(date);
    delay(100);

    myWifi.read(date, 15);
    myWifi.write(date);
    myRTC.setLoggingStart(date);
    delay(100);

    myWifi.read(date, 15);
    myWifi.write(date);
    myRTC.setLoggingEnd(date);
    delay(100);

    myWifi.read(date, 15);
    myWifi.write(date);
    myRTC.setNextReset(date);
    delay(100);
}