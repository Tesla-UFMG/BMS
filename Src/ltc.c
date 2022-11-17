/*
 * ltc.c
 *
 *  Created on: 15 de mar de 2022
 *      Author: Thiago
 */

#include "ltc.h"
#include "constants.h"
#include "main.h"
#include "ntc.h"
#include "dwt_delay.h"
#include "math.h"
#include "stdbool.h"

#define BYTESWAP(word) ((word >> 8) + (word << 8))

extern SPI_HandleTypeDef hspi1;
static uint16_t pec_table[LTC_PEC_TABLE_LENGTH];

void LTC_Init(LTC_config *config) {
	config->GPIO   = ALL_GPIOS_READ;
	config->REFON  = REFERENCE_SHUTS_DOWN_AFTER_CONVERSIONS;
	config->SWTRD  = SOFTWARE_TIMER_ENABLE_PIN_LOW;
	config->ADCOPT = SELECT_ADC_MODES_FAST;
	config->VUV    = DEFULT_VOLTAGE;
	config->VOV    = DEFULT_VOLTAGE;
	config->DCTO   = DISCHARGE_DISABLE;
	config->command->MD  = MD_FILTRED;
	config->command->DCP = DCP_PERMITED;
	LTC_SendBroadcastCommand(config, LTC_COMMAND_WRCOMM);
}

uint16_t LTC_PEC(uint16_t *data , uint8_t len) {
	int32_t remainder, address;
	remainder = LTC_PEC_SEED;
	for (uint8_t i = 0; i < len; i++) {
		address   = ((remainder >> 7) ^ ((data[i] >> 8) & 0xFF)) & 0xFF; //calculate PEC table address
		remainder = (remainder << 8 ) ^ pec_table[address];
		address   = ((remainder >> 7) ^ (data[i] & 0xFF)) & 0xFF;    	 //calculate PEC table address
		remainder = (remainder << 8 ) ^ pec_table[address];
	}
	return (remainder * 2); //The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
}

uint16_t LTC_PEC2(uint16_t* data, uint8_t size) {
	uint16_t aux, in0;
	uint16_t pec = LTC_PEC_SEED;
	for(int i = 0; i < size; i++) {
		data[i] = BYTESWAP(data[i]);
		for(int j = 15; j >= 0; j--) {
			aux = 0x00;
			in0 =  ((data[i] >> j) & 0x01) ^ ((pec >> 14) & 0x01);
			aux =  in0;
			aux =  aux << 4;
			aux += in0;
			aux =  aux << 2;
			aux += in0;
			aux =  aux << 1;
			aux += in0;
			aux =  aux << 3;
			aux += in0;
			aux =  aux << 1;
			aux += in0;
			aux =  aux << 3;
			aux += in0;
			pec =  aux ^ (pec << 1);
		}
		data[i] = BYTESWAP(data[i]);
	}
	pec = pec << 1;
	return (BYTESWAP(pec));
}

void LTC_PEC_InitTable() {
	uint16_t remainder;
	for(int i = 0; i < LTC_PEC_TABLE_LENGTH; i++) {
		remainder = i << 7;
		for(int bit = 8; bit > 0; --bit) {
			if(remainder & 0x4000) {
				remainder = remainder << 1;
				remainder = remainder ^ 0x4599;
			}
			else
				remainder = remainder << 1;
		}
		pec_table[i] = remainder&0xFFFF;
	}
}

void LTC_ChipSelect(uint8_t level) {
	HAL_GPIO_WritePin(ISOSPI_CS_GPIO_Port, ISOSPI_CS_Pin , level);
	DWT_Delay_us(10);
}

void LTC_StartTrasmission() { LTC_ChipSelect(LOW); }

void LTC_EndTramission() { LTC_ChipSelect(HIGH); }

uint16_t LTC_SPI(uint16_t Tx_data) {
	uint16_t Rx_data = 0;
	HAL_SPI_TransmitReceive(&hspi1,(uint8_t *) &Tx_data, (uint8_t *) &Rx_data, 1, 50);
	return(BYTESWAP(Rx_data));
}

void LTC_WakeUp() {
	LTC_StartTrasmission();
	LTC_SPI(0);
	LTC_EndTramission();
}

void LTC_TransmitCommand(uint16_t command) {
	uint16_t pec = LTC_PEC(&command, 1);
	LTC_SPI(command);
	LTC_SPI(pec);
}

void LTC_TransmitReceive(uint16_t command, uint16_t* tx_data, uint16_t* rx_data) {
	if((command & 0x07FF) == LTC_COMMAND_WRCFG) {
		uint16_t pec = LTC_PEC(tx_data, 3);
		tx_data[3] = pec;
	}
	if((tx_data[0] & 0x07FF) < LTC_COMMAND_ADCV) {
		for (uint8_t i = 0; i < SPI_BUFFER_LENGTH; ++i) {
			rx_data[i] = LTC_SPI(tx_data[i]);
		}
	}
}

