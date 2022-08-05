/*
 * can.c
 *
 *  Created on: Mar 17, 2022
 *      Author: Thiago
 */

#include "can.h"
#include "main.h"
#include "stm32f1xx.h"

extern CAN_HandleTypeDef hcan;

CAN_FilterTypeDef sFilterConfig;
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef	RxHeader;
uint8_t TxData[8];
uint8_t RxData[8];
uint32_t TxMailbox;

void CAN_CofigFilter() {
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;

	if(HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK) {
		Error_Handler();
	}
}

void CAN_Init() {
	if (HAL_CAN_Start(&hcan) != HAL_OK) {
	  /* Start Error */
	  Error_Handler();
	}
	if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
	  /* Notification Error */
	  Error_Handler();
	}
	TxHeader.ExtId = 0x01;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.DLC = 8;
	TxHeader.TransmitGlobalTime = DISABLE;
}

void CAN_AddToBuffer(uint16_t word0, uint16_t word1, uint16_t word2, uint16_t word3) {
	TxData[0] = word0;
	TxData[1] = word0 >> 8;
	TxData[2] = word1;
	TxData[3] = word1 >> 8;
	TxData[4] = word2;
	TxData[5] = word2 >> 8;
	TxData[6] = word3;
	TxData[7] = word3 >> 8;
}

void CAN_SendMessage(uint32_t id) {
    TxHeader.StdId = id;
    if(HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
    	Error_Handler();
    }
    HAL_Delay(20);
}

void CAN_Transmit(uint16_t word0, uint16_t word1, uint16_t word2, uint16_t word3, uint32_t id) {
	CAN_AddToBuffer(word0, word1, word2, word3);
	CAN_SendMessage(id);
}


