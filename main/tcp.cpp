/******************************************************************************************
/*	tcp.cpp
	Created: 13.01.2021
	Author: Simon Wilkes
	Last Modified: 13.01.2021
	By: Simon Wilkes
 
	implements used functions of wifi communication
******************************************************************************************/
#include "tcp.h"
#include <WiFi101.h>

char ssid[] = MYSSID;
char pass[] = MYPASS;

WiFiServer server(23);
int status = WL_IDLE_STATUS;
WiFiClient client;

void printWiFiStatus() {
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

void initWifi(){    
    while ( status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass);
        delay(1000);
    }
    Serial.println("connected to Wifi");
    server.begin();
    printWiFiStatus();
}
void restartWifi(){
    WiFi.end();
    while ( status != WL_CONNECTED) {
        status = WiFi.begin(ssid, pass);
        delay(100);
    }
    server.begin();
    while(!client){
        client = server.available();
    }
}

int TCPread(char* buffer, int len){
    if (client.available() > 0) {
        // read the bytes incoming from the client:
        int i = 0;
        while(i<len){
            buffer[i] = client.read();
            Serial.print(buffer[i]);
            if(buffer[i] == 13){
            Serial.println("");
                return i;
            }
            i++;
        }
        return 0;
    }
}

void TCPwrite(char* buffer){
    client.println(buffer);
}

void waitForClient(){
    Serial.println("Waiting for client connection");
    while(!client){
        client = server.available();
    }
    Serial.println("We have a new client");
}


WiFiClient giveclient(){
    return client;
}

void TCPflush(){
    client.flush();
}