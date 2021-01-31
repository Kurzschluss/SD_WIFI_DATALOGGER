/**
 * @file tcp.h
 * @author Simon Wilkes (simonwilkes@hotmail.de)
 * @brief provides class for communicating with a dedicated Matlab script
 * @version 1.0
 * @date 2021-01-31
 * 
 * 
 */
#ifndef _TCP_
#define _TCP_
#include <WiFi101.h>
#include "config.h"

// ------------------------------------------------------------------------------------
// Definitions



/**
 * @brief provides means for communicating with a dedicated Matlab script
 * 
 */
class MyWifi{
	public:
        /**
         * @brief Construct a new My Wifi object
         * 
         * @param port optional (default = 23)
         */
		MyWifi(const int port = 23);

        /**
         * @brief initializes the Wifi. Only returns when connection is established. Uses ssid and pass
         * Defined in config.h
         * 
         */
		void init();

        /**
         * @brief Prints the Wifi Status to Serial
         * 
         */
		void printStatus();

        /**
         * @brief waits for client connection. Only returns when established
         * 
         */
		void waitForClient();

        /**
         * @brief 
         * 
         * @param buffer char[] received characters get saved to this buffer
         * @param len max count of characters to receive
         * @return int number of characters received returns 0 when buffer was not big enough
         */
		int read(char* buffer, int len);

        /**
         * @brief writes buffer to TCP connection. Adds an \r at the end (ASCII 13)
         * 
         * @param buffer char date to get send
         */
		void write(char* buffer);

        /**
         * @brief flushes the tcp connection
         * 
         */
		void flush();

        /**
         * @brief saves the connected client
         * 
         */
		WiFiClient client;
	private:

        /**
         * @brief server object
         * 
         */
		WiFiServer server;

        /**
         * @brief status of server.
         * 
         */
		int status = WL_IDLE_STATUS;
};





#endif // _TCP_