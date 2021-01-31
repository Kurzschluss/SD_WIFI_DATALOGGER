/**
 * @file tcp.cpp
 * @author Simon Wilkes (simonwilkes@hotmail.de)
 * @brief implements functions of tcp.h
 * @version 1.0
 * @date 2021-01-31
 * 
 * 
 */
#include "tcp.h"
#include <WiFi101.h>


MyWifi::MyWifi(const int port):server(port){
}


void MyWifi::printStatus() {
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

void MyWifi::init(){    
    while ( status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(MYSSID);
        status = WiFi.begin(MYSSID, MYPASS);
        delay(1000);
    }
    Serial.println("connected to Wifi");
    server.begin();
    printStatus();
}


int MyWifi::read(char* buffer, int len){        
    int i = 0;
    while(i<len){
        char buf = client.read();   // read the bytes incoming from the client:        
        if(buf == 13){              // 13 == \r
            Serial.println("");
            return i;               //returns read count
        }
        buffer[i] = buf;
        Serial.print(buffer[i]);
        i++;
    }
    return 0;                       //to buffer not big enough
}

void MyWifi::write(char* buffer){
    client.println(buffer);         
}

void MyWifi::waitForClient(){
    Serial.println("Waiting for client connection");
    while(!client){                     ///only exit then client is available
        client = server.available();
    }
    Serial.println("We have a new client");
}


void MyWifi::flush(){
    client.flush();
}