/******************************************************************************************
/*	AdcSetup.cpp
	Created: 04.09.2019
	Author: Julian Peters
	Last Modified: 18.12.2019
	By: Julian Peters
 
	The code to handle the Analog-Digital-Converter is implemented in this file.
******************************************************************************************/ 



// ----------------------------------------------------------------------------------------
// Includes

#include "AdcSetup.h"
#include "sam.h"
//#include "Configuration.h"
#include "Arduino.h"



// ------------------------------------------------------------------------------------
// function implementations, description in prototype section above or in header-file

int adc_init(void)
{	
	// See also basic initialization in Arduino file 'wiring.cpp' in ArduinoCore/src
	
	// Disable ADC, because registers are enable protected
	ADC->CTRLA.bit.ENABLE = 0;                     
	  
	while ( ADC->STATUS.bit.SYNCBUSY == 1 );       // Wait for synchronization
	
	// Set Clock Divisor, as low as possible to increase speed
	ADC->CTRLB.bit.PRESCALER = ADC_CTRLB_PRESCALER_DIV32_Val;
	
	while ( ADC->STATUS.bit.SYNCBUSY == 1 );       // Wait for synchronization
	
	// Set resolution, the lower the faster
	// should also match the connected sensors resolution
	#if (ADC_RESOLUTION == 8)
	ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_8BIT_Val;
	
	#elif (ADC_RESOLUTION == 10)
	ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_10BIT_Val;
	
	#else
	ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_12BIT_Val;
	#endif // ADC_RESOLUTION
	
	while ( ADC->STATUS.bit.SYNCBUSY == 1 );       // Wait for synchronization
	  
	ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 |	// 1 sample, no averaging
					ADC_AVGCTRL_ADJRES(0x0ul);		// Adjusting result by 0
	//ADC->AVGCTRL.bit.SAMPLENUM = ADC_AVGCTRL_SAMPLENUM_1_Val;
	  
	// Sampling Time Length, Number of half CLK_ADC Cycles
	ADC->SAMPCTRL.reg = 3;                      
	
	// Optional: apply offset and gain corrections
	//ADC->OFFSETCORR.bit.OFFSETCORR;
	//ADC->GAINCORR.bit.GAINCORR;
	//ADC->CTRLB.bit.CORREN = 1;
	  
	// Enable ADC
	ADC->CTRLA.bit.ENABLE = 1;   
	                  
	// Wait for synchronization  
	while ( ADC->STATUS.bit.SYNCBUSY == 1 );
	
	// done in bs_init()
	//adc_configurePins();
	
	// Do one Conversion, because the first conversion after reference is changed must not be used
	adc_getValue(A1);
	  
	return 0;
}


uint16_t adc_getValue(uint8_t arduinoPinNo)
{
	// Wait for synchronization
	while ( ADC->STATUS.bit.SYNCBUSY == 1 );
	
	// Which Pin ?
	// Selection for the positive ADC input
	ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[arduinoPinNo].ulADCChannelNumber; 
	
	while ( ADC->STATUS.bit.SYNCBUSY == 1 );
	
	// manual Start of Conversion by setting Trigger Bit
	// This is a first Trash Conversion, necessary after change of MUXPOS ???
	//ADC->SWTRIG.bit.START = 1;
	
	// Clear the Data Ready flag
	ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
	
	// Sync for Trash conversion
	//while ( ADC->STATUS.bit.SYNCBUSY == 1 );
	
	// manual Start of Conversion by setting Trigger Bit
	// Maybe a trash conversion must be done
	// because MUXPOS may count as a reference change??? or is it only the AREF in REFCTRL
	ADC->SWTRIG.bit.START = 1;
	
	while( ADC->STATUS.bit.SYNCBUSY == 1 );  
	     
	// Wait for Conversion to Complete (RESRDY becomes 1 when ready)
	while( (1 != ADC->INTFLAG.bit.RESRDY) || (ADC->STATUS.bit.SYNCBUSY == 1) );
	
	// map Value to 0..analogRef(3V3)
	//uint16_t valueRead = ADC->RESULT.reg;
	//mapResolution(valueRead, adcResolution, 3300);
	
	// maybe Sync needed before accessing Result
	//while( ADC->STATUS.bit.SYNCBUSY == 1 );
	
	return 	ADC->RESULT.reg;	// uint16_t

}