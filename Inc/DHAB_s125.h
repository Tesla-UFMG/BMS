/****************************************************************/
/**					Team Formula Tesla UFMG - 2018				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Eric, Virginia							*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/

#ifndef DHAB_H
#define DHAB_H

#include "defines.h"
#include "stm32f1xx_hal.h"
#include <math.h>


typedef struct DHAB{

	//CURRENT VARIABLES:

	uint16_t c_sum;
	uint8_t c_error[4];

}DHAB;

void DHAB_init();

uint8_t DHAB_get_error(DHAB* sensor);

#endif
