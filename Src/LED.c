#include "LED.h"
#include "defines.h"
#include "main.h"
#include "gpio.h"
#include "usart.h"

uint32_t blink_time = 0;
uint16_t mask = ERR_BALANCE, flag = 0;
uint8_t state_1, state_2;



void LED_init(){
	blink_time = 0;
	LED_write(LED_BLUE);
	HAL_Delay(2000);
	LED_write(LED_RED);
	HAL_Delay(1000);
	LED_write(LED_GREEN + LED_RED);
	HAL_Delay(1000);
	LED_write(LED_GREEN);
	HAL_Delay(1000);
}


void LED_get_state(uint16_t mask){

	if((mask & ERR_OVER_VOLTAGE) == ERR_OVER_VOLTAGE){

		state_1 = LED_RED;
		state_2 = 0;

	}
	if((mask & ERR_UNDER_VOLTAGE) == ERR_UNDER_VOLTAGE){

		state_1 = LED_RED;
		state_2 = LED_BLUE;

	}
	if((mask & ERR_OVER_CURRENT) == ERR_OVER_CURRENT){

		state_1 = LED_RED + LED_GREEN + LED_BLUE;
		state_2 = 0;

	}
	if((mask & ERR_OVER_TEMPERATURE) == ERR_OVER_TEMPERATURE){

		state_1 = LED_RED + LED_GREEN;
		state_2 = 0;

	}
	if((mask & ERR_OPEN_FUSES) == ERR_OPEN_FUSES){

		state_1 = LED_PURPLE;
		state_2 = 0;

	}
	if((mask & ERR_COMM_ERROR) == ERR_COMM_ERROR){

		state_1 = LED_BLUE;
		state_2 = 0;

	}
	if((mask & ERR_AIR_ERROR) == ERR_AIR_ERROR){

		state_1 = LED_CYAN;
		state_2 = 0;

	}
	if((mask & ERR_GLV_VOLTAGE) == ERR_GLV_VOLTAGE){

		state_1 = LED_PURPLE;
		state_2 = LED_GREEN;

	}
	if((mask & ERR_BALANCE) == ERR_BALANCE){

		state_1 = LED_RED;
		state_2 = LED_GREEN;

	}

}

void LED_error(uint16_t ERROR_FLAG){

	flag = ERROR_FLAG;
	if(ERROR_FLAG == ERR_NO_ERROR){
		state_1 = LED_GREEN;
		state_2 = LED_BLUE;
	}else{
		flag = ERROR_FLAG;
	}
}

void LED_write(uint8_t state){

	if((state & LED_RED) == LED_RED)
		HAL_GPIO_WritePin(ERR_LED_RED_GPIO_Port, ERR_LED_RED_Pin, 1);
	else
		HAL_GPIO_WritePin(ERR_LED_RED_GPIO_Port, ERR_LED_RED_Pin, 0);

	if((state & LED_GREEN) == LED_GREEN)
		HAL_GPIO_WritePin(ERR_LED_GREEN_GPIO_Port, ERR_LED_GREEN_Pin, 1);
	else
		HAL_GPIO_WritePin(ERR_LED_GREEN_GPIO_Port, ERR_LED_GREEN_Pin, 0);

	if((state & LED_BLUE) == LED_BLUE)
		HAL_GPIO_WritePin(ERR_LED_BLUE_GPIO_Port, ERR_LED_BLUE_Pin, 1);
	else
		HAL_GPIO_WritePin(ERR_LED_BLUE_GPIO_Port, ERR_LED_BLUE_Pin, 0);

}
