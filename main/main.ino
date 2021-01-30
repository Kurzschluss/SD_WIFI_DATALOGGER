#include <SPI.h>
#include "SdFat.h"
#include <RTCZero.h>
#include <WiFi101.h>
#include "BufferedPrint.h"
#include "FreeStack.h"

#define adcSensitivity 1
#define ERROR_LED_PIN 0


#include "TimerSetup.h"
#include "AdcSetup.h"
#include "tcp.h"
#include "myrtc.h"
#include "mySD.h"


MyRTC myRTC;






void TC3_Handler(){
  bufferData(adc_getValue(14) * adcSensitivity);
  bufferData(adc_getValue(15) * adcSensitivity);
  if(myRTC.getStopLogging()){
    ts_stopTimer();
  }
  TC3->COUNT16.INTFLAG.reg |= TC_INTFLAG_MC0; // Match Compare Register 0
}

void adcInit() {
  adc_init();
}


void setup(){
    setmyrtc(&myRTC);
    pinMode(ERROR_LED_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    WiFi.setPins(8,7,4,2);
    Serial.begin(9600);
    // while(!Serial){};
    FillStack();

    initWifi();
    initSD();
    ts_init(); 

    waitForClient();

    adcInit();
}


void loop(){
    WifiMeasurement();
}

void WifiMeasurement(){
  initMatlabCommunication();
  myRTC.startAlarms();
  createBinFile();
  while(true){
    if(myRTC.getStartLogging()){
      logData();
      createCsvFile();
      binaryToCsv();
      csvOverTcp();
      digitalWrite(LED_BUILTIN, LOW);
      myRTC.waitForReset();
    }
  }
}

void initMatlabCommunication(){
  char date[15];

  TCPflush();
  delay(100);

  char message[] = "start of transmission";
  TCPwrite(message);
  delay(100);

  TCPread(date, 15);
  delay(100);

  TCPread(date, 15);
  myRTC.init(date);
  delay(100);

  TCPread(date, 15);
  delay(100);
  myRTC.setLoggingStart(date);

  TCPread(date, 15);
  delay(100);
  myRTC.setLoggingEnd(date);

  TCPread(date, 15);
  delay(100);
  myRTC.setNextReset(date);
}
