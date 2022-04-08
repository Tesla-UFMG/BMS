/*
 * nextion_functions.h
 *
 *  Created on: 13 de ago de 2019
 *      Author: Rodolfo Lessa
 */

#ifndef NEXTION_FUNCTIONS_H_
#define NEXTION_FUNCTIONS_H_

#include "bms.h"

// NEXTION PAGES DEFINES
#define N_PAGE0 0
#define N_PAGE1 1
#define N_PAGE2 2
#define N_PAGE3 3
#define N_PAGE4 4
#define N_PAGE5 5
#define N_PAGE6 6
#define N_PAGE7 7

uint8_t actual_page;

void display_init();
void uart3MessageReceived(BMS_struct *BMS);
uint8_t nexSetPageError(BMS_struct *BMS);
void display_show(BMS_struct *BMS);

#endif /* NEXTION_FUNCTIONS_H_ */
