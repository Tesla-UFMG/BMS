/*
 * nextion_functions.h
 *
 *  Created on: 13 de ago de 2019
 *      Author: Rodolfo Lessa
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "bms.h"

// NEXTION PAGES DEFINES
typedef enum
{
	PAGE0 = 0,
	PAGE1,
	PAGE2,
	PAGE3,
	PAGE4
} NextionPage_e;
	
typedef struct
{
	uint16_t actual;
	uint16_t previous;
} NextionData_t;

typedef enum
{
	NO_ADVICE,
	GLV,
	BSE
} NextionAdvice_e;

// uint8_t actual_page;

void display_init();
// void uart3MessageReceived(BMS_struct *BMS);
uint8_t nexSetPageError(BMS_struct *BMS);
void display_show(BMS_struct *BMS);

#endif /* DISPLAY_H_ */
