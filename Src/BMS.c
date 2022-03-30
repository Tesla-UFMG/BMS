/****************************************************************/
/**					Team Formula Tesla UFMG - 2018				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Eric, Virginia							*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/

#include "BMS.h"

static int8_t retries[NUMBER_OF_ERRORS];
static uint16_t safety_limits[NUMBER_OF_ERRORS];

static const uint16_t CAN_ID_TABLE[8][5] = {

		{
				256,		//PACK 2 		(cells 0, 1, 2, 3)
				257,		//PACK 2 		(cells 4, 5, 6, 7)
				258,		//PACK 2 		(cells 8, 9, 10, 11)
				259,		//PACK 2 		(cell 12, sensor 0, 1, 2)
				260,		//PACK 2 		(sensor 3, 4, 0, 0)
		},
		{
				261,		//PACK 2 		(cells 0, 1, 2, 3)
				262,		//PACK 2 		(cells 4, 5, 6, 7)
				263,		//PACK 2 		(cells 8, 9, 10, 11)
				264,		//PACK 2 		(cell 12, sensor 0, 1, 2)
				265,		//PACK 2 		(sensor 3, 4, 0, 0)
		},
		{
				266,		//PACK 2 		(cells 0, 1, 2, 3)
				267,		//PACK 2 		(cells 4, 5, 6, 7)
				268,		//PACK 2 		(cells 8, 9, 10, 11)
				269,		//PACK 2 		(cell 12, sensor 0, 1, 2)
				270,		//PACK 2 		(sensor 3, 4, 0, 0)
		},
		{
				271,		//PACK 2 		(cells 0, 1, 2, 3)
				272,		//PACK 2 		(cells 4, 5, 6, 7)
				273,		//PACK 2 		(cells 8, 9, 10, 11)
				274,		//PACK 2 		(cell 12, sensor 0, 1, 2)
				275,		//PACK 2 		(sensor 3, 4, 0, 0)
		},
		{
				276,		//PACK 2 		(cells 0, 1, 2, 3)
				277,		//PACK 2 		(cells 4, 5, 6, 7)
				278,		//PACK 2 		(cells 8, 9, 10, 11)
				279,		//PACK 2 		(cell 12, sensor 0, 1, 2)
				280,		//PACK 2 		(sensor 3, 4, 0, 0)
		},

		{
				281,		//PACK 2 		(cells 0, 1, 2, 3)
				282,		//PACK 2 		(cells 4, 5, 6, 7)
				283,		//PACK 2 		(cells 8, 9, 10, 11)
				284,		//PACK 2 		(cell 12, sensor 0, 1, 2)
				285,		//PACK 2 		(sensor 3, 4, 0, 0)
		},
		{
				50,		//CHARGING CURRENT (bytes 0, 1, 2, 3) AND DISCHARGING CURRENT(bytes 4, 5, 6, 7)
		},
		{
				51,		//BMS MODE(bytes 0),COMM MODE(bytes 1), AIR STATUS(bytes 2 & 3), BATTERY %(bytes 4 & 5), GLOBAL ERROR FLAG(byte 6)
				52,
				53
		},
};

void BMS_Init(BMS_struct *BMS) {
	BMS->config = (LTC_config*) calloc(1 ,sizeof(LTC_config));
	BMS->config->command = (LTC_command*) calloc(1 ,sizeof(LTC_command));

	for(uint8_t i = 0; i < N_OF_SLAVES; i++){
		BMS->sensor[i] = (LTC_sensor*) calloc(1, sizeof(LTC_sensor));
		BMS->sensor[i]->ADDR = i;
		LTC_Init(BMS->config);
	}

	BMS->error = ERR_NO_ERROR;
	BMS->mode  = BMS_MONITORING;
	BMS_SetSafetyLimits(BMS);

	open_shutdown();
	bms_indicator_light_turn(OFF);
	led_debug_turn(OFF);
	if(BMS->mode == BMS_CHARGING)
		chager_enable();
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
		BMS->config->command->BROADCAST = TRUE;
		LTC_SendCommand(BMS->config);

		BMS->v_min = UINT16_MAX;
		BMS->v_max = 0;
		for(uint8_t i = 0; i < N_OF_SLAVES; i++){
			LTC_Read(LTC_READ_CELL, BMS->config, BMS->sensor[i]);
			if(BMS->sensor[i]->V_MIN < BMS->v_min)
				BMS->v_min = BMS->sensor[i]->V_MIN;
			if(BMS->sensor[i]->V_MAX > BMS->v_max)
				BMS->v_max = BMS->sensor[i]->V_MAX;
		}
	}
	if (BMS_CONVERT&BMS_CONVERT_GPIO) {
		BMS->config->command->NAME = LTC_COMMAND_ADAX;
		BMS->config->command->BROADCAST = TRUE;
		LTC_SendCommand(BMS->config);

		BMS->t_max = 0;
		for(uint8_t i = 0; i < N_OF_SLAVES; i++){
			LTC_Read(LTC_READ_GPIO, BMS->config, BMS->sensor[i]);
			for(uint8_t j = 0; j < N_OF_THERMISTORS; j++){
				if(BMS->sensor[i]->GxV[j] > BMS->t_max)
					BMS->t_max = BMS->sensor[i]->GxV[j];
			}
		}
	}
	if (BMS_CONVERT&BMS_CONVERT_STAT) {
		BMS->config->command->NAME = LTC_COMMAND_ADSTAT;
		BMS->config->command->BROADCAST = TRUE;
		LTC_SendCommand(BMS->config);

		BMS->v_TS = 0;
		for(uint8_t i = 0; i < N_OF_SLAVES; i++){
			LTC_Read(LTC_READ_STATUS, BMS->config, BMS->sensor[i]);
			BMS->v_TS += BMS->sensor[i]->SOC;
		}
		BMS->v_TS /= (N_OF_PACKS / PACKS_IN_SERIES);
	}
}

void BMS_Balance(BMS_struct BMS) {
	if(BMS->mode & BMS_BALANCING) {
		for(uint8_t i = 0; i < TIME_BALANCING_SEC; i++) {
			for(uint8_t j = 0; j < N_OF_SLAVES; j++) {
				LTC_SetBalanceFlag(BMS->config, BMS->sensor[j]);
				LTC_Balance(BMS->config, BMS->sensor[j]);
			}
			DWT_Delay_us(ONE_SEC_IN_MS);
		}
	}
}

void BMS_AIR_Status(BMS_struct BMS) {
	if(HAL_GPIO_ReadPin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin) == GPIO_PIN_RESET)
		BMS->AIR = AIR_OPEN;
	else
		BMS->AIR = AIR_CLOSED;
}

void BMS_Monitoring(BMS_struct *BMS){
	BMS_Convert(BMS_CONVERT_CELL|BMS_CONVERT_GPIO|BMS_CONVERT_STAT, BMS);
	BMS_Balance(BMS);
	BMS_AIR_Status(BMS);
}

void BMS_ErrorTreatment(BMS_struct *BMS) {
	retries[OVER_VOLTAGE]     += BMS->maxCellVoltage > safety_limits[OVER_VOLTAGE]  ? 1 : -1;
	retries[UNDER_VOLTAGE]    += BMS->minCellVoltage < safety_limits[UNDER_VOLTAGE] ? 1 : -1;
	retries[OVER_TEMPERATURE] += BMS->maxCellTemperature > safety_limits[OVER_TEMPERATURE] ? 1 : -1;

	for(uint8_t i = 0; i < NUMBER_OF_ERRORS; i++) {
		if(retries[i] > MAX_RETRIES) {
			retries[i] = MAX_RETRIES;
			BMS->error |= (1 << i);
			error_flag[i] = true;
		}else if(retries[i] < 0)
			retries[i] = 0;
	}

	if(BMS->error != ERR_NO_ERROR){
		charger_disable();
		open_shutdown();
		bms_indicator_light_turn(ON);
		led_debug_turn(ON);
	}
}

void can_buf(uint8_t buffer[8], uint16_t word1, uint16_t word2, uint16_t word3, uint16_t word4){

	buffer[0] = word1;
	buffer[1] = word1 >> 8;
	buffer[2] = word2;
	buffer[3] = word2 >> 8;
	buffer[4] = word3;
	buffer[5] = word3 >> 8;
	buffer[6] = word4;
	buffer[7] = word4 >> 8;

}

void BMS_can(BMS_struct *BMS){
	uint8_t can_buffer[8];

	//PACK 5
	can_buffer[0] = BMS->sensor[0]->CxV[0];
	can_buffer[1] = BMS->sensor[0]->CxV[0] >> 8;
	can_buffer[2] = BMS->sensor[0]->CxV[1];
	can_buffer[3] = BMS->sensor[0]->CxV[1] >> 8;
	can_buffer[4] = BMS->sensor[0]->CxV[2];
	can_buffer[5] = BMS->sensor[0]->CxV[2] >> 8;
	can_buffer[6] = BMS->sensor[0]->CxV[3];
	can_buffer[7] = BMS->sensor[0]->CxV[3] >> 8;

	CAN_Transmit(can_buffer, 256);

	can_buffer[0] = BMS->sensor[0]->CxV[6];
	can_buffer[1] = BMS->sensor[0]->CxV[6] >> 8;
	can_buffer[2] = BMS->sensor[0]->CxV[7];
	can_buffer[3] = BMS->sensor[0]->CxV[7] >> 8;
	can_buffer[4] = BMS->sensor[0]->CxV[8];
	can_buffer[5] = BMS->sensor[0]->CxV[8] >> 8;
	can_buffer[6] = BMS->sensor[0]->CxV[9];
	can_buffer[7] = BMS->sensor[0]->CxV[9] >> 8;

	CAN_Transmit(can_buffer, 257);

	can_buffer[0] = BMS->sensor[1]->CxV[0];
	can_buffer[1] = BMS->sensor[1]->CxV[0] >> 8;
	can_buffer[2] = BMS->sensor[1]->CxV[1];
	can_buffer[3] = BMS->sensor[1]->CxV[1] >> 8;
	can_buffer[4] = BMS->sensor[1]->CxV[2];
	can_buffer[5] = BMS->sensor[1]->CxV[2] >> 8;
	can_buffer[6] = BMS->sensor[1]->CxV[3];
	can_buffer[7] = BMS->sensor[1]->CxV[3] >> 8;

	CAN_Transmit(can_buffer, 258);

	can_buffer[0] = BMS->sensor[1]->CxV[4];
	can_buffer[1] = BMS->sensor[1]->CxV[4] >> 8;
	can_buffer[2] = BMS->sensor[0]->GxV[0];
	can_buffer[3] = BMS->sensor[0]->GxV[0] >> 8;
	can_buffer[4] = BMS->sensor[0]->GxV[1];
	can_buffer[5] = BMS->sensor[0]->GxV[1] >> 8;
	can_buffer[6] = BMS->sensor[0]->GxV[2];
	can_buffer[7] = BMS->sensor[0]->GxV[2] >> 8;

	CAN_Transmit(can_buffer, 259);

	can_buffer[0] = BMS->sensor[0]->GxV[3];
	can_buffer[1] = BMS->sensor[0]->GxV[3] >> 8;
	can_buffer[2] = BMS->sensor[0]->GxV[4];
	can_buffer[3] = BMS->sensor[0]->GxV[4] >> 8;
	can_buffer[4] = 0;
	can_buffer[5] = 0 >> 8;
	can_buffer[6] = 0;
	can_buffer[7] = 0 >> 8;

	CAN_Transmit(can_buffer, 260);

	//PACK 6
	can_buffer[0] = BMS->sensor[1]->CxV[5];
	can_buffer[1] = BMS->sensor[1]->CxV[5] >> 8;
	can_buffer[2] = BMS->sensor[1]->CxV[6];
	can_buffer[3] = BMS->sensor[1]->CxV[6] >> 8;
	can_buffer[4] = BMS->sensor[1]->CxV[7];
	can_buffer[5] = BMS->sensor[1]->CxV[7] >> 8;
	can_buffer[6] = BMS->sensor[1]->CxV[8];
	can_buffer[7] = BMS->sensor[1]->CxV[8] >> 8;

	CAN_Transmit(can_buffer, 261);

	can_buffer[0] = BMS->sensor[1]->CxV[9];
	can_buffer[1] = BMS->sensor[1]->CxV[9] >> 8;
	can_buffer[2] = BMS->sensor[2]->CxV[0];
	can_buffer[3] = BMS->sensor[2]->CxV[0] >> 8;
	can_buffer[4] = BMS->sensor[2]->CxV[1];
	can_buffer[5] = BMS->sensor[2]->CxV[1] >> 8;
	can_buffer[6] = BMS->sensor[2]->CxV[2];
	can_buffer[7] = BMS->sensor[2]->CxV[2] >> 8;

	CAN_Transmit(can_buffer, 262);

	can_buffer[0] = BMS->sensor[2]->CxV[3];
	can_buffer[1] = BMS->sensor[2]->CxV[3] >> 8;
	can_buffer[2] = BMS->sensor[2]->CxV[6];
	can_buffer[3] = BMS->sensor[2]->CxV[6] >> 8;
	can_buffer[4] = BMS->sensor[2]->CxV[7];
	can_buffer[5] = BMS->sensor[2]->CxV[7] >> 8;
	can_buffer[6] = BMS->sensor[2]->CxV[8];
	can_buffer[7] = BMS->sensor[2]->CxV[8] >> 8;

	CAN_Transmit(can_buffer, 263);

	can_buffer[0] = BMS->sensor[2]->CxV[9];
	can_buffer[1] = BMS->sensor[2]->CxV[9] >> 8;
	can_buffer[2] = BMS->sensor[2]->GxV[0];
	can_buffer[3] = BMS->sensor[2]->GxV[0] >> 8;
	can_buffer[4] = BMS->sensor[2]->GxV[1];
	can_buffer[5] = BMS->sensor[2]->GxV[1] >> 8;
	can_buffer[6] = BMS->sensor[2]->GxV[2];
	can_buffer[7] = BMS->sensor[2]->GxV[2] >> 8;

	CAN_Transmit(can_buffer, 264);

	can_buffer[0] = BMS->sensor[2]->GxV[3];
	can_buffer[1] = BMS->sensor[2]->GxV[3] >> 8;
	can_buffer[2] = BMS->sensor[2]->GxV[4];
	can_buffer[3] = BMS->sensor[2]->GxV[4] >> 8;
	can_buffer[4] = 0;
	can_buffer[5] = 0 >> 8;
	can_buffer[6] = 0;
	can_buffer[7] = 0 >> 8;

	CAN_Transmit(can_buffer, 265);

	//PACK 4
	can_buffer[0] = BMS->sensor[5]->CxV[0];
	can_buffer[1] = BMS->sensor[5]->CxV[0] >> 8;
	can_buffer[2] = BMS->sensor[5]->CxV[1];
	can_buffer[3] = BMS->sensor[5]->CxV[1] >> 8;
	can_buffer[4] = BMS->sensor[5]->CxV[2];
	can_buffer[5] = BMS->sensor[5]->CxV[2] >> 8;
	can_buffer[6] = BMS->sensor[5]->CxV[3];
	can_buffer[7] = BMS->sensor[5]->CxV[3] >> 8;

	CAN_Transmit(can_buffer, 266);

	can_buffer[0] = BMS->sensor[5]->CxV[6];
	can_buffer[1] = BMS->sensor[5]->CxV[6] >> 8;
	can_buffer[2] = BMS->sensor[5]->CxV[7];
	can_buffer[3] = BMS->sensor[5]->CxV[7] >> 8;
	can_buffer[4] = BMS->sensor[5]->CxV[8];
	can_buffer[5] = BMS->sensor[5]->CxV[8] >> 8;
	can_buffer[6] = BMS->sensor[5]->CxV[9];
	can_buffer[7] = BMS->sensor[5]->CxV[9] >> 8;

	CAN_Transmit(can_buffer, 267);

	can_buffer[0] = BMS->sensor[4]->CxV[0];
	can_buffer[1] = BMS->sensor[4]->CxV[0] >> 8;
	can_buffer[2] = BMS->sensor[4]->CxV[1];
	can_buffer[3] = BMS->sensor[4]->CxV[1] >> 8;
	can_buffer[4] = BMS->sensor[4]->CxV[2];
	can_buffer[5] = BMS->sensor[4]->CxV[2] >> 8;
	can_buffer[6] = BMS->sensor[4]->CxV[3];
	can_buffer[7] = BMS->sensor[4]->CxV[3] >> 8;

	CAN_Transmit(can_buffer, 268);

	can_buffer[0] = BMS->sensor[4]->CxV[4];
	can_buffer[1] = BMS->sensor[4]->CxV[4] >> 8;
	can_buffer[2] = BMS->sensor[5]->GxV[0];
	can_buffer[3] = BMS->sensor[5]->GxV[0] >> 8;
	can_buffer[4] = BMS->sensor[5]->GxV[1];
	can_buffer[5] = BMS->sensor[5]->GxV[1] >> 8;
	can_buffer[6] = BMS->sensor[5]->GxV[2];
	can_buffer[7] = BMS->sensor[5]->GxV[2] >> 8;

	CAN_Transmit(can_buffer, 269);

	can_buffer[0] = BMS->sensor[5]->GxV[3];
	can_buffer[1] = BMS->sensor[5]->GxV[3] >> 8;
	can_buffer[2] = BMS->sensor[5]->GxV[4];
	can_buffer[3] = BMS->sensor[5]->GxV[4] >> 8;
	can_buffer[4] = 0;
	can_buffer[5] = 0 >> 8;
	can_buffer[6] = 0;
	can_buffer[7] = 0 >> 8;

	CAN_Transmit(can_buffer, 270);

	//PACK 3
	can_buffer[0] = BMS->sensor[4]->CxV[5];
	can_buffer[1] = BMS->sensor[4]->CxV[5] >> 8;
	can_buffer[2] = BMS->sensor[4]->CxV[6];
	can_buffer[3] = BMS->sensor[4]->CxV[6] >> 8;
	can_buffer[4] = BMS->sensor[4]->CxV[7];
	can_buffer[5] = BMS->sensor[4]->CxV[7] >> 8;
	can_buffer[6] = BMS->sensor[4]->CxV[8];
	can_buffer[7] = BMS->sensor[4]->CxV[8] >> 8;

	CAN_Transmit(can_buffer, 271);

	can_buffer[0] = BMS->sensor[4]->CxV[9];
	can_buffer[1] = BMS->sensor[4]->CxV[9] >> 8;
	can_buffer[2] = BMS->sensor[3]->CxV[0];
	can_buffer[3] = BMS->sensor[3]->CxV[0] >> 8;
	can_buffer[4] = BMS->sensor[3]->CxV[1];
	can_buffer[5] = BMS->sensor[3]->CxV[1] >> 8;
	can_buffer[6] = BMS->sensor[3]->CxV[2];
	can_buffer[7] = BMS->sensor[3]->CxV[2] >> 8;

	CAN_Transmit(can_buffer, 272);

	can_buffer[0] = BMS->sensor[3]->CxV[3];
	can_buffer[1] = BMS->sensor[3]->CxV[3] >> 8;
	can_buffer[2] = BMS->sensor[3]->CxV[6];
	can_buffer[3] = BMS->sensor[3]->CxV[6] >> 8;
	can_buffer[4] = BMS->sensor[3]->CxV[7];
	can_buffer[5] = BMS->sensor[3]->CxV[7] >> 8;
	can_buffer[6] = BMS->sensor[3]->CxV[8];
	can_buffer[7] = BMS->sensor[3]->CxV[8] >> 8;

	CAN_Transmit(can_buffer, 273);

	can_buffer[0] = BMS->sensor[3]->CxV[9];
	can_buffer[1] = BMS->sensor[3]->CxV[9] >> 8;
	can_buffer[2] = BMS->sensor[3]->GxV[0];
	can_buffer[3] = BMS->sensor[3]->GxV[0] >> 8;
	can_buffer[4] = BMS->sensor[3]->GxV[1];
	can_buffer[5] = BMS->sensor[3]->GxV[1] >> 8;
	can_buffer[6] = BMS->sensor[3]->GxV[2];
	can_buffer[7] = BMS->sensor[3]->GxV[2] >> 8;

	CAN_Transmit(can_buffer, 274);

	can_buffer[0] = BMS->sensor[3]->GxV[3];
	can_buffer[1] = BMS->sensor[3]->GxV[3] >> 8;
	can_buffer[2] = BMS->sensor[3]->GxV[4];
	can_buffer[3] = BMS->sensor[3]->GxV[4] >> 8;
	can_buffer[4] = 0;
	can_buffer[5] = 0 >> 8;
	can_buffer[6] = 0;
	can_buffer[7] = 0 >> 8;

	CAN_Transmit(can_buffer, 275);

	//PACK 2
	can_buffer[0] = BMS->sensor[6]->CxV[0];
	can_buffer[1] = BMS->sensor[6]->CxV[0] >> 8;
	can_buffer[2] = BMS->sensor[6]->CxV[1];
	can_buffer[3] = BMS->sensor[6]->CxV[1] >> 8;
	can_buffer[4] = BMS->sensor[6]->CxV[2];
	can_buffer[5] = BMS->sensor[6]->CxV[2] >> 8;
	can_buffer[6] = BMS->sensor[6]->CxV[3];
	can_buffer[7] = BMS->sensor[6]->CxV[3] >> 8;

	CAN_Transmit(can_buffer, 276);

	can_buffer[0] = BMS->sensor[6]->CxV[6];
	can_buffer[1] = BMS->sensor[6]->CxV[6] >> 8;
	can_buffer[2] = BMS->sensor[6]->CxV[7];
	can_buffer[3] = BMS->sensor[6]->CxV[7] >> 8;
	can_buffer[4] = BMS->sensor[6]->CxV[8];
	can_buffer[5] = BMS->sensor[6]->CxV[8] >> 8;
	can_buffer[6] = BMS->sensor[6]->CxV[9];
	can_buffer[7] = BMS->sensor[6]->CxV[9] >> 8;

	CAN_Transmit(can_buffer, 277);

	can_buffer[0] = BMS->sensor[7]->CxV[0];
	can_buffer[1] = BMS->sensor[7]->CxV[0] >> 8;
	can_buffer[2] = BMS->sensor[7]->CxV[1];
	can_buffer[3] = BMS->sensor[7]->CxV[1] >> 8;
	can_buffer[4] = BMS->sensor[7]->CxV[2];
	can_buffer[5] = BMS->sensor[7]->CxV[2] >> 8;
	can_buffer[6] = BMS->sensor[7]->CxV[3];
	can_buffer[7] = BMS->sensor[7]->CxV[3] >> 8;

	CAN_Transmit(can_buffer, 278);

	can_buffer[0] = BMS->sensor[7]->CxV[4];
	can_buffer[1] = BMS->sensor[7]->CxV[4] >> 8;
	can_buffer[2] = BMS->sensor[6]->GxV[0];
	can_buffer[3] = BMS->sensor[6]->GxV[0] >> 8;
	can_buffer[4] = BMS->sensor[6]->GxV[1];
	can_buffer[5] = BMS->sensor[6]->GxV[1] >> 8;
	can_buffer[6] = BMS->sensor[6]->GxV[2];
	can_buffer[7] = BMS->sensor[6]->GxV[2] >> 8;

	CAN_Transmit(can_buffer, 279);

	can_buffer[0] = BMS->sensor[6]->GxV[3];
	can_buffer[1] = BMS->sensor[6]->GxV[3] >> 8;
	can_buffer[2] = BMS->sensor[6]->GxV[4];
	can_buffer[3] = BMS->sensor[6]->GxV[4] >> 8;
	can_buffer[4] = 0;
	can_buffer[5] = 0 >> 8;
	can_buffer[6] = 0;
	can_buffer[7] = 0 >> 8;

	CAN_Transmit(can_buffer, 280);

	//PACK 1
	can_buffer[0] = BMS->sensor[7]->CxV[5];
	can_buffer[1] = BMS->sensor[7]->CxV[5] >> 8;
	can_buffer[2] = BMS->sensor[7]->CxV[6];
	can_buffer[3] = BMS->sensor[7]->CxV[6] >> 8;
	can_buffer[4] = BMS->sensor[7]->CxV[7];
	can_buffer[5] = BMS->sensor[7]->CxV[7] >> 8;
	can_buffer[6] = BMS->sensor[7]->CxV[8];
	can_buffer[7] = BMS->sensor[7]->CxV[8] >> 8;

	CAN_Transmit(can_buffer, 281);

	can_buffer[0] = BMS->sensor[7]->CxV[9];
	can_buffer[1] = BMS->sensor[7]->CxV[9] >> 8;
	can_buffer[2] = BMS->sensor[8]->CxV[0];
	can_buffer[3] = BMS->sensor[8]->CxV[0] >> 8;
	can_buffer[4] = BMS->sensor[8]->CxV[1];
	can_buffer[5] = BMS->sensor[8]->CxV[1] >> 8;
	can_buffer[6] = BMS->sensor[8]->CxV[2];
	can_buffer[7] = BMS->sensor[8]->CxV[2] >> 8;

	CAN_Transmit(can_buffer, 282);

	can_buffer[0] = BMS->sensor[8]->CxV[3];
	can_buffer[1] = BMS->sensor[8]->CxV[3] >> 8;
	can_buffer[2] = BMS->sensor[8]->CxV[6];
	can_buffer[3] = BMS->sensor[8]->CxV[6] >> 8;
	can_buffer[4] = BMS->sensor[8]->CxV[7];
	can_buffer[5] = BMS->sensor[8]->CxV[7] >> 8;
	can_buffer[6] = BMS->sensor[8]->CxV[8];
	can_buffer[7] = BMS->sensor[8]->CxV[8] >> 8;

	CAN_Transmit(can_buffer, 283);

	can_buffer[0] = BMS->sensor[8]->CxV[9];
	can_buffer[1] = BMS->sensor[8]->CxV[9] >> 8;
	can_buffer[2] = BMS->sensor[8]->GxV[0];
	can_buffer[3] = BMS->sensor[8]->GxV[0] >> 8;
	can_buffer[4] = BMS->sensor[8]->GxV[1];
	can_buffer[5] = BMS->sensor[8]->GxV[1] >> 8;
	can_buffer[6] = BMS->sensor[8]->GxV[2];
	can_buffer[7] = BMS->sensor[8]->GxV[2] >> 8;

	CAN_Transmit(can_buffer, 284);

	can_buffer[0] = BMS->sensor[8]->GxV[3];
	can_buffer[1] = BMS->sensor[8]->GxV[3] >> 8;
	can_buffer[2] = BMS->sensor[8]->GxV[4];
	can_buffer[3] = BMS->sensor[8]->GxV[4] >> 8;
	can_buffer[4] = 0;
	can_buffer[5] = 0 >> 8;
	can_buffer[6] = 0;
	can_buffer[7] = 0 >> 8;

	CAN_Transmit(can_buffer, 285);

	/*///////////////////////////////////////////////*/

	can_buffer[0] = ((int16_t)BMS->current[0]);
	can_buffer[1] = ((int16_t)BMS->current[0]) >> 8;
	can_buffer[2] = ((int16_t)BMS->current[1]);
	can_buffer[3] = ((int16_t)BMS->current[1]) >> 8;
	can_buffer[4] = ((int16_t)BMS->current[2]);
	can_buffer[5] = ((int16_t)BMS->current[2]) >> 8;
	can_buffer[6] = ((int16_t)BMS->current[3]);
	can_buffer[7] = ((int16_t)BMS->current[3]) >> 8;

	CAN_Transmit(can_buffer, 50);

	can_buf(can_buffer, 15,
			(uint16_t)(BMS->charge_percent),
			0,
			0);

	CAN_Transmit(can_buffer, 51);

	can_buffer[0] = 0;
	can_buffer[1] = 0;
	can_buffer[2] = BMS->v_TS;
	can_buffer[3] = BMS->v_TS >> 8;
	can_buffer[4] = 0;
	can_buffer[5] = 0;
	can_buffer[6] = BMS->t_max;
	can_buffer[7] = BMS->t_max >> 8;

	CAN_Transmit(can_buffer, 52);

	can_buf(can_buffer, BMS->v_min,
			BMS->v_max,
			0,
			0);

	CAN_Transmit(can_buffer, 53);
}
