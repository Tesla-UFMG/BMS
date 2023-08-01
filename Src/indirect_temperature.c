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
#define TEMP_TIMER		0.5 //Delay between simulations (seconds)
#define h  20 //Mica plate convection coefficient
#define A  0.001596 //Mica plate surface area
#define CONSTANT_TEMPERATURE_CALC 0.00182 //(1/(0.495*1106.34)) Numerical equation's part

uint8_t initializeController = 0;

//setting initial values for temperatures. It's needed due to how the calculation works
void initialize_indirect_temperatures (BMS_struct *BMS)
{
	if(initializeController == 0)
	{
		for(int i = 0; i < NUMBER_OF_SLAVES; i++)
			{
			BMS->sensor[i]->GxV[1] = BMS->sensor[i]->GxV[0];
			BMS->sensor[i]->GxV[2] = BMS->sensor[i]->GxV[3];
			BMS->sensor[i]->GxV[4] = BMS->sensor[i]->GxV[3];
			BMS->sensor[i]->GxV[5] = BMS->sensor[i]->GxV[6];
			BMS->sensor[i]->GxV[7] = BMS->sensor[i]->GxV[6];
			BMS->sensor[i]->GxV[8] = BMS->sensor[i]->GxV[9];
			BMS->sensor[i]->GxV[10] = BMS->sensor[i]->GxV[11];
			}
		++initializeController;
	}
}

float calculate_single_temperature (float current, BMS_struct *BMS, int celula, int slave)
{
	uint16_t OCVVoltage[SOC_RANGE] = {28500, 32200, 32600, 32800, 33000, 33040, 33080, 33100, 33130, 33180, 36000};

	uint8_t SoCPossibleValues[SOC_RANGE] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

	float cellTemperature;
	float voltageDiference;
	uint16_t cellBMSVoltage = BMS->sensor[slave]->CxV[celula];

	//The formula respects the S.I, so, the temperatures have to be in Kelvin (Celsius + 273). Celsius is *10, so kelvin need to bem *10 as well
	uint16_t actualCellTemp = BMS->sensor[slave]->GxV[celula] + 2730;
	uint16_t nextCellTemp = BMS->sensor[slave]->GxV[celula+1] + 2730;
	uint16_t lastCellTemp = BMS->sensor[slave]->GxV[celula-1] + 2730;

	//Every 10* is being implemented due to temperature being multiplied per 10;

	for(int i = 0; i < SOC_RANGE; ++i)
	{
		if((BMS->socTruncatedValue == SoCPossibleValues[i]) && BMS->mode == BMS_CHARGING && (current < 0))
		{
			voltageDiference = (float)(OCVVoltage[i] - cellBMSVoltage)/10000;
			cellTemperature = actualCellTemp + CONSTANT_TEMPERATURE_CALC*((-1)*10*current*(voltageDiference) - 2*(2*actualCellTemp - lastCellTemp - nextCellTemp) - (h*A*(actualCellTemp - 3000)))*TEMP_TIMER - (2730);
			return (uint16_t)cellTemperature;
		}
		else if((BMS->socTruncatedValue + 10 == SoCPossibleValues[i]) && BMS->mode == BMS_MONITORING && (current > 0))
		{
			voltageDiference = (float)(OCVVoltage[i] - cellBMSVoltage)/10000;
			cellTemperature = actualCellTemp + CONSTANT_TEMPERATURE_CALC*(10*current*(voltageDiference) - 2*(2*actualCellTemp - lastCellTemp - nextCellTemp) - (h*A*(actualCellTemp - 3000)))*TEMP_TIMER - (2730);
			return (uint16_t)cellTemperature;
		}
		else if((BMS->socTruncatedValue + 10 == SoCPossibleValues[i]) && BMS->mode == BMS_MONITORING && (current < 0))
		{
			voltageDiference = (float)(OCVVoltage[i] - cellBMSVoltage)/10000;
			cellTemperature = actualCellTemp + CONSTANT_TEMPERATURE_CALC*(10*current*((-1)*voltageDiference) - 2*(2*actualCellTemp - lastCellTemp - nextCellTemp) - (h*A*(actualCellTemp - 3000)))*TEMP_TIMER - (2730);
			return (uint16_t)cellTemperature;
		}
		else
			return BMS->sensor[slave]->GxV[celula];
	}
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


