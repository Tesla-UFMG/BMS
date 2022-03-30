/*
 * can.h
 *
 *  Created on: Mar 17, 2022
 *      Author: Thiago
 */

#ifndef CAN_H_
#define CAN_H_

#include "stdint.h"

#define CAN_BUFFER_SIZE	4

void CAN_Init();
void CAN_Buffer(uint16_t word1, uint16_t word2, uint16_t word3, uint16_t word4);
void CAN_Transmit(uint32_t id);

#endif /* CAN_H_ */
