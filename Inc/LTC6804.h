/****************************************************************/
/**					Team Formula Tesla UFMG - 2019				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Rodolfo, Heuller, Wellington				*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/

#ifndef LTC_2_H
#define LTC_2_H

#include <stdarg.h>
#include "dwt_stm32_delay.h"
#include "defines.h"



typedef struct LTC_command{

	uint16_t NAME;
	uint8_t BROADCAST;


	//COMMAND SETTINGS
	uint16_t MD;		// set the ADC mode
	uint16_t DCP;		// set if discharge is permitted during discharge
	uint16_t CH;		// set CELL channels to convert
	uint16_t CHG;		// set GPIO channels to convert
	uint16_t CHST;		// set STAT channels to convert
	uint16_t PUP;		// set if pull up or pull down is enabled in ADOW
	uint16_t ST;		// set the self test mode

}LTC_command;

typedef struct LTC_config{

	LTC_command *command;

	uint8_t ORDER:1; 	// 1 bit - set printing order, 0 = normal, 1 = lowest to highest;

	//CONFIGURATION REGISTER
	uint8_t GPIO:5; 	// 5 bits - set individual gpio modes
	uint8_t REFON:1; 	// 1 bit - set the reference configuration
	uint8_t SWTRD:1;	// 1 bit - set the under voltage limit
	uint8_t ADCOPT:1;	// 1 bit - set the ADC mode
	uint16_t VUV:12; 	// 12 bits - set the under voltage limit
	uint16_t VOV; 		// 12 bits - set the over  voltage limit
	uint8_t DCTO:4; 	// 4 bits - set the duration of discharge
	uint8_t ADC_READY;

}LTC_config;


typedef struct LTC_sensor{

	uint8_t ADDR;
	uint8_t V_ERROR[12];
	uint8_t T_ERROR[5];

	//CELL REGISTERS A to D
	uint16_t CxV[12]; 	// 12 * 16 bits - get CELL voltages

	//AUXILIARY REGISTERS A & B
	uint16_t GxV[5]; 	// 5 * 16 bits - get GPIO voltages
	uint16_t REF;		// 16 bits - get the second reference voltage

	//STATUS REGISTER A & B
	uint16_t SOC; 		// 16 bits - get the sum of voltages
	uint16_t ITMP; 		// 16 bits - get the internal temperature
	uint16_t VA; 		// 16 bits - get the analog voltage
	uint16_t VD; 		// 16 bits - get the digital voltage
	uint16_t DCC;  		// 12 bits - set which cell to discharge

	uint16_t V_MAX;
	uint16_t V_MIN;
	uint16_t V_DELTA;
	uint16_t CHARGE[12];
	uint16_t TOTAL_CHARGE;


}LTC_sensor;

//typedef enum{
//
//	//GPIO set individual GPIO modes:
//	//Write: 0 -> GPIOx Pin Pull-Down ON; 1 -> GPIOx Pin Pull-Down OFF
//	//Read:  0 -> GPIOx Pin at Logic 0;   1 -> GPIOx Pin at Logic 1
//	CONFIG_GPIO  = 0b00000001,
//
//	//REFON - set the reference configuration:
//	//0 - Reference Shuts Down after Conversions
//	//1 - Reference Remains Powered Up Until Watchdog Timeout
//	CONFIG_REFON = 0b00000010,
//
//	//SWTRD:
//	//0 -> SWTEN Pin at Logic 0
//	//1 -> SWTEN Pin at Logic 1
//	CONFIG_SWTRD = 0b00000100,
//
//	//ADCOP - set the ADC configuration:
//	//0 -> Selects Modes 27kHz, 7kHz or 26Hz with MD[1:0] Bits in ADC Conversion Commands
//	//1 -> Selects Modes 14kHz, 3kHz or 2kHz with MD[1:0] Bits in ADC Conversion Commands.
//	CONFIG_ADCOP = 0b00001000,
//
//	//VUV - Undervoltage Comparison Voltage:
//	//Comparison voltage = (VUV + 1) � 16 � 100�V
//	CONFIG_VUV 	 = 0b00010000,
//
//	//VOV - Overvoltage Comparison Voltage:
//	//Comparison voltage = (VOV + 1) � 16 � 100�V
//	CONFIG_VOV 	 = 0b00100000,
//
//	//DCC - Discharge Cell x:
//	//1 -> Turn ON  Shorting Switch for Cell x
//	//0 -> Turn OFF Shorting Switch for Cell x
//	CONFIG_DCC 	 = 0b01000000,
//
//	//DCTO -  Discharge TimeOut Value:
//	// Value    :	0 |  1  |  2  |	 3  |  4  |  5  |  6  |  7  |  8  |  9  |  A  |  B  |  C  |  D  |  E  |  F  |
//	// Time(Min):	0 | 0.5 |  1  |  2  |  3  |  4  |  5  |  10 |  15 |	 20 |  30 |	 40 |  60 |  75 |  90 |	120 |
//	CONFIG_DCTO  = 0b10000000
//
//}LTC_CONFIGURATIONS;


#define LTC_COMMAND_WRCFG 	0b00000000001	// Write Configuration Register Group
#define LTC_COMMAND_RDCFG 	0b00000000010	// Read Configuration Register Group
#define LTC_COMMAND_RDCVA 	0b00000000100	// Read Cell Voltage Register Group A
#define LTC_COMMAND_RDCVB 	0b00000000110	// Read Cell Voltage Register Group B
#define LTC_COMMAND_RDCVC 	0b00000001000	// Read Cell Voltage Register Group C
#define LTC_COMMAND_RDCVD 	0b00000001010	// Read Cell Voltage Register Group D
#define LTC_COMMAND_RDAUXA	0b00000001100	// Read Auxiliary Register Group A
#define LTC_COMMAND_RDAUXB	0b00000001110	// Read Auxiliary Register Group B
#define LTC_COMMAND_RDSTATA	0b00000010000	// Read Status Register Group A
#define LTC_COMMAND_RDSTATB	0b00000010010	// Read Status Register Group B
#define LTC_COMMAND_ADCV	0b01001100000	// Start Cell Voltage ADC Conversion and Poll Status
#define LTC_COMMAND_ADOW	0b01000101000	// Start Open Wire ADC Conversion and Poll Status
#define LTC_COMMAND_CVST	0b01000000111	// Start Self-Test Cell Voltage Conversion and Poll Status
#define LTC_COMMAND_ADAX	0b10001100000	// Start GPIOs ADC Conversion and Poll Status
#define LTC_COMMAND_AXST	0b10000000111	// Start Self-Test GPIOs Conversion and Poll Status
#define LTC_COMMAND_ADSTAT	0b10001101000	// Start Status group ADC Conversion and Poll Status
#define LTC_COMMAND_STATST	0b10000001111	// Start Self-Test Status group Conversion and Poll Status
#define LTC_COMMAND_ADCVAX	0b10001101111	// Start Combined Cell Voltage and GPIO1, GPIO2 Conversion and Poll Status
#define LTC_COMMAND_CLRCELL	0b11100010001	// Clear Cell Voltage Register Group
#define LTC_COMMAND_CLRAUX	0b11100010010 	// Clear Auxiliary Register Group
#define LTC_COMMAND_CLRSTAT	0b11100010011	// Clear Status Register Group
#define LTC_COMMAND_PLADC	0b11100010100	// Poll ADC Conversion Status
#define LTC_COMMAND_DIAGN	0b11100010101	// Diagnose MUX and Poll Status
#define LTC_COMMAND_WRCOMM	0b11100100001	// Write COMM Register Group   ***** NOT IMPLEMENTED
#define LTC_COMMAND_RDCOMM	0b11100100010	// Read COMM Register Group    ***** NOT IMPLEMENTED
#define LTC_COMMAND_STCOMM	0b11100100011	// Start I2C/SPI Communication ***** NOT IMPLEMENTED

typedef enum{

	LTC_READ_CELL 	= 0b0001,
	LTC_READ_GPIO  	= 0b0010,
	LTC_READ_STATUS = 0b0100,
	LTC_READ_CONFIG = 0b1000,

}LTC_READ;

typedef enum{

	MD_FAST 	= 0b0010000000,	//	27kHz or 14kHz
	MD_NORMAL 	= 0b0100000000,	//	 7kHz or  3kHz
	MD_FILTRED  = 0b0110000000,	//	 26Hz or  2kHz

}LTC_MD;

typedef enum{

	DCP_PERMITED 	 = 0b00000010000,
	DCP_NOT_PERMITED = 0b00000000000,

}LTC_DCP;

typedef enum{

	PUP_PULL_UP 	= 0b00001000000,
	PUP_PULL_DOWN 	= 0b00000000000,

}LTC_PUP;

typedef enum{

	ST_01 	= 0b00000100000,
	ST_02 	= 0b00001000000,

}LTC_ST;

typedef enum{

	CH_ALL	= 0b00000000000,
	CH_1_7 	= 0b00000000001,
	CH_2_8  = 0b00000000010,
	CH_3_9  = 0b00000000011,
	CH_4_10 = 0b00000000100,
	CH_5_11 = 0b00000000101,
	CH_6_12	= 0b00000000110,

}LTC_CH;

typedef enum{

	CHG_ALL		= 0b00000000000,
	CHG_GPIO1	= 0b00000000001,
	CHG_GPIO2	= 0b00000000010,
	CHG_GPIO3 	= 0b00000000011,
	CHG_GPIO4  	= 0b00000000100,
	CHG_GPIO5 	= 0b00000000101,
	CHG_2REF	= 0b00000000110,

}LTC_CHG;

typedef enum{

	CHST_ALL 	= 0b00000000000,
	CHST_SOC 	= 0b00000000001,
	CHST_ITMP 	= 0b00000000010,
	CHST_VA 	= 0b00000000011,
	CHST_VD		= 0b00000000100,

}LTC_CHST;


void LTC_init(LTC_config *config);
void LTC_balance_test(LTC_config *config, LTC_sensor *sensor);
void LTC_sort(LTC_sensor *sensor, uint8_t left, uint8_t right);
void LTC_send_command(LTC_config *config, ...);
void LTC_read(uint8_t LTC_READ, LTC_config *config, LTC_sensor *sensor);
void LTC_set_balance_flag(LTC_config *config, LTC_sensor *sensor);
void LTC_reset_balance_flag(LTC_config *config, LTC_sensor *sensor);
void LTC_balance(LTC_config *config, LTC_sensor *sensor);
void LTC_open_wire(uint8_t LTC_READ, LTC_config *config, LTC_sensor *sensor);


#endif
