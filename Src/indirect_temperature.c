///*
// * indirect_temperature.c
// *
// *  Created on: 16 de mar de 2023
// *      Author: Haroldo
// */

#include "constants.h"
#include "indirect_temperature.h"
#include "bms.h"
#include "ltc.h"

#define SOC_RANGE		11 //0% to 100% equally separeted
#define TEMP_TIMER		5 //Delay between simulations (seconds)
#define h  20 //Mica plate convection coefficient
#define A  0.001596 //Mica plate surface area
#define SC1_HIGH_RANGE 2 //200A Channel of sensor 1
#define SC2_HIGH_RANGE 0 //200A Channel of sensor 2
#define CONSTANT_TEMPERATURE_CALC 0.00182 //(1/(0.495*1106.34)) Numerical equation's part


float calculate_single_temperature (float current, BMS_struct *BMS, int celula, int slave)
{
	uint8_t OCVVoltage[SOC_RANGE] = {2.85, 3.22, 3.26, 3.28, 3.30, 3.304, 3.308, 3.31, 3.313, 3.318, 3.6};

	uint8_t SoCPossibleValues[SOC_RANGE] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

	float cellTemperature;
	uint16_t cellBMSVoltage = BMS->sensor[slave]->CxV[celula];

	//The formula respects the S.I, so, the temperatures have to be in Kelvin (Celsius + 273)
	uint16_t actualCellTemp = BMS->sensor[slave]->GxV[celula] + 273;
	uint16_t nextCellTemp = BMS->sensor[slave]->GxV[celula+1] + 273;
	uint16_t lastCellTemp = BMS->sensor[slave]->GxV[celula-1] + 273;

	for(int i = 0; i < SOC_RANGE; ++i)
	{
		if(BMS->socTruncatedValue == SoCPossibleValues[i])
		{
			cellTemperature = actualCellTemp + CONSTANT_TEMPERATURE_CALC*(current*(OCVVoltage[i] - cellBMSVoltage) - 2*(2*actualCellTemp - lastCellTemp - nextCellTemp) - (h*A*(actualCellTemp - 300)))*TEMP_TIMER - (273);
		};
	}
	return cellTemperature;
}

void calculate_temperatures (BMS_struct *BMS)
{
	for(int i = 0; i < NUMBER_OF_SLAVES; ++i)
	{
		if(i == 0 || i == 1)
		{
			BMS->sensor[i]->GxV[1] = calculate_single_temperature(BMS->current[SC1_HIGH_RANGE], BMS, 1, i);
			BMS->sensor[i]->GxV[2] = calculate_single_temperature(BMS->current[SC1_HIGH_RANGE], BMS, 2, i);
			BMS->sensor[i]->GxV[4] = calculate_single_temperature(BMS->current[SC1_HIGH_RANGE], BMS, 4, i);
			BMS->sensor[i]->GxV[5] = calculate_single_temperature(BMS->current[SC1_HIGH_RANGE], BMS, 5, i);
			BMS->sensor[i]->GxV[7] = calculate_single_temperature(BMS->current[SC1_HIGH_RANGE], BMS, 7, i);
			BMS->sensor[i]->GxV[8] = calculate_single_temperature(BMS->current[SC1_HIGH_RANGE], BMS, 8, i);
			BMS->sensor[i]->GxV[10] = calculate_single_temperature(BMS->current[SC1_HIGH_RANGE], BMS, 10, i);
		}
		else if(i == 2 || i == 3)
		{
			BMS->sensor[i]->GxV[1] = calculate_single_temperature(BMS->current[SC2_HIGH_RANGE], BMS, 1, i);
			BMS->sensor[i]->GxV[2] = calculate_single_temperature(BMS->current[SC2_HIGH_RANGE], BMS, 2, i);
			BMS->sensor[i]->GxV[4] = calculate_single_temperature(BMS->current[SC2_HIGH_RANGE], BMS, 4, i);
			BMS->sensor[i]->GxV[5] = calculate_single_temperature(BMS->current[SC2_HIGH_RANGE], BMS, 5, i);
			BMS->sensor[i]->GxV[7] = calculate_single_temperature(BMS->current[SC2_HIGH_RANGE], BMS, 7, i);
			BMS->sensor[i]->GxV[8] = calculate_single_temperature(BMS->current[SC2_HIGH_RANGE], BMS, 8, i);
			BMS->sensor[i]->GxV[10] = calculate_single_temperature(BMS->current[SC2_HIGH_RANGE], BMS, 10, i);
		}
	}
}