uint16_t LTC_MakeCommand(LTC_command *command) {
	switch(command->NAME) {
		case LTC_COMMAND_ADCV:
			return command->NAME | command->MD | command->DCP | command->CH;
			break;

		case LTC_COMMAND_ADOW:
			return command->NAME | command->MD | command->PUP | command->DCP | command->CH;
			break;

		case LTC_COMMAND_CVST:
		case LTC_COMMAND_AXST:
		case LTC_COMMAND_STATST:
			return command->NAME | command->MD | command->ST;
			break;

		case LTC_COMMAND_ADAX	:
			return command->NAME | command->MD | command->CHG;
			break;

		case LTC_COMMAND_ADSTAT	:
			return command->NAME | command->MD | command->CHST;
			break;

		case LTC_COMMAND_ADCVAX	:
			return command->NAME | command->MD | command->CHG;
			break;

		default:
			return command->NAME;
			break;
	}
}

void LTC_ReceiveMessage(LTC_sensor* sensor, LTC_config* config, uint16_t rx_data[4]) {
	switch(config->command->NAME & 0x07FF) {

	case LTC_COMMAND_RDCFG:
		config->ADCOPT = (rx_data[0] & 0x1);
		config->SWTRD  = (rx_data[0] >> 1) & 0x1;
		config->REFON  = (rx_data[0] >> 2) & 0x1;
		config->GPIO   = (rx_data[0] >> 3) & 0x1F;
		config->VUV    = (rx_data[0] >> 8) | (rx_data[1] & 0x000F);
		config->VOV    = (rx_data[1] >> 4);
		config->DCTO   = (rx_data[2] >> 12);
		break;

	case LTC_COMMAND_RDCVA:
		sensor->CxV[0] = rx_data[0];
		sensor->CxV[1] = rx_data[1];
		sensor->CxV[2] = rx_data[2];
		break;

	case LTC_COMMAND_RDCVB:
		sensor->CxV[3] = rx_data[0];
		sensor->CxV[4] = rx_data[1];
		sensor->CxV[5] = rx_data[2];
		break;

	case LTC_COMMAND_RDCVC:
		sensor->CxV[6] = rx_data[0];
		sensor->CxV[7] = rx_data[1];
		sensor->CxV[8] = rx_data[2];
		break;

	case LTC_COMMAND_RDCVD:
		sensor->CxV[9]  = rx_data[0];
		sensor->CxV[10] = rx_data[1];
		sensor->CxV[11] = rx_data[2];
		break;

	case LTC_COMMAND_RDAUXA:
		sensor->GxV[0] = rx_data[0];
		sensor->GxV[1] = rx_data[1];
		sensor->GxV[2] = rx_data[2];
		break;

	case LTC_COMMAND_RDAUXB:
		sensor->GxV[3] = rx_data[0];
		sensor->GxV[4] = rx_data[1];
		sensor->REF    = rx_data[2];
		break;

	case LTC_COMMAND_RDSTATA:
		sensor->SOC  = rx_data[0] * 0.2;
		sensor->ITMP = rx_data[1] * 7.5;
		sensor->VA   = rx_data[2];
		break;

	case LTC_COMMAND_RDSTATB:
		sensor->VD = rx_data[0];
		uint32_t flag = rx_data[1] | (uint32_t)rx_data[2] << 16;
		for (uint8_t j = 0; j < LTC_MAX_SUPPORTED_CELLS; j++)
			sensor->V_ERROR[j] = (flag >> (j * 2)) & 0x3;
		break;

	case LTC_COMMAND_ADCV	:
	case LTC_COMMAND_ADOW	:
	case LTC_COMMAND_CVST	:
	case LTC_COMMAND_ADAX	:
	case LTC_COMMAND_AXST	:
	case LTC_COMMAND_ADSTAT	:
	case LTC_COMMAND_STATST	:
	case LTC_COMMAND_ADCVAX	:
		config->ADC_READY = false;
		break;

	case LTC_COMMAND_PLADC	:
		if(rx_data[0] == 0 || rx_data[1] == 0 || rx_data[2] == 0)
			config->ADC_READY = false;
		else
			config->ADC_READY = true;
		break;

	case LTC_COMMAND_DIAGN	:
		break;
	case LTC_COMMAND_WRCOMM	:
		break;
	case LTC_COMMAND_RDCOMM	:
		break;
	case LTC_COMMAND_STCOMM	:
		break;
	default:
		break;
	}
}

void LTC_ConfigCommandName(LTC_sensor* sensor, LTC_config* config) {
	config->command->NAME |= ((sensor->ADDR & (0x1111 * ~config->command->BROADCAST)) | ~config->command->BROADCAST << 4) << 11;
}

