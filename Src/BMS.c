/*
 * bms.c
 *
 *  Created on: 17 de mar de 2022
 *      Author: Thiago
 */

#include "bms.h"
#include "can.h"
#include "charger.h"
#include "constants.h"
#include "dwt_delay.h"
#include "main.h"
#include "led.h"
#include "shutdown_circuit.h"
#include "stdbool.h"

static int8_t retries[NUMBER_OF_ERRORS];
static uint16_t safety_limits[NUMBER_OF_ERRORS];
bool error_flag[NUMBER_OF_ERRORS];


void BMS_Init(BMS_struct *BMS) {
	BMS->config = (LTC_config*) calloc(1 ,sizeof(LTC_config));
	BMS->config->command = (LTC_command*) calloc(1 ,sizeof(LTC_command));

	for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++){
		BMS->sensor[i] = (LTC_sensor*) calloc(1, sizeof(LTC_sensor));
		BMS->sensor[i]->ADDR = i;
		LTC_Init(BMS->config);
	}

	BMS->error = ERR_NO_ERROR;
	BMS->mode  = BMS_MONITORING;
	BMS_SetSafetyLimits(BMS);

	close_shutdown_circuit();
	bms_indicator_light_turn(OFF);
	led_debug_turn(OFF);
	if(BMS->mode == BMS_CHARGING)
		charger_enable();
	else
		charger_disable();

	LTC_Init(BMS->config);
}

void BMS_SetSafetyLimits(BMS_struct* BMS) {
	switch (BMS->mode) {
		case BMS_MONITORING:
			safety_limits[OVER_VOLTAGE]     = VOLTAGE_MAX_DISCHARGE;
			safety_limits[UNDER_VOLTAGE]    = VOLTAGE_MIN_DISCHARGE;
			safety_limits[OVER_TEMPERATURE] = TEMPERATURE_MAX_DISCHARGE;
			break;
		case BMS_CHARGING:
			safety_limits[OVER_VOLTAGE]     = VOLTAGE_MAX_CHARGE;
			safety_limits[UNDER_VOLTAGE]    = VOLTAGE_MIN_CHARGE;
			safety_limits[OVER_TEMPERATURE] = TEMPERATURE_MAX_CHARGE;
			break;
		case BMS_BALANCING:
			safety_limits[OVER_VOLTAGE]     = VOLTAGE_MAX_BALANCE;
			safety_limits[UNDER_VOLTAGE]    = VOLTAGE_MIN_BALANCE;
			safety_limits[OVER_TEMPERATURE] = TEMPERATURE_MAX_BALANCE;
			break;
		default:
			Error_Handler();
			break;
	}
}

void BMS_Convert(uint8_t BMS_CONVERT, BMS_struct *BMS) {
	if (BMS_CONVERT&BMS_CONVERT_CELL) {
		BMS->config->command->NAME = LTC_COMMAND_ADCV;
		BMS->config->command->BROADCAST = true;
		LTC_SendCommand(BMS->config);

		BMS->minCellVoltage = UINT16_MAX;
		BMS->maxCellVoltage = 0;
		for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++) {
			LTC_Read(LTC_READ_CELL, BMS->config, BMS->sensor[i]);
			if(BMS->sensor[i]->V_MIN < BMS->minCellVoltage)
				BMS->minCellVoltage = BMS->sensor[i]->V_MIN;
			if(BMS->sensor[i]->V_MAX > BMS->maxCellVoltage)
				BMS->maxCellVoltage = BMS->sensor[i]->V_MAX;
		}
	}
	if (BMS_CONVERT&BMS_CONVERT_GPIO) {
		BMS->config->command->NAME = LTC_COMMAND_ADAX;
		BMS->config->command->BROADCAST = true;
		LTC_SendCommand(BMS->config);

		BMS->maxCellTemperature = 0;
		for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++) {
			LTC_Read(LTC_READ_GPIO, BMS->config, BMS->sensor[i]);
			for(uint8_t j = 0; j < NUMBER_OF_THERMISTORS; j++){
				if(BMS->sensor[i]->GxV[j] > BMS->maxCellTemperature)
					BMS->maxCellTemperature = BMS->sensor[i]->GxV[j];
			}
		}
	}
	if (BMS_CONVERT&BMS_CONVERT_STAT) {
		BMS->config->command->NAME = LTC_COMMAND_ADSTAT;
		BMS->config->command->BROADCAST = true;
		LTC_SendCommand(BMS->config);

		BMS->tractiveSystemVoltage = 0;
		for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++) {
			LTC_Read(LTC_READ_STATUS, BMS->config, BMS->sensor[i]);
			BMS->tractiveSystemVoltage += BMS->sensor[i]->SOC;
		}
		BMS->tractiveSystemVoltage /= (NUMBER_OF_PACKS / NUMBER_OF_PACKS_IN_SERIES);
	}
}

void BMS_Balance(BMS_struct* BMS) {
	if(BMS->mode & BMS_BALANCING) {
		if(BMS->error == ERR_NO_ERROR) {
			for(uint8_t i = 0; i < TIME_BALANCING_SEC; i++) {
				for(uint8_t j = 0; j < NUMBER_OF_SLAVES; j++) {
					LTC_SetBalanceFlag(BMS->config, BMS->sensor[j]);
					LTC_Balance(BMS->config, BMS->sensor[j]);
				}
				DWT_Delay_us(ONE_SEC_IN_US);
			}
		}else{
			for(uint8_t j = 0; j < NUMBER_OF_SLAVES; j++) {
				LTC_ResetBalanceFlag(BMS->config, BMS->sensor[j]);
				LTC_Balance(BMS->config, BMS->sensor[j]);
			}
		}
	}
}

void BMS_AIR_Status(BMS_struct* BMS) {
	if(HAL_GPIO_ReadPin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin) == GPIO_PIN_RESET)
		BMS->AIR = AIR_OPEN;
	else
		BMS->AIR = AIR_CLOSED;
}

void BMS_Monitoring(BMS_struct* BMS) {
	BMS_Convert(BMS_CONVERT_CELL|BMS_CONVERT_GPIO|BMS_CONVERT_STAT, BMS);
	BMS_Balance(BMS);
	BMS_AIR_Status(BMS);
}

void BMS_ErrorTreatment(BMS_struct *BMS) {
	retries[OVER_VOLTAGE]     += BMS->maxCellVoltage > safety_limits[OVER_VOLTAGE]  ? 1 : -1;
	retries[UNDER_VOLTAGE]    += BMS->minCellVoltage < safety_limits[UNDER_VOLTAGE] ? 1 : -1;
	retries[OVER_TEMPERATURE] += BMS->maxCellTemperature > safety_limits[OVER_TEMPERATURE] ? 1 : -1;

	for(uint8_t i = 0; i < NUMBER_OF_ERRORS; i++) {
		if(retries[i] >= MAX_RETRIES) {
			retries[i] = MAX_RETRIES;
			BMS->error |= (1 << i);
			error_flag[i] = true;
		}else if(retries[i] < 0)
			retries[i] = 0;
	}

	if(BMS->error != ERR_NO_ERROR) {
		charger_disable();
		open_shutdown_circuit();
		bms_indicator_light_turn(ON);
		led_debug_turn(ON);
	}
}
