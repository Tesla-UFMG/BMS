/**
  ******************************************************************************
  * File Name          : CAN.c
  * Description        : This file provides code for the configuration
  *                      of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "can.h"


/* USER CODE BEGIN 0 */

uint8_t* idRx;
uint8_t* vetRx;
uint8_t vetTx[__DLC__];

#include "dwt_stm32_delay.h"

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 9;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_6TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration    
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();
  
    /**CAN GPIO Configuration    
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

static void initRxMes(CAN_RxHeaderTypeDef *RxMessage)
{
  uint8_t i = 0;

  RxMessage->StdId = 0x321;
  RxMessage->ExtId = 0x00;
  RxMessage->IDE = CAN_ID_STD;
  RxMessage->DLC = 0;
  RxMessage->FMI = 0;
  for (i = 0;i < 8;i++)
  {
    RxMessage->Data[i] = 0x00;
  }
}

/*Configuring the CAN Filter:*/
void CAN_Config_Filter(void){

	uint32_t filter_id = 0x00000000;
  uint32_t filter_mask = 0xFFFFFF10;
	//uint32_t filter_mask = 0xFFFFFF00;  //256
	CAN_FilterTypeDef filter;

  filter.FilterIdHigh = ((filter_id << 5)  | (filter_id >> (32 - 5))) & 0xFFFF; // STID[10:0] & EXTID[17:13]
  filter.FilterIdLow = (filter_id >> (11 - 3)) & 0xFFF8; // EXID[12:5] & 3 Reserved bits
  filter.FilterMaskIdHigh = ((filter_mask << 5)  | (filter_mask >> (32 - 5))) & 0xFFFF;
  filter.FilterMaskIdLow = (filter_mask >> (11 - 3)) & 0xFFF8;
  filter.FilterFIFOAssignment = 0;
  filter.FilterBank = 0;
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  filter.FilterScale = CAN_FILTERSCALE_32BIT;
  filter.FilterActivation = ENABLE;
  filter.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&hcan, &filter) != HAL_OK)  // RETORNA O STATUS DA FUNCAO
	{
	  Error_Handler();
	}
}

void CAN_Config_Frames(void){
	static CAN_TxHeaderTypeDef        TxMessage; // Struct de definicao da estrutura da mensagem CAN Tx
	static CAN_RxHeaderTypeDef        RxMessage;

	initRxMes(&RxMessage);

	hcan.pTxMsg = &TxMessage;
	hcan.pRxMsg = &RxMessage;

	/*Configure Transmission process:*/

	// CONFIGURA A STRUCT TxMessage
	hcan.pTxMsg->StdId = 0x300; //Specifies the standard identifier
	hcan.pTxMsg->ExtId = 0x01; //Specifies the extended identifier.
	hcan.pTxMsg->RTR = CAN_RTR_DATA; //FRAME DE DADO|FRAME REMOTA / Specifies the type of frame for the message that will be transmitted.
	hcan.pTxMsg->IDE = CAN_ID_STD;//STANDARD ID 11b|EXTENDED ID 29b /Specifies the type of identifier for the message that will be transmitted.
	hcan.pTxMsg->DLC = 8; //Specifies the length of the frame that will be transmitted.
}

/*Start the Reception process and enable reception interrupt*/
void CAN_Receive_IT(void){
	if (HAL_CAN_Receive_IT(&hcan, CAN_RX_FIFO0) != HAL_OK)
	{
	  /* Reception Error */
	  Error_Handler();
	}
}

void CAN_Transmit(uint8_t *vet, uint32_t id){

	hcan.pTxMsg->StdId = id; //Specifies the standard identifier
	for(uint8_t i=0; i < hcan.pTxMsg->DLC; i++)
		hcan.pTxMsg->Data[i] = vet[i];

	/*Start the Transmission process:*/
	HAL_StatusTypeDef trans_status = HAL_CAN_Transmit_IT(&hcan);

	//Error handler
	if (trans_status != HAL_OK)
	{
		//Error_Handler();
	}

	DWT_Delay_us(7000);

}

/**
  * @brief  Initializes a Rx Message.
  * @param  CanRxMsgTypeDef *RxMessage
  * @retval None
  */



//void HAL_CAN_TxCpltCallback(CAN_HandleTypeDef * hcan){}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef * hcan)
{
	/* For CAN Rx frames received in FIFO number 0 */
  __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_FOV0);
  HAL_CAN_Receive_IT(hcan, CAN_RX_FIFO0);
  __HAL_CAN_FIFO_RELEASE(hcan, CAN_RX_FIFO0);

	/* For CAN Rx frames received in FIFO number 1 */
  //__HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_FOV1);
  //__HAL_CAN_Receive_IT(hcan, CAN_FIFO1);
	__HAL_CAN_RESET_HANDLE_STATE(hcan);
	__HAL_CAN_ENABLE_IT(hcan, CAN_IT_ERROR_WARNING | CAN_IT_ERROR_PASSIVE | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE | CAN_IT_ERROR | CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO0_OVERRUN | CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_RX_FIFO1_OVERRUN | CAN_IT_TX_MAILBOX_EMPTY);
	__HAL_UNLOCK(hcan);
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
