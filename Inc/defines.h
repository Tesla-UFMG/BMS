/****************************************************************************
 *					Team Formula Tesla UFMG - 2017
 * File:   defines.h
 * PIC: 24FJ64GA004 Family
 * Compiler: XC16 v1.3
 * Author: Patrick Garcia (Botze), Pedro Ivan Ribeiro (Somalia)
 * Contributors: Eric Drumond
 * License: Free - Open Source
 * Created on 28 de Mar�o de 2017, 15:43
 ****************************************************************************/
#ifndef DEFINES_H
#define DEFINES_H


//defines gerais
#define BLINK_TIME					500
#define BLINK_ERROR_TIME			200
#define MAX_VOLTAGE					96
#define OP_VOLTAGE					92
#define ST_VOLTAGE					86
#define MIN_V_CELL					1000
#define MAX_V_CELL					1000
#define N_OF_PACKS					6
#define N_OF_SLAVES					9
#define N_OF_CELLS					12
#define N_OF_THERMISTORS			5
#define PACKS_IN_SERIES				2
#define MAX_CELL_V_DISCHARGE		36000//36500
#define MAX_CELL_V_CHARGE			36000
#define MAX_CELL_V_STORE			33000/
#define MIN_CELL_V					26000//27500
#define MIN_CELL_V_BALLANCE 		23000
#define RESET_V_MAX					0
#define RESET_V_MIN					36000
#define FUSE_LIMIT					1
#define V_ERROR_COM					65528
#define MAX_TEMPERATURE				500
#define T_ERROR_COM					1
#define RESET_T_MAX					0
#define BALANCE_TIME				120
#define BALANCE_V_ERROR				5

#define READ  0
#define WRITE 1


//BMS OPERATION MODES:
#define OPP_DEFAULT 	0
#define OPP_CHARGING 	1
#define OPP_BALANCING 	2
#define OPP_TESTING 	3

#define BMS_MONITORING 	0
#define BMS_CHARGING 	0b01
#define BMS_BALANCING 	0b10

//BMS COMMUNICATION MODES:
#define COMM_TC_ONLY 	0
#define COMM_UART_DEBUG	1
#define COMM_FULL		2

#define UART_RATE			500 //ms
#define CAN_RATE			3000 //ms
#define CAN_TEMPERATURE_ID  3
#define CAN_ERROR_FLAG_ID  	4
#define CAN_CURRENT_ID		6
#define CAN_GENERAL_ID		7

//ERROR DEFINES:
#define ERR_NO_ERROR 			0b000000000
#define ERR_OVER_VOLTAGE		0b000000001
#define ERR_UNDER_VOLTAGE		0b000000010
#define ERR_OVER_CURRENT		0b000000100
#define ERR_OVER_TEMPERATURE	0b000001000
#define ERR_OPEN_FUSES			0b000010000
#define ERR_COMM_ERROR			0b000100000
#define ERR_AIR_ERROR			0b001000000
#define ERR_GLV_VOLTAGE			0b010000000
#define ERR_BALANCE				0b100000000


//LED DEFINES:
#define LED_BLACK			0
#define LED_RED				0b001
#define LED_GREEN			0b010
#define LED_BLUE			0b100
#define LED_PURPLE			LED_RED + LED_BLUE
#define LED_YELLOW			LED_RED + LED_GREEN
#define LED_CYAN			LED_BLUE + LED_GREEN
#define LED_WHITE			LED_RED + LED_GREEN + LED_BLUE

//AIR DEFINES:
#define AIR_OPEN			0b00
#define AIR_CLOSED			0b01
#define AIR_ERROR			0b10

//defines logicos
#define FALSE 	0
#define TRUE 	1
#define OUTPUT 	0
#define INPUT 	1

//DEFINES SENSOR DE TENS�O:

#define READ_VOLTAGES 0
#define READ_TEMPERATURES 2

#define STAT_OPPERATING	0b00000000
#define STAT_OFFLINE	0b00100000

#define MAX_RETRIES	100
#define MIN_V_DELTA 100

#define T_WAKEUP 	400
#define T_REFUP_V 	20
#define T_ADC		3000

#define B_START 	1
#define B_PAUSE		0

#define SEED_PEC	16

