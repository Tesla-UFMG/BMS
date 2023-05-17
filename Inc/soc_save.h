/*
 * soc_save.h
 *
 *  Created on: 1 de mai de 2023
 *      Author: Alan Franklin
 */

#ifndef SOC_SAVE_H_
#define SOC_SAVE_H_

#include "stdint.h"
#define SOC_DATA_FLASH_ADDRESS 0x0800fc00
#define WORDS_READ_TWO 2
#define FLASH_WORD_SIZE 2

enum {
	SOC_VALUE,
	REMAINING_CHARGE
};

void soc_save(uint32_t soc_value, uint32_t remaining_charge);
void soc_read(uint32_t *read_soc_value, uint32_t *read_remaining_charge);

#endif /* SOC_SAVE_H_ */
