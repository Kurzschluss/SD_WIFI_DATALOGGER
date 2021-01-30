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


void TC3_Handler(){
  bufferData(adc_getValue(14) * adcSensitivity);
  bufferData(adc_getValue(15) * adcSensitivity);
  if(getStopLogging()){
    ts_stopTimer();
  }
  TC3->COUNT16.INTFLAG.reg |= TC_INTFLAG_MC0; // Match Compare Register 0
}

void adcInit() {
  adc_init();
}


void setup(){
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
    // startmessaging();
    LongTimeMeasurement();
}

void LongTimeMeasurement(){
  initMatlabCommunication();
  RTCStartAlarms();
  createBinFile();
  while(true){
    if(getStartLogging()){
      logData();
      createCsvFile();
      binaryToCsv();
      csvOverTcp();
      digitalWrite(LED_BUILTIN, LOW);
      waitForReset();
    }
  }
}


void startmessaging(){
    // Read any Serial data.
    clearSerialInput();
    Serial.println();
    Serial.println(F("type:"));
    Serial.println(F("b - open existing bin file"));
    Serial.println(F("c - convert file to csv"));
    Serial.println(F("t - convert and send over tcp"));
    Serial.println(F("l - list files"));
    Serial.println(F("p - print data to Serial"));
    Serial.println(F("r - record ADC data"));
    Serial.println(F("s - start recording cycle (10s, 120s)"));
    while(!Serial.available()) {
        SysCall::yield();
    }
    char c = tolower(Serial.read());
    Serial.println();
    if (ERROR_LED_PIN >= 0) {
        digitalWrite(ERROR_LED_PIN, LOW);
    }
    // Read any Serial data.
    clearSerialInput();

    if (c == 'b') {
      openBinFile();
    } else if (c == 'c') {
        if (createCsvFile()) {
            binaryToCsv();
        }
    } else if (c == 'l') {
        readls();    
    } else if (c == 't') {
        if (createCsvFile()) {
      binaryToCsv();
        }
      csvOverTcp();    
    } else if (c == 'p') {
      printData();
    } else if (c == 's') {
      LongTimeMeasurement();
    } else if (c == 'r') {
      createBinFile();
      logData();
    } else {
      Serial.println(F("Invalid entry"));
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
  initRTC(date);
  TCPread(date, 15);
  delay(100);
  setLoggingStart(date);
  TCPread(date, 15);
  delay(100);
  setLoggingEnd(date);
  TCPread(date, 15);
  delay(100);
  setNextReset(date);
}