//GLOBAL COMMANDS:
#define DUMMY 					0
#define COMMAND_ADCV			1		//COMANDO PARA MANDAR O LTC6804 CONVERTER AS TENS�ES
#define COMMAND_ADOW_ALL_PUP_0	2		//COMANDO PARA MANDAR O LTC6804 CONVERTER AS TENS�ES EM MODO PULL-DOWN
#define COMMAND_ADOW_ALL_PUP_1	3		//COMANDO PARA MANDAR O LTC6804 CONVERTER AS TENS�ES EM MODO PULL-UP
#define COMMAND_ADAX_ALL		4		//COMANDO PARA MANDAR O LTC6804 CONVERTER OS ADC's DOS TERMISTORES
#define COMMAND_CLEAR_V			5		//COMANDO PARA MANDAR O LTC6804 LIMPAR OS REGISTRADORES DE TENSAO
#define COMMAND_CLEAR_T			6		//COMANDO PARA MANDAR O LTC6804 LIMPAR OS REGISTRADORES DE TEMPERATURA


//ADDRESSED COMMANDS:
#define COMMAND_READ_CONFIG		6
#define COMMAND_WRITE_CONFIG	7
//------//

//SENSOR DE CORRENTE
#define N_OF_DHAB		4
#define MAX_CURRENT 	800 //A
#define ITERATIONS 		10
#define ZERO_CURRENT0	466 //492
#define ZERO_CURRENT1 	463
#define ZERO_CURRENT2 	465 //491
#define ZERO_CURRENT3 	464
#define SENSOR_GAIN0 	0.4651
#define SENSOR_GAIN1 	0.058823
#define SENSOR_GAIN2 	0.4651
#define SENSOR_GAIN3 	0.058823

//PORTS DEFINES
//#define __DLC__ 8
//#define DEBUG_Pin GPIO_PIN_13
//#define DEBUG_GPIO_Port GPIOC
//#define OSCI_Pin GPIO_PIN_0
//#define OSCI_GPIO_Port GPIOD
//#define OSCO_Pin GPIO_PIN_1
//#define OSCO_GPIO_Port GPIOD
//#define CURRENT2_200_Pin GPIO_PIN_0
//#define CURRENT2_200_GPIO_Port GPIOA
//#define CURRENT2_25_Pin GPIO_PIN_1
//#define CURRENT2_25_GPIO_Port GPIOA
//#define CURRENT1_200_Pin GPIO_PIN_2
//#define CURRENT1_200_GPIO_Port GPIOA
//#define CURRENT1_25_Pin GPIO_PIN_3
//#define CURRENT1_25_GPIO_Port GPIOA
//#define ISOSPI_CS_Pin GPIO_PIN_4
//#define ISOSPI_CS_GPIO_Port GPIOA
//#define ISOSPI_SCK_Pin GPIO_PIN_5
//#define ISOSPI_SCK_GPIO_Port GPIOA
//#define ISOSPI_MISO_Pin GPIO_PIN_6
//#define ISOSPI_MISO_GPIO_Port GPIOA
//#define ISOSPI_MOSI_Pin GPIO_PIN_7
//#define ISOSPI_MOSI_GPIO_Port GPIOA
//#define GLV_SAMPLE_Pin GPIO_PIN_1
//#define GLV_SAMPLE_GPIO_Port GPIOB
//#define MODE_SELECT_Pin GPIO_PIN_8
//#define MODE_SELECT_GPIO_Port GPIOA
//#define MODE_SELECT_EXTI_IRQn EXTI9_5_IRQn
//#define FLAG_RESET_Pin GPIO_PIN_10
//#define FLAG_RESET_GPIO_Port GPIOA
//#define FLAG_RESET_EXTI_IRQn EXTI15_10_IRQn
//#define AIR_ENABLE_Pin GPIO_PIN_3
//#define AIR_ENABLE_GPIO_Port GPIOB
//#define CHARGE_ENABLE_Pin GPIO_PIN_4
//#define CHARGE_ENABLE_GPIO_Port GPIOB
//#define ERR_LED_Pin GPIO_PIN_7
//#define ERR_LED_GPIO_Port GPIOB
//#define AIR_AUX_PLUS_Pin GPIO_PIN_8
//#define AIR_AUX_PLUS_GPIO_Port GPIOB
//#define AIR_AUX_MINUS_Pin GPIO_PIN_9
//#define AIR_AUX_MINUS_GPIO_Port GPIOB

#endif
