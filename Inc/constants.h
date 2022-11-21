/*
 * constants.h
 *
 *  Created on: 18 de mar de 2022
 *      Author: Thiago
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

//GENERAL DEFINES
#define NUMBER_OF_PACKS				4
#define NUMBER_OF_SLAVES			4
#define NUMBER_OF_CELLS				12
#define NUMBER_OF_THERMISTORS		5
#define NUMBER_OF_PACKS_IN_SERIES	2

//BATTERY CELL DEFINES
#define VOLTAGE_MAX_DISCHARGE		36000
#define VOLTAGE_MAX_CHARGE			35900
#define VOLTAGE_MAX_BALANCE			36000

#define VOLTAGE_MIN_DISCHARGE		22000
#define VOLTAGE_MIN_CHARGE			22000
#define VOLTAGE_MIN_BALANCE			22000

#define TEMPERATURE_MAX_DISCHARGE	550
#define TEMPERATURE_MAX_CHARGE		500
#define TEMPERATURE_MAX_BALANCE		400

//CELL BALANCE DEFINES
#define BALANCE_THRESHOLD_VOLTAGE	10
#define TIME_BALANCING_SEC			30
#define DELAY_AFTER_BALANCE_SEC		3

//ADC DEFINES
#define NUMBER_OF_ADC_CHANNELS 	NUMBER_OF_CURRENT_SENSORS*CHANELLS_PER_CURRENT_SENSOR
#define ADC_BUFFER_SIZE 		NUMBER_OF_ADC_CHANNELS
#define FILTER_GAIN 			255

//CURRENT SENSOR DEFINES
#define NUMBER_OF_CURRENT_SENSORS 		3
#define CHANELLS_PER_CURRENT_SENSOR		2
#define THRESHOLD_CURRENT_mA			3000

//BMS OPERATION MODES:
#define BMS_MONITORING 	0
#define BMS_CHARGING 	0b01
#define BMS_BALANCING 	0b10

//ERROR DEFINES:
#define ERR_NO_ERROR 			0b000000000
#define ERR_OVER_VOLTAGE		0b000000001
#define ERR_UNDER_VOLTAGE		0b000000010
#define ERR_OVER_CURRENT		0b000000100
#define ERR_OVER_TEMPERATURE	0b000001000
#define ERR_OPEN_FUSES			0b000010000
#define ERR_COMM_ERROR			0b000100000
#define ERR_AIR_ERROR			0b001000000
#define ERR_GLV_VOLTAGE			0b010000000
#define ERR_BALANCE				0b100000000
#define NUMBER_OF_ERRORS		3
#define MAX_RETRIES				5

//TIME DEFINES
#define T_WAKEUP 			400
#define T_REFUP_V 			20
#define T_ADC				3000
#define ONE_SEC_IN_US		1000000
#define CHARGER_DELAY		100

//AIR STATUS
#define AIR_OPEN	SET
#define AIR_CLOSED	RESET

//LOGIC DEFINES
#define HIGH  SET
#define LOW	  RESET
#define ON	  SET
#define OFF	  RESET

#endif /* CONSTANTS_H_ */
