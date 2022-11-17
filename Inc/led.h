/*
 * led.h
 *
 *  Created on: Mar 16, 2022
 *      Author: Thiago
 */

#ifndef LED_H_
#define LED_H_

#include "stm32f1xx.h"

void bms_indicator_light_turn(GPIO_PinState state);
void led_debug_turn(GPIO_PinState state);

#endif /* LED_H_ */