void LTC_WriteConfigRegister(LTC_sensor *sensor, LTC_config *config, uint16_t *tx_data) {
	tx_data[0] = (config->ADCOPT << 8) | (config->SWTRD << 9) | (config->REFON << 10) | (config->GPIO << 11) | (config->VUV);
	tx_data[1] = (config->VUV >> 8) | (config->VOV << 4);
	tx_data[2] |= ((sensor->DCC & 0xff) << 8) | ((sensor->DCC & 0xf00) >> 8) | ((config->DCTO & 0xf) << 4);
}

void LTC_Communication(LTC_config *config, uint16_t* tx_data, uint16_t* rx_data) {
	uint16_t command = LTC_MakeCommand(config->command);
	LTC_WakeUp();
	LTC_StartTrasmission();
	LTC_TransmitCommand(command);
	LTC_TransmitReceive(command, tx_data, rx_data);
	LTC_EndTramission();
}

void LTC_SendBroadcastCommand(LTC_config *config, uint16_t command_name) {
	uint16_t tx_data[4] = {0, 0, 0, 0};
	uint16_t rx_data[4] = {0, 0, 0, 0};
	config->command->NAME = command_name;
	LTC_Communication(config, tx_data, rx_data);
}

void LTC_SendAddressedCommand(LTC_config *config, LTC_sensor *sensor, uint16_t command_name) {
	uint16_t tx_data[4] = {0, 0, 0, 0};
	uint16_t rx_data[4] = {0, 0, 0, 0};

	config->command->NAME = command_name;
	LTC_ConfigCommandName(sensor, config);

	if((config->command->NAME & 0x07FF) == LTC_COMMAND_WRCFG)
		LTC_WriteConfigRegister(sensor, config, tx_data);

	LTC_Communication(config, tx_data, rx_data);
	LTC_ReceiveMessage(sensor, config, rx_data);
}

void LTC_ConvertTemp(LTC_sensor* sensor) {
	for(uint8_t i = 0; i < NUMBER_OF_THERMISTORS; i++){
		sensor->GxV[i] = NTC_ConvertTemp(sensor->GxV[i], sensor->REF);
	}
}

void LTC_Wait(LTC_config *config, LTC_sensor *sensor) {
	do{
		LTC_SendAddressedCommand(config, sensor, LTC_COMMAND_PLADC);
	}while(!config->ADC_READY);
}

void LTC_Read(uint8_t LTC_READ, LTC_config *config, LTC_sensor *sensor) {
	config->command->BROADCAST = false;
	if (LTC_READ&LTC_READ_CELL) {
		LTC_Wait(config, sensor);
		LTC_SendAddressedCommand(config, sensor, LTC_COMMAND_RDCVA);
		LTC_SendAddressedCommand(config, sensor, LTC_COMMAND_RDCVB);
		LTC_SendAddressedCommand(config, sensor, LTC_COMMAND_RDCVC);
		LTC_SendAddressedCommand(config, sensor, LTC_COMMAND_RDCVD);

		sensor->V_MIN = UINT16_MAX;
		sensor->V_MAX = 0;
		for(uint8_t i = 0; i < NUMBER_OF_CELLS; i++) {
			if(sensor->CxV[i] < sensor->V_MIN)
				sensor->V_MIN = sensor->CxV[i];
			if(sensor->CxV[i] > sensor->V_MAX)
				sensor->V_MAX = sensor->CxV[i];
		}
		sensor->V_DELTA = sensor->V_MAX - sensor->V_MIN;
	}
	if (LTC_READ&LTC_READ_GPIO) {
		LTC_Wait(config, sensor);
		LTC_SendAddressedCommand(config, sensor, LTC_COMMAND_RDAUXA);
		LTC_SendAddressedCommand(config, sensor, LTC_COMMAND_RDAUXB);
		LTC_ConvertTemp(sensor);
	}
	if (LTC_READ&LTC_READ_STATUS) {
		LTC_Wait(config, sensor);
		LTC_SendAddressedCommand(config, sensor, LTC_COMMAND_RDSTATA);
		LTC_SendAddressedCommand(config, sensor, LTC_COMMAND_RDSTATB);
	}
	if (LTC_READ&LTC_READ_CONFIG){
		LTC_Wait(config, sensor);
		LTC_SendAddressedCommand(config, sensor, LTC_COMMAND_RDCFG);
	}
}

void LTC_SetBalanceFlag(LTC_config *config, LTC_sensor *sensor) {
	sensor->DCC = 0;
	if(sensor->V_DELTA > BALANCE_THRESHOLD_VOLTAGE) {
		for (uint8_t i = 0; i < NUMBER_OF_CELLS; ++i) {
			if((sensor->CxV[i] - sensor->V_MIN) > BALANCE_THRESHOLD_VOLTAGE) {
				sensor->DCC |= 1 << i;
			}
		}
	}
}

void LTC_ResetBalanceFlag(LTC_config *config, LTC_sensor *sensor) { sensor->DCC = 0; }

void LTC_Balance(LTC_config *config, LTC_sensor *sensor) {
	LTC_SendAddressedCommand(config, sensor, LTC_COMMAND_WRCFG);
}
