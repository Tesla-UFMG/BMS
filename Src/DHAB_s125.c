/****************************************************************/
/**					Team Formula Tesla UFMG - 2018				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Eric, Virginia							*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/

#include "DHAB_s125.h"

/*******************************************************
 Function void DHAB_read(DHAB_sensor*)

V1.0:
The function uses the microcontroller's ADCs to read
and calculate the current passing through the DHAB S125
sensor.

 Version 1.0 - Initial release 15/05/2020 by Tesla UFMG
*******************************************************/
void DHAB_read(DHAB_sensor *sensor)
{
	for(int i = 0; i < N_OF_DHAB; i++){

	}
}

void DHAB_init(){

}

//uint8_t DHAB_get_error(DHAB* sensor){
//
//	uint8_t i, error_flag = 0;
//
//	for(i = 0; i < 4; i++){
//		if(sensor->current[i] > MAX_CURRENT)
//			sensor->c_error[i] = ERR_OVER_VOLTAGE;
//		else
//			sensor->c_error[i] = ERR_NO_ERROR;
//
//		error_flag |= sensor->c_error[i];
//
//	}
//	return error_flag;
//}

/*******************************************************
 Function double DHAB_currentIntegration(DHAB_sensor)

V1.0:
The function takes the read values from the current sensors
and calculates the amount of charge drawn or loaded by the
battery integrating the current in time after the conversion
is done.

 Version 1.0 - Initial release 15/05/2020 by Tesla UFMG
*******************************************************/
double DHAB_currentIntegration(DHAB_sensor *sensor){
	clock_t runningTime;
	double totalCoulombCounting = 0;

	runningTime = clock();
	if(ADC_FLAG_EOC && ADC_FLAG_JEOC){
		runningTime = clock() - runningTime;
		double timeSpent = (double)runningTime/CLOCK_PER_SEC;
		for(uint8_t i = 0; i < N_OF_DHAB; i++){
			sensor[i]->coulombCounting = sensor[i]->current*timeSpent;
			totalCoulombCounting = totalCoulombCounting + sensor[i]->coulombCounting;
		}
	}

	return totalCoulombCounting;
}
