/****************************************************************/
/**					Team Formula Tesla UFMG - 2018				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Eric, Virginia							*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/

#ifndef LTC6804_H
#define LTC6804_H

#include "defines.h"
#include "stm32f1xx_hal.h"
#include <math.h>

typedef struct LTC6804{

	//VOLTAGE VARIABLES:
	uint16_t v_cell[N_OF_CELLS];
	uint32_t v_sum;
	uint16_t v_max_cell;
	uint16_t v_min_cell;
	uint16_t v_error[N_OF_CELLS];
	uint16_t v_var;

	//TEMPERATURE VARIABLES:
	uint16_t t_cell[6];
	uint16_t t_max;
	uint32_t t_mean;
	uint8_t t_error[N_OF_THERMISTORS];

	uint8_t address, status;

}LTC6804;

void LTC6804_set_thermistor_zeros();

uint8_t LTC6804_init(LTC6804 *sensor, uint8_t address);

void LTC6804_convert_all();

uint8_t LTC6804_check_status(LTC6804* sensor);

uint8_t LTC6804_read_registers(uint16_t* data, uint8_t address, uint8_t option);

void LTC6804_convert_fuses();

uint8_t LTC6804_check_fuses(LTC6804 *sensor);

uint8_t LTC6804_get_error(LTC6804* sensor);

uint16_t LTC6804_convert_to_degrees(uint16_t value, uint16_t v_ref, uint16_t zero);

void LTC6804_balance_init(LTC6804 *sensor);

void LTC6804_balance_set_flag(LTC6804 *sensor);

void LTC6804_balance_reset_flag(LTC6804 *sensor);

void LTC6804_balance(LTC6804 *sensor);

void LTC6804_pause_balancing(LTC6804 *sensor);

#endif
