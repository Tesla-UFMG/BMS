/*
 * bms.h
 *
 *  Created on: 17 de mar de 2022
 *      Author: Thiago
 */

#ifndef BMS_H
#define BMS_H

#include "ltc.h"
#include "constants.h"
#include "stdlib.h"
#include "ID.h"

typedef struct BMS_struct {

	//GENERAL
	uint8_t mode;
	uint16_t error;
	uint8_t whichcell;

	LTC_sensor *sensor[NUMBER_OF_SLAVES];
	LTC_config *config;
//	DHAB_sensor *dhabSensor[N_OF_DHAB];

	uint16_t tractiveSystemVoltage;
	uint16_t maxCellVoltage;
	uint16_t minCellVoltage;
	uint16_t deltaVoltage;
	uint16_t maxCellTemperature;
	uint16_t averageCellTemperature;

	uint16_t lastMaxCellVoltageDebug;
	uint16_t lastMinCellVoltageDebug;
	uint16_t lastCellTempDebug;

	int8_t maxVoltageErrors;
	int8_t minVoltageErrors;
	int8_t maxTempErrors;

	//HIGH MONITORING
	uint8_t whichCellMaxVoltage;
	uint8_t whichCellMinVoltage;
	uint8_t maxTemperature;
	uint8_t maxStack;
	uint8_t minStack;

	//CURRENT
	float current[ADC_BUFFER_SIZE];
	float c_adc[ADC_BUFFER_SIZE];

	//AIR
	uint8_t AIR;

	//SoC of accumulator
	uint32_t socPrecisionValue;
	float remainingCharge;
	float actualCharge;
	float totalIntegration;
	float integration;
	uint16_t socTruncatedValue;

	//FLASH
	uint32_t read_soc;
	uint32_t read_rmc;
	uint32_t read_nos;
}BMS_struct;

typedef enum{

	OVER_VOLTAGE 	 = 0,
	UNDER_VOLTAGE  	 = 1,
	OVER_TEMPERATURE = 2

}BMS_RETRIES;

#define BMS_CONVERT_CELL 	1
#define BMS_CONVERT_GPIO	2
#define BMS_CONVERT_STAT	4
#define BMS_CONVERT_CONFIG	8
#define ACCUMULATOR_TOTAL_CHARGE 72000 //[40 A/h to Coulomb]

void BMS_Init(BMS_struct *BMS);
void BMS_SetSafetyLimits(BMS_struct* BMS);
void BMS_Monitoring(BMS_struct *BMS);
void BMS_Convert(uint8_t BMS_CONVERT, BMS_struct *BMS);
void BMS_Balance(BMS_struct *BMS);
void BMS_AIR_status(BMS_struct *BMS);
void BMS_ErrorTreatment(BMS_struct *BMS);
void BMS_SoC_Calculation(BMS_struct *BMS);
void BMS_Initial_Charge(BMS_struct *BMS);
uint16_t float2uint16(float f);

#endif
