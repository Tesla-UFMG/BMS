/****************************************************************/
/**					Team Formula Tesla UFMG - 2018				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Eric, Virginia							*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/
#ifndef LED_H
#define LED_H

#include "stm32f1xx_hal.h"

void LED_init();

void LED_error(uint16_t ERROR_FLAG);

void LED_blink(uint32_t period_1, uint32_t period_2, uint8_t state_1, uint8_t state_2);

void LED_write(uint8_t state);

void LED_get_state(uint16_t mask);

#endif
