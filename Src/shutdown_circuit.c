/*
 * shutdown_circuit.c
 *
 *  Created on: Mar 16, 2022
 *      Author: Thiago
 */

#include "shutdown_circuit.h"
#include "main.h"

void open_shutdown_circuit() {
	HAL_GPIO_WritePin(AIR_ENABLE_GPIO_Port, AIR_ENABLE_Pin, RESET);
}

void close_shutdown_circuit() {
	HAL_GPIO_WritePin(AIR_ENABLE_GPIO_Port, AIR_ENABLE_Pin, SET);
}
