/*
 * led.c
 *
 *  Created on: Mar 16, 2022
 *      Author: Thiago
 */

#include "led.h"
#include "main.h"
#include "stdbool.h"

void bms_indicator_light_turn(GPIO_PinState state) {
	HAL_GPIO_WritePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin, state);
}

void led_debug_turn(GPIO_PinState state) {
	//o LED de debug da master é acionado com a lógica inversa
	HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, !state);
}
