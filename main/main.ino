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

#define DELAY 2500


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
        delay(DELAY);

        delay(DELAY);
        WiFi.end();
        myRTC.waitForReset();
        }
    }
}

/**
 * @brief starts communication with matlab and sets correct times in RTC
 * 
 */
void initMatlabCommunication(){
    
    char datenow[14];
    char dateStart[14];
    char dateEnd[14];
    char dateReset[14];

    myWifi.flush();
    char message[] = "start of transmission";
    myWifi.write(message);
    delay(DELAY);

    //just for again flushing the input should read "flush"
    // char discard[15];
    // myWifi.read(discard, 15);
    myWifi.read(datenow, 14);
    myRTC.init(datenow);
    myWifi.read(dateStart, 14);
    myRTC.setLoggingStart(dateStart);
    myWifi.read(dateEnd, 14);    
    myRTC.setLoggingEnd(dateEnd);
    myWifi.read(dateReset, 14);
    myRTC.setNextReset(dateReset);
    delay(DELAY);
    myWifi.client.flush();
    myWifi.server.flush();
    myWifi.write(datenow);
    myWifi.write(dateStart);
    myWifi.write(dateEnd);
    myWifi.write(dateReset);
    delay(DELAY);
}
