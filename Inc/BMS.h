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
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"

//BMS STRUCT
typedef struct BMS_struct {

	//GENERAL
	uint8_t mode;
	uint16_t error;

	LTC_sensor *sensor[NUMBER_OF_SLAVES];
	LTC_config *config;
//	DHAB_sensor *dhabSensor[N_OF_DHAB];

	uint16_t tractiveSystemVoltage;
	uint16_t maxCellVoltage;
	uint16_t minCellVoltage;
	uint16_t deltaVoltage;
	uint16_t maxCellTemperature;
	uint16_t averageCellTemperature;

	//CURRENT
	float current[ADC_BUFFER_SIZE];
	float c_adc[ADC_BUFFER_SIZE];

	//AIR
	uint8_t AIR;

	//SoC of accumulator
	uint8_t socValue;
	float remainingCharge;
	float totalIntegration;
	float integration;

}BMS_struct;

//BMS ERROR
typedef enum{

	OVER_VOLTAGE 	 = 0,
	UNDER_VOLTAGE  	 = 1,
	OVER_TEMPERATURE = 2

}BMS_RETRIES;

//Defines
#define BMS_CONVERT_CELL 	1
#define BMS_CONVERT_GPIO	2
#define BMS_CONVERT_STAT	4
#define BMS_CONVERT_CONFIG	8
#define ACCUMULATOR_TOTAL_CHARGE 40 //[A/h]

//Flash Memory
#define FLASH_USER_START_ADDR   ((uint32_t)0x0800F800) // Last sector of flash memory of STM32F103C8T6
#define FLASH_USER_END_ADDR     ((uint32_t)0x0800FFFF) // Last available flash memory address
#define FLASH_USER_SIZE         ((uint32_t)0x00008000) // Flash memory sector size
#define SOC_VALUE_ADDR          ((uint32_t)0x0800F800)
#define REMAINING_CHARGE_ADDR   ((uint32_t)0x0800F804)

//BMS Functions
void BMS_Init(BMS_struct *BMS);
void BMS_SetSafetyLimits(BMS_struct* BMS);
void BMS_Monitoring(BMS_struct *BMS);
void BMS_Convert(uint8_t BMS_CONVERT, BMS_struct *BMS);
void BMS_Balance(BMS_struct *BMS);
void BMS_AIR_status(BMS_struct *BMS);
void BMS_ErrorTreatment(BMS_struct *BMS);
void BMS_SoC_Calculation(BMS_struct *BMS);
void BMS_Initial_Charge(BMS_struct *BMS);
void BMS_WriteF_Flash(uint32_t address, float value);
void BMS_WriteI_Flash(uint32_t address, int value);
float BMS_ReadF_Flash(uint32_t address);
int BMS_ReadI_Flash(uint32_t address);
uint16_t float2uint16(float f);

#endif
