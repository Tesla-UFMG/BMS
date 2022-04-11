/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define __DLC__ 8
#define DEBUG_Pin GPIO_PIN_13
#define DEBUG_GPIO_Port GPIOC
#define OSCI_Pin GPIO_PIN_0
#define OSCI_GPIO_Port GPIOD
#define OSCO_Pin GPIO_PIN_1
#define OSCO_GPIO_Port GPIOD
#define CURRENT2_200_Pin GPIO_PIN_0
#define CURRENT2_200_GPIO_Port GPIOA
#define CURRENT2_25_Pin GPIO_PIN_1
#define CURRENT2_25_GPIO_Port GPIOA
#define CURRENT1_200_Pin GPIO_PIN_2
#define CURRENT1_200_GPIO_Port GPIOA
#define CURRENT1_25_Pin GPIO_PIN_3
#define CURRENT1_25_GPIO_Port GPIOA
#define ISOSPI_CS_Pin GPIO_PIN_4
#define ISOSPI_CS_GPIO_Port GPIOA
#define ISOSPI_SCK_Pin GPIO_PIN_5
#define ISOSPI_SCK_GPIO_Port GPIOA
#define ISOSPI_MISO_Pin GPIO_PIN_6
#define ISOSPI_MISO_GPIO_Port GPIOA
#define ISOSPI_MOSI_Pin GPIO_PIN_7
#define ISOSPI_MOSI_GPIO_Port GPIOA
#define CURRENT3_200_Pin GPIO_PIN_0
#define CURRENT3_200_GPIO_Port GPIOB
#define CURRENT3_25_Pin GPIO_PIN_1
#define CURRENT3_25_GPIO_Port GPIOB
#define MODE_SELECT_Pin GPIO_PIN_8
#define MODE_SELECT_GPIO_Port GPIOA
#define MODE_SELECT_EXTI_IRQn EXTI9_5_IRQn
#define FLAG_RESET_Pin GPIO_PIN_10
#define FLAG_RESET_GPIO_Port GPIOA
#define FLAG_RESET_EXTI_IRQn EXTI15_10_IRQn
#define AIR_ENABLE_Pin GPIO_PIN_3
#define AIR_ENABLE_GPIO_Port GPIOB
#define CHARGE_ENABLE_Pin GPIO_PIN_4
#define CHARGE_ENABLE_GPIO_Port GPIOB
#define ERR_LED_Pin GPIO_PIN_7
#define ERR_LED_GPIO_Port GPIOB
#define AIR_AUX_PLUS_Pin GPIO_PIN_6
#define AIR_AUX_PLUS_GPIO_Port GPIOB
#define AIR_AUX_MINUS_Pin GPIO_PIN_5
#define AIR_AUX_MINUS_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
