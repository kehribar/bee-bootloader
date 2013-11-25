/*-----------------------------------------------------------------------------
/ 
/	Analog related functions.
/
/------------------------------------------------------------------------------
/   ihsan Kehribar - 2013 
/----------------------------------------------------------------------------*/
#include "hal_analog.h"
/*---------------------------------------------------------------------------*/
void init_adc(uint8_t ch)
{
	/* Internal 1.1V Voltage Reference with external capacitor at AREF pin */
	ADMUX = (1<<REFS1)|(1<<REFS0)|ch;

	/* ADC enable with 128 clock prescale */
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}
/*---------------------------------------------------------------------------*/
uint16_t read_adc()
{
	return ADC;
}
/*---------------------------------------------------------------------------*/