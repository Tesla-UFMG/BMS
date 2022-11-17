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

#define CAN_ID_PACKS_INITIAL	260
#define CAN_ID_GENERAL1			51
#define CAN_ID_GENERAL2 		52

void CAN_Init();
void CAN_AddToBuffer(uint16_t word1, uint16_t word2, uint16_t word3, uint16_t word4);
void CAN_SendMessage(uint32_t id);
void CAN_Transmit(uint16_t word0, uint16_t word1, uint16_t word2, uint16_t word3, uint32_t id);

#endif /* CAN_H_ */
