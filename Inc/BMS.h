/****************************************************************/
/**					Team Formula Tesla UFMG - 2018				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Eric, Virginia							*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/
#ifndef BMS_H
#define BMS_H

#include "LTC6804.h"
#include "DHAB_s125.h"
#include "defines.h"

typedef struct BMS_struct {

	//GENERAL
	uint16_t error_flag;
	uint8_t communication_mode;
	uint8_t opperation_mode;
	uint8_t opperating_packs;
	uint8_t status;

	//VOLTAGE | TEMPERATURE
	LTC6804 *sensor_v[N_OF_PACKS];
	uint16_t lowest_cell;  //of the bank
	uint16_t GLV_voltage;
	uint16_t v_bank;

	//CURRENT
	float current[4];

	//CHARGE_PERCENTAGE
	int32_t charge;

	//AIR
	uint8_t AIR;

}BMS_struct;

void BMS_mode_selector(BMS_struct *BMS);

void BMS_init(BMS_struct *BMS);

void BMS_monitoring(BMS_struct *BMS);

uint8_t BMS_AIR_status(BMS_struct *BMS);

int BMS_charging(BMS_struct BMS);

int BMS_discharging(BMS_struct BMS);

int BMS_balance(BMS_struct *BMS);

int BMS_communication(BMS_struct *BMS);

void BMS_error(BMS_struct *BMS);

void BMS_can(BMS_struct *BMS);

void BMS_uart(BMS_struct *BMS);

#endif
