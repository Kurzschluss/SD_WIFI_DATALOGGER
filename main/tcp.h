/******************************************************************************************
/*	tcp.h
	Created: 13.01.2021
	Author: Simon Wilkes
	Last Modified: 13.01.2021
	By: Simon Wilkes
 
	Definitions of tcp.cpp that are needed by other files are written here.
******************************************************************************************/
#ifndef _TCP_
#define _TCP_
#include <WiFi101.h>

// ------------------------------------------------------------------------------------
// Definitions
#define MYSSID "testMe"
#define MYPASS "wasd1234"



// ------------------------------------------------------------------------------------
// Function prototypes

/**************************************************************************************
/*	ts_init
	
	@brief	waits for wifi connection to be established
			
			
	
	@return always 0, no error code usable
***************************************************************************************/
void initWifi();

void printWiFiStatus();

void waitForClient();

WiFiClient giveclient();

int TCPread(char* buffer, int len);

void TCPwrite(char* buffer);

void TCPflush();



#endif // _TCP_