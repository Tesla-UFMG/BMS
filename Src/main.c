/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

//static float CURRENT_ZERO[4] = {2250, 2990, 2396, 2396};
//static const float CURRENT_GAIN[4] = {1.22, 1.52, 1.22, 1.22};
//static float CURRENT_ZERO[4] = {2223, 2750, 2223, 2227};
static const float CURRENT_GAIN[4] = {1.22, 1.51, 1.22, 1.22};
ErrorStatus  HSEStartUpStatus;
HAL_StatusTypeDef FlashStatus;

BMS_struct* BMS;
int32_t ADC_BUF[5];
uint32_t adc_time;
uint8_t mode_button = 0, debounce_flag, accept_flag, accept_time, debounce_time, mode;
uint16_t VirtAddVarTab[NumbOfVar] = {0x5555, 0x6666, 0x7777};
char LED_DEBUG = 1;

extern DMA_HandleTypeDef hdma_usart3_rx;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define FILTER_GAIN 255

int aux = 0;
int initialReadings = 0;
float CURRENT_ZERO[N_OF_DHAB];

float filter(float old, float new){
	return (FILTER_GAIN * old + new) / (FILTER_GAIN + 1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef * hadc)
{
	if(initialReadings < 5){
		for(uint8_t i = 0; i < N_OF_DHAB; i++){
			CURRENT_ZERO[i] += ((float)ADC_BUF[i] * (float)CURRENT_GAIN[i]);
			initialReadings++;
			if(initialReadings == 5){
				for(uint8_t j = 0; j < N_OF_DHAB; j++)
					CURRENT_ZERO[j] = CURRENT_ZERO[j]/5;
				}
		}
	}
	else{
		for(uint8_t i = 0; i < N_OF_DHAB; i++){
		BMS->c_adc[i] = filter((float)BMS->c_adc[i], (float)ADC_BUF[i]);
		BMS->current[i] = filter(BMS->current[i], ((float)ADC_BUF[i] * CURRENT_GAIN[i]) - CURRENT_ZERO[i]);//		BMS->current[i] = filter(BMS->current[i], (ADC_BUF[i]));
		}
	}
	BMS->v_GLV = filter(BMS->v_GLV , ((float)(ADC_BUF[4] + 400) * 4.5));
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_CAN_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

	HAL_FLASH_Unlock();
	EE_Init();

	DWT_Delay_Init();

	HAL_ADC_Start_DMA(&hadc1, (uint32_t* )ADC_BUF, 5);
	USART_DMA_Init(&huart3, &hdma_usart3_rx);

	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim4);

	BMS = (BMS_struct*) calloc(1, sizeof(BMS_struct));
	BMS_init(BMS);

	DWT_Delay_us(50000);

	HAL_GPIO_WritePin(CHARGE_ENABLE_GPIO_Port, CHARGE_ENABLE_Pin, 1);

	NexPageShow(1);

  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	while (1)
	{
		BMS_check(&LED_DEBUG);
		BMS_monitoring(BMS);
		BMS_error(BMS);
		BMS_can(BMS);
		nexLoop(BMS);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while(1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
