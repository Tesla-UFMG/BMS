/*
 * indirect_temperature.h
 *
 *  Created on: 14 de mar de 2023
 *      Author: Haroldo
 */

#ifndef INDIRECT_TEMPERATURE_H_
#define INDIRECT_TEMPERATURE_H_

#include "BMS.h"

float calculate_single_temperature (float current, BMS_struct *BMS, int celula, int slave);

void calculate_temperatures (BMS_struct *BMS);

#endif
