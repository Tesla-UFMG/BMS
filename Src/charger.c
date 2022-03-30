/*
 * charger.c
 *
 *  Created on: Mar 16, 2022
 *      Author: Thiago
 */

#include "charger.h"
#include "main.h"
#include "dwt_stm32_delay.h"

void chager_enable() {
	HAL_GPIO_WritePin(CHARGE_ENABLE_GPIO_Port, CHARGE_ENABLE_Pin, SET);
	DWT_Delay_us(100);
}

void charger_disable() {
	HAL_GPIO_WritePin(CHARGE_ENABLE_GPIO_Port, CHARGE_ENABLE_Pin, RESET);
	DWT_Delay_us(100);
}