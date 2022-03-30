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
#include "eeprom.h"
#include "can.h"
#include "defines.h"

uint8_t NextError[5];

typedef struct BMS_struct {

	//GENERAL
	uint16_t error;
	uint8_t communication_mode;
	uint8_t mode;
	uint8_t discharging;
	uint8_t opperating_packs;
	uint8_t status;

	LTC_sensor *sensor[N_OF_SLAVES];
	LTC_config *config;
	DHAB_sensor *dhabSensor[N_OF_DHAB];

	uint16_t v_GLV;
	uint16_t v_TS;
	uint16_t v_min;  	//of the bank
	uint16_t v_max; 	//of the bank
	uint16_t v_delta;   //of the bank
	uint16_t t_max;		//of the bank

	//CURRENT
	float current[4];
	float c_adc[4];

	//CHARGE_PERCENTAGE
	int32_t charge;
	float charge_percent;
	int32_t charge_max;
	int32_t charge_min;
	float charge_variation_percent;
	float discharge_percent;
	float discharge_variation_percent;

	//AIR
	uint8_t AIR;

}BMS_struct;

void BMS_mode_selector(BMS_struct *BMS);

void BMS_Init(BMS_struct *BMS);

void BMS_convert(uint8_t BMS_CONVERT, BMS_struct *BMS);

void BMS_monitoring(BMS_struct *BMS);

uint8_t BMS_AIR_status(BMS_struct *BMS);

void BMS_charging(BMS_struct BMS);

void BMS_discharging(BMS_struct BMS);

int BMS_balance(BMS_struct *BMS);

int BMS_communication(BMS_struct *BMS);

void BMS_error(BMS_struct *BMS);

void BMS_can(BMS_struct *BMS);

void BMS_uart(BMS_struct *BMS);

void BMS_initial_SOC(BMS_struct *BMS);

#endif
