/****************************************************************/
/**					Team Formula Tesla UFMG - 2018				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Eric, Virginia							*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/

#include "DHAB_s125.h"
#include "stm32f1xx_hal.h"
#include "dwt_stm32_delay.h"
#include "main.h"
#include "usart.h"
#include <stdlib.h>



void DHAB_init(){

}

//uint8_t DHAB_get_error(DHAB* sensor){
//
//	uint8_t i, error_flag = 0;
//
//	for(i = 0; i < 4; i++){
//		if(sensor->current[i] > MAX_CURRENT)
//			sensor->c_error[i] = ERR_OVER_VOLTAGE;
//		else
//			sensor->c_error[i] = ERR_NO_ERROR;
//
//		error_flag |= sensor->c_error[i];
//
//	}
//	return error_flag;
//}
