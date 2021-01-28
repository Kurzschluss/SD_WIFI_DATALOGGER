/******************************************************************************************
/*	AdcSetup.h
	Created: 04.09.2019
	Author: Julian Peters
	Last Modified: 18.12.2019
	By: Julian Peters
 
	Definitions of AdcSetup.cpp that are needed by other files are written here.
******************************************************************************************/ 



#ifndef ADCSETUP_H_
#define ADCSETUP_H_



// ----------------------------------------------------------------------------------------
// Inlcudes

#include <stdint.h>



// ----------------------------------------------------------------------------------------
// Function prototypes

/******************************************************************************************
/*	adc_init
	
	@brief	Initializes the adc.
			Prescaler set to DIV4.
			Resolution set to adcResolution from configuration.h file.
			Sampling Length set to 0 (minimum).
			Adjusting set to 0.
			Enable ADC.
			Performing 1st Conversion, because that one must not be used.
	
	@dependencies	bs_init() must be called first, there the Pins of the Analog Inputs 
					are set                                                     
******************************************************************************************/
int adc_init(void);

/******************************************************************************************
/*	adc_getValue 
	@brief  converts the analog value of the as parameter specified pin to digital uint16_t.
			Select Positive Input for ADC (Pin to read) in MUXPOS (MUXNEG is GND)
			Clear the Data ready Flag in INTFLAG.
			Trigger the conversion.
			Wait for Conversion complete INTFLAG.bit.RESRDY .
			Return result.
	
	@param	arduinoPinNo: Pin Number where value shall be read in Arduino Specification
	@return result of ADC conversion in specified Resolution (configuration.h)                                                      
******************************************************************************************/
uint16_t adc_getValue(uint8_t arduinoPinNo);


#endif /* ADCSETUP_H_ */