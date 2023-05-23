/*
 * soc_save.c
 *
 *  Created on: 1 de mai de 2023
 *      Author: Alan Franklin
 */

#include "soc_save.h"
#include "FLASH_PAGE_F1.h"
#include "BMS.h"


void soc_save(uint32_t soc_value, uint32_t remaining_charge, BMS_struct *BMS){
	uint32_t soc_data[FLASH_WORD_SIZE] = {0, 0, 0};
	static uint32_t number_of_saves;

	soc_read(&BMS->read_soc, &BMS->read_rmc, &BMS->read_nos);
	//number_of_saves = (BMS->read_nos)=0; //Run the code for the first time
	number_of_saves = (BMS->read_nos)++; //Run the code like this after

	soc_data[SOC_VALUE] = soc_value;
	soc_data[REMAINING_CHARGE] = remaining_charge;
	soc_data[NUMBER_OF_SAVES] = number_of_saves;
	Flash_Write_Data(SOC_DATA_FLASH_ADDRESS, soc_data, FLASH_WORD_SIZE);
}

void soc_read(uint32_t *read_soc_value, uint32_t *read_remaining_charge, uint32_t *read_number_of_saves){
	uint32_t flash_read_data[FLASH_WORD_SIZE] = {0, 0, 0};
	Flash_Read_Data(SOC_DATA_FLASH_ADDRESS,flash_read_data, WORDS_READ);
	*read_soc_value = flash_read_data[SOC_VALUE];
	*read_remaining_charge = flash_read_data[REMAINING_CHARGE];
	*read_number_of_saves = flash_read_data[NUMBER_OF_SAVES];
}
