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
#include "timer_handler.h"
#include "indirect_temperature.h"

static int8_t retries[NUMBER_OF_ERRORS];
static uint16_t safety_limits[NUMBER_OF_ERRORS];
static uint32_t tempTimer;
bool error_flag[NUMBER_OF_ERRORS];



void BMS_Init(BMS_struct *BMS) {
	BMS->config = (LTC_config*) calloc(1 ,sizeof(LTC_config));
	BMS->config->command = (LTC_command*) calloc(1 ,sizeof(LTC_command));

	LTC_PEC_InitTable();

	for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++) {
		BMS->sensor[i] = (LTC_sensor*) calloc(1, sizeof(LTC_sensor));
		BMS->sensor[i]->ADDR = i;
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
		LTC_SendBroadcastCommand(BMS->config, LTC_COMMAND_ADCV);

		uint16_t aux_minCellVoltage = UINT16_MAX;
		uint16_t aux_maxCellVoltage = 0;
		for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++) {
			LTC_Read(LTC_READ_CELL, BMS->config, BMS->sensor[i]);
			if(BMS->sensor[i]->V_MIN < aux_minCellVoltage)
				aux_minCellVoltage = BMS->sensor[i]->V_MIN;
			if(BMS->sensor[i]->V_MAX > aux_maxCellVoltage)
				aux_maxCellVoltage = BMS->sensor[i]->V_MAX;
		}
		BMS->maxCellVoltage = aux_maxCellVoltage;
		BMS->minCellVoltage = aux_minCellVoltage;
		BMS->deltaVoltage = BMS->maxCellVoltage - BMS->minCellVoltage;
	}
	if (BMS_CONVERT&BMS_CONVERT_GPIO) {
		LTC_SendBroadcastCommand(BMS->config, LTC_COMMAND_ADCV);

		uint16_t aux_maxCellTemperature = 0;
		for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++) {
			LTC_Read(LTC_READ_GPIO, BMS->config, BMS->sensor[i]);
			for(uint8_t j = 0; j < NUMBER_OF_TEMPERATURE_MEASUREMENTS; j++){
				if(BMS->sensor[i]->GxV[j] > aux_maxCellTemperature)
					aux_maxCellTemperature = BMS->sensor[i]->GxV[j];
			}
		}
		BMS->maxCellTemperature = aux_maxCellTemperature;
	}
	if (BMS_CONVERT&BMS_CONVERT_STAT) {
		LTC_SendBroadcastCommand(BMS->config, LTC_COMMAND_ADCV);

		BMS->tractiveSystemVoltage = 0;
		for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++) {
			LTC_Read(LTC_READ_STATUS, BMS->config, BMS->sensor[i]);
			BMS->tractiveSystemVoltage += BMS->sensor[i]->SOC;
		}
		BMS->tractiveSystemVoltage /= (NUMBER_OF_PACKS / NUMBER_OF_PACKS_IN_SERIES);
	}
}

void BMS_ElectricalManagement(BMS_struct *BMS) {
	LTC_SendBroadcastCommand(BMS->config, LTC_COMMAND_ADCV);
	uint16_t aux_minCellVoltage = UINT16_MAX;
	uint16_t aux_maxCellVoltage = 0;
	for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++) {
		LTC_Read(LTC_READ_CELL, BMS->config, BMS->sensor[i]);
		if(BMS->sensor[i]->V_MIN < aux_minCellVoltage)
			aux_minCellVoltage = BMS->sensor[i]->V_MIN;
		if(BMS->sensor[i]->V_MAX > aux_maxCellVoltage)
			aux_maxCellVoltage = BMS->sensor[i]->V_MAX;
	}
	BMS->maxCellVoltage = aux_maxCellVoltage;
	BMS->minCellVoltage = aux_minCellVoltage;
	BMS->deltaVoltage = BMS->maxCellVoltage - BMS->minCellVoltage;
}

void BMS_ThermalManagement(BMS_struct *BMS) {
	LTC_SendBroadcastCommand(BMS->config, LTC_COMMAND_ADAX);
	uint16_t aux_maxCellTemperature = 0;
	int sumCellTemperature = 0;
	for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++)
	{
		LTC_Read(LTC_READ_GPIO, BMS->config, BMS->sensor[i]);
	}
	initialize_indirect_temperatures(BMS);
	if(timer_wait_ms(tempTimer, 500))
	{
		calculate_temperatures(BMS);
	    timer_restart(&tempTimer);
	}
	for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++)
	{
		for(uint8_t j = 0; j < NUMBER_OF_TEMPERATURE_MEASUREMENTS; j++)
		{
			if(BMS->sensor[i]->GxV[j] > aux_maxCellTemperature)
			{
			aux_maxCellTemperature = BMS->sensor[i]->GxV[j];
			BMS->whichcell = j;
			}
			sumCellTemperature += BMS->sensor[i]->GxV[j];
		}
		BMS->maxCellTemperature = aux_maxCellTemperature;
	}
	BMS->averageCellTemperature = sumCellTemperature / (NUMBER_OF_SLAVES*NUMBER_OF_TEMPERATURE_MEASUREMENTS);
}

