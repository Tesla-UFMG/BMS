/*
 * ntc.c
 *
 *  Created on: Mar 17, 2022
 *      Author: Thiago
 */

#include "ntc.h"

/*******************************************************
The function converts the ADC value read by the LTC6804
(on the voltage divider) into a resistance value of the thermistor.
The resistance value is then converted to temperature in °C.
The mathematical expression is presented below:

	1/T = 1/T0 + 1/B * ln(R/R0)

	T is the ambient temperature in Kelvins.
	T0 is the reference temperature, also in Kelvin (usually 25°C = 298.15K).
	B is the beta constant (datasheet).
	R is the thermistor resistance at the ambient temperature.
	R0 is the thermistor resistance at temperature T0.

	T' = 10 * (T - 273)

	T' is the ambient temperature in tenths of Celsius (to improve accuracy)
	Example: T = 298 K => T' = 250 d°C (25,0°C)
*******************************************************/
uint16_t NTC_ConvertTemp(uint16_t gpio_voltage, uint16_t reference_voltage) {
	if(reference_voltage - gpio_voltage != 0) {
		float r;
		uint16_t temperature_ddegC;
		r = ( gpio_voltage*10000) / (reference_voltage - gpio_voltage);
		temperature_ddegC = 10 * (1 / ( 1/(float)t0 + 1/(float)B * log(r/r0) ) - 273);
		return temperature_ddegC;
	}else {
		return 0;
	}

}
