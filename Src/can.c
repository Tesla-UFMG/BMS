/*
 * can.c
 *
 *  Created on: Mar 17, 2022
 *      Author: Thiago
 */

#include "can.h"
#include "main.h"
#include "stm32f1xx.h"

extern CAN_HandleTypeDef hcan1;

static uint8_t can_buffer[8];
static CAN_TxHeaderTypeDef TxHeader;
static uint32_t TxMailbox;

void CAN_Init() {
	if (HAL_CAN_Start(&hcan1) != HAL_OK) {
	  /* Start Error */
	  Error_Handler();
	}
	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
	  /* Notification Error */
	  Error_Handler();
	}
	TxHeader.ExtId = 0x01;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.DLC = 8;
	TxHeader.TransmitGlobalTime = DISABLE;
}

void CAN_Buffer(uint16_t word1, uint16_t word2, uint16_t word3, uint16_t word4) {
	can_buffer[0] = word1;
	can_buffer[1] = word1 >> 8;
	can_buffer[2] = word2;
	can_buffer[3] = word2 >> 8;
	can_buffer[4] = word3;
	can_buffer[5] = word3 >> 8;
	can_buffer[6] = word4;
	can_buffer[7] = word4 >> 8;
}

void CAN_Transmit(uint32_t id) {
    TxHeader.StdId = id;
    if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, can_buffer, &TxMailbox) != HAL_OK) {
    	Error_Handler();
    }
    HAL_Delay(20);
}