void BMS_SafetyManagement(BMS_struct *BMS) {
	LTC_SendBroadcastCommand(BMS->config, LTC_COMMAND_ADCV);
	BMS->tractiveSystemVoltage = 0;
	for(uint8_t i = 0; i < NUMBER_OF_SLAVES; i++) {
		LTC_Read(LTC_READ_STATUS, BMS->config, BMS->sensor[i]);
		BMS->tractiveSystemVoltage += BMS->sensor[i]->SOC;
	}
	BMS->tractiveSystemVoltage /= (NUMBER_OF_PACKS / NUMBER_OF_PACKS_IN_SERIES);
	BMS_CheckContactorsStatus(BMS);
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

void BMS_CheckContactorsStatus(BMS_struct* BMS) {
	if(HAL_GPIO_ReadPin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin) == GPIO_PIN_RESET)
		BMS->AIR = AIR_OPEN;
	else
		BMS->AIR = AIR_CLOSED;
}

void BMS_Monitoring(BMS_struct* BMS) {
	BMS_ElectricalManagement(BMS);
	BMS_ThermalManagement(BMS);
	BMS_SafetyManagement(BMS);
	BMS_Balance(BMS);
}

void BMS_ErrorTreatment(BMS_struct *BMS) {
	retries[OVER_VOLTAGE]     += BMS->maxCellVoltage > safety_limits[OVER_VOLTAGE]  ? 1 : -1;
	retries[UNDER_VOLTAGE]    += BMS->minCellVoltage < safety_limits[UNDER_VOLTAGE] ? 1 : -1;
	retries[OVER_TEMPERATURE] += BMS->maxCellTemperature > safety_limits[OVER_TEMPERATURE] ? 1 : -1;
	BMS->maxTempErrors = retries[2];
	BMS->maxVoltageErrors = retries[0];
	BMS->minVoltageErrors = retries[1];

	for(uint8_t i = 0; i < NUMBER_OF_ERRORS; i++) {
		if(retries[i] >= MAX_RETRIES) {
			retries[i] = MAX_RETRIES;
			BMS->error |= (1 << i);
			error_flag[i] = true;
			BMS->lastCellTempDebug = BMS->maxCellTemperature;
			BMS->lastMaxCellVoltageDebug = BMS->maxCellVoltage;
			BMS->lastMinCellVoltageDebug = BMS->minCellVoltage;

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

void BMS_Datalloger(BMS_struct* BMS) {
	uint32_t can_id = CAN_ID_PACKS_INITIAL;
	LTC_sensor* sensor;
	for(uint8_t i = 0; i < NUMBER_OF_PACKS; i++) {
		sensor = BMS->sensor[i];
		for(uint8_t j = 0; j < NUMBER_OF_CELLS/CAN_BUFFER_SIZE; j++) {
			CAN_Transmit(sensor->CxV[j*CAN_BUFFER_SIZE],
					   sensor->CxV[j*CAN_BUFFER_SIZE + 1],
					   sensor->CxV[j*CAN_BUFFER_SIZE + 2],
					   sensor->CxV[j*CAN_BUFFER_SIZE + 3],
					   can_id);
			can_id++;
		}
		CAN_Transmit(sensor->GxV[0], sensor->GxV[1], sensor->GxV[2], sensor->GxV[3], can_id);
		can_id++;
		CAN_Transmit(sensor->GxV[4], sensor->SOC, sensor->REF, sensor->DCC, can_id);
		can_id++;
	}
	CAN_Transmit(BMS->maxCellVoltage, BMS->minCellVoltage, BMS->deltaVoltage, BMS->maxCellTemperature, 226);
	CAN_Transmit(BMS->mode, BMS->error, BMS->AIR, BMS->tractiveSystemVoltage, 227);
	//CAN_Transmit(16800, 50, 0, 0, 52);
	//CAN_Transmit(0, BMS->maxCellVoltage, BMS->averageCellTemperature, BMS->maxCellTemperature, 53);
	CAN_Transmit(BMS->minCellVoltage/100, 0, float2uint16(BMS->current[0]), float2uint16(BMS->current[1]), 54);
	CAN_Transmit(0,float2uint16(BMS->current[1]), 0, float2uint16(BMS->current[3]), 230);
	CAN_Transmit(0,0,0, BMS->averageCellTemperature, 231);
	CAN_Transmit(BMS->socTruncatedValue,0,0,0, 232);

}

uint16_t float2uint16(float f) {
	uint16_t gain   = 100;
	uint16_t offset = UINT16_MAX/2;
	return (uint16_t) (gain*f + offset);
}

void BMS_SoC_Calculation(BMS_struct *BMS) {
	BMS->actualCharge = BMS->remainingCharge - BMS->totalIntegration;
	BMS->socPrecisionValue = (BMS->actualCharge/ACCUMULATOR_TOTAL_CHARGE)*100;
	BMS->socTruncatedValue = (BMS->actualCharge/ACCUMULATOR_TOTAL_CHARGE)*10;
	BMS->socTruncatedValue *= 10; //Converting to percent
}

void BMS_Initial_Charge(BMS_struct *BMS) {
	soc_read(&BMS->read_soc, &BMS->read_rmc, &BMS->read_nos);
	//read_soc is divided per 1000 because when salving in flash to not lose the precision we *1000 and convert float to uint32_t.
	if((BMS->read_rmc)/1000 < ACCUMULATOR_TOTAL_CHARGE && (BMS->read_rmc/1000) != 0){
		BMS->remainingCharge = 144000;
	}
	else{
		BMS->remainingCharge = ACCUMULATOR_TOTAL_CHARGE;
		//We had problems when trying to multiply *100, but when doing in 2 ways it works
		BMS->socTruncatedValue = ((BMS->remainingCharge/ACCUMULATOR_TOTAL_CHARGE)*10);
		BMS->socTruncatedValue *= 10; //Converting to percent
	}
}
