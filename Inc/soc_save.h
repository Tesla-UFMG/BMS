/*
 * soc_save.h
 *
 *  Created on: 1 de mai de 2023
 *      Author: Alan Franklin
 */

#ifndef SOC_SAVE_H_
#define SOC_SAVE_H_

#include "stdint.h"
#include "BMS.h"
#define SOC_DATA_FLASH_ADDRESS 0x0800fc00
#define WORDS_READ 3
#define FLASH_WORD_SIZE 3

enum {
	SOC_VALUE,
	REMAINING_CHARGE,
	NUMBER_OF_SAVES
};

void soc_save(uint32_t soc_value, uint32_t remaining_charge,BMS_struct *BMS);
void soc_read(uint32_t *read_soc_value, uint32_t *read_remaining_charge, uint32_t *read_number_of_saves);

#endif /* SOC_SAVE_H_ */
