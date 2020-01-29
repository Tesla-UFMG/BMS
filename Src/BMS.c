/****************************************************************/
/**					Team Formula Tesla UFMG - 2018				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Eric, Virginia							*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/

#include "BMS.h"
#include "LED.h"
#include "stm32f1xx_hal.h"
#include "usart.h"
#include "dwt_stm32_delay.h"
#include "can.h"
#include "eeprom.h"
#include <stdlib.h>

static uint32_t comm_time;
static int8_t UV_retries, OV_retries, OT_retries;

uint8_t balance_enable = 1;

//extern uint8_t mode_button;
//extern I2C_HandleTypeDef hi2c2;


static const uint16_t CAN_ID_TABLE[6][6] = {
		{
				0x600,		//PACK 0 V_GROUP 0 		(cells 0, 1, 2, 3)
				0x601,		//PACK 0 V_GROUP 1		(cells 4, 5, 6, 7)
				0x602,		//PACK 0 V_GROUP 2      (cells 8, 9, 10, 11)
				0x603,		//PACK 0 T_GROUP        (sensor 0, 1, 2, 3)
				0x604,		//PACK 0 ERROR_FLAG 1	(cell_flags 0, 1, 2, 3, 4, 5, 6, 7)
				0x605		//PACK 0 ERROR_FLAG 2	(cell_flags 8, 9, 10, 11)
		},
		{
				0x610,		//PACK 1 V_GROUP 0 		(cells 0, 1, 2, 3)
				0x611,		//PACK 1 V_GROUP 1      (cells 4, 5, 6, 7)
				0x612,		//PACK 1 V_GROUP 2      (cells 8, 9, 10, 11)
				0x613,		//PACK 1 T_GROUP        (sensor 0, 1, 2, 3)
				0x614,		//PACK 1 ERROR_FLAG 1	(cell_flags 0, 1, 2, 3, 4, 5, 6, 7)
				0x615		//PACK 1 ERROR_FLAG 2   (cell_flags 8, 9, 10, 11)
		},
		{
				0x620,		//PACK 2 V_GROUP 0 		(cells 0, 1, 2, 3)
				0x621,		//PACK 2 V_GROUP 1      (cells 4, 5, 6, 7)
				0x622,		//PACK 2 V_GROUP 2      (cells 8, 9, 10, 11)
				0x623,		//PACK 2 T_GROUP        (sensor 0, 1, 2, 3)
				0x624,		//PACK 2 ERROR_FLAG 1	(cell_flags 0, 1, 2, 3, 4, 5, 6, 7)
				0x625		//PACK 2 ERROR_FLAG 2   (cell_flags 8, 9, 10, 11)
		},
		{
				0x630,		//PACK 3 V_GROUP 0      (cells 0, 1, 2, 3)
				0x631,		//PACK 3 V_GROUP 1      (cells 4, 5, 6, 7)
				0x632,		//PACK 3 V_GROUP 2      (cells 8, 9, 10, 11)
				0x633,		//PACK 3 T_GROUP        (sensor 0, 1, 2, 3)
				0x634,		//PACK 3 ERROR_FLAG 1	(cell_flags 0, 1, 2, 3, 4, 5, 6, 7)
				0x635		//PACK 3 ERROR_FLAG 2   (cell_flags 8, 9, 10, 11)
		},
		{
				0x640,		//CHARGING CURRENT (bytes 0, 1, 2, 3) AND DISCHARGING CURRENT(bytes 4, 5, 6, 7)
		},
		{
				0x650		//BMS MODE(bytes 0),COMM MODE(bytes 1), AIR STATUS(bytes 2 & 3), BATTERY %(bytes 4 & 5), GLOBAL ERROR FLAG(byte 6)

		},
};

extern uint8_t c_zero_flag;

void BMS_init(BMS_struct *BMS){
	comm_time = HAL_GetTick();

	CAN_ConfigFilter();
	CAN_ConfigFrames();
	CAN_Receive();

	BMS->error_flag = ERR_NO_ERROR;
	BMS->opperating_packs = 0;
	BMS->lowest_cell = 0;

	for(int i = 0; i < N_OF_PACKS; i++){
		BMS->sensor_v[i] = (LTC6804*) malloc(sizeof(LTC6804));
		LTC6804_init(BMS->sensor_v[i], i);
	}
	LTC6804_set_thermistor_zeros();

	c_zero_flag = 1;

	if(BMS->opperation_mode == OPP_CHARGING)
		HAL_GPIO_WritePin(CHARGE_ENABLE_GPIO_Port, CHARGE_ENABLE_Pin, SET);


//	EE_WriteVariable(0x0, (uint16_t) 0);
//	EE_WriteVariable(0x1, (uint16_t) 0);

	uint16_t aux;
	EE_ReadVariable(0x0, &aux);
	BMS->charge = ((uint32_t)aux) << 16; 	//load upper bytes
	EE_ReadVariable(0x1, &aux);
	BMS->charge += aux;		//load lower bytes

}

void BMS_monitoring(BMS_struct *BMS){
	uint8_t i, opperating_packs = 0;


	LTC6804_convert_all();

	for(i = 0; i < N_OF_PACKS; i++){

		//UART_print("\n%d", BMS->sensor_v[i]->status);
		if(LTC6804_check_status(BMS->sensor_v[i])){
			opperating_packs++;
		}

		if (BMS->sensor_v[i]->status == STAT_OPPERATING) {
			//VOLTAGES
			BMS->error_flag |= LTC6804_read_registers(BMS->sensor_v[i]->v_cell, BMS->sensor_v[i]->address, READ_VOLTAGES);
			//TEMPERATURES
			BMS->error_flag |= LTC6804_read_registers(BMS->sensor_v[i]->t_cell, BMS->sensor_v[i]->address, READ_TEMPERATURES);
			//FUSES

		}
	}


//	if (BMS->error_flag != ERR_NO_ERROR) {
//		LTC6804_convert_fuses();
//
//		for(i = 0; i < N_OF_PACKS; i++){
//
//			if (BMS->sensor_v[i]->status == STAT_OPPERATING) {
//
//				if(LTC6804_check_fuses(BMS->sensor_v[i]) == ERR_OPEN_FUSES)
//					BMS->error_flag |= ERR_OPEN_FUSES;
//				//			else
//				//				BMS->error_flag &= ~ERR_OPEN_FUSES;
//			}
//
//		}
//	}

	BMS->opperating_packs = opperating_packs;

	BMS->AIR = BMS_AIR_status(BMS);
}

uint8_t BMS_AIR_status(BMS_struct *BMS){
	HAL_GPIO_WritePin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin, RESET);
	if (HAL_GPIO_ReadPin(AIR_AUX_MINUS_GPIO_Port, AIR_AUX_MINUS_Pin) == 1)
		return AIR_ERROR;

	HAL_GPIO_WritePin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin, SET);
	if (HAL_GPIO_ReadPin(AIR_AUX_MINUS_GPIO_Port, AIR_AUX_MINUS_Pin) == 1)
		return AIR_CLOSED;
	else
		return AIR_OPEN;
}

int BMS_charging(BMS_struct BMS){
	//EM CONSTRU플O...
	return 0;
}

int BMS_discharging(BMS_struct BMS){
	//EM CONSTRU플O...
	return 0;
}

int BMS_balance(BMS_struct *BMS){
	for(uint8_t i = 0; i < N_OF_PACKS; i++){
		if(LTC6804_check_status(BMS->sensor_v[i])){
			LTC6804_balance(BMS->sensor_v[i]);
		}
	}
	//EM CONSTRU플O...
	return 0;
}

int BMS_communication(BMS_struct *BMS){
	//EM CONSTRU플O...


	UART_print("\n\n%d\n\n", BMS->charge);
	EE_WriteVariable(0x0, (uint16_t) (BMS->charge >> 16));
	EE_WriteVariable(0x1, (uint16_t) BMS->charge);

	switch(BMS->communication_mode){
	case COMM_TC_ONLY:
		BMS_can(BMS);
		break;

	case COMM_FULL:

		BMS_can(BMS);
		if ((HAL_GetTick() - comm_time) > UART_RATE){
			BMS_uart(BMS);
			comm_time = HAL_GetTick();
		}
		break;

	case COMM_UART_DEBUG:

		if ((HAL_GetTick() - comm_time) > UART_RATE){
			BMS_uart(BMS);
			comm_time = HAL_GetTick();
		}

		break;
	}
	return 0;
}

void BMS_error(BMS_struct *BMS){

	uint16_t flag = 0;


	for(int i = 0; i < N_OF_PACKS; i++){
		if (BMS->sensor_v[i]->status == STAT_OPPERATING) {

			flag |= LTC6804_get_error(BMS->sensor_v[i]) &~ ERR_BALANCE;


			//BMS->error_flag &= ~(ERR_UNDER_VOLTAGE | ERR_OVER_VOLTAGE | ERR_OVER_TEMPERATURE);

		}
	}


	if((flag & ERR_UNDER_VOLTAGE) == ERR_UNDER_VOLTAGE)
		UV_retries++;
	else
		UV_retries--;
	if((flag & ERR_OVER_VOLTAGE) == ERR_OVER_VOLTAGE)
		OV_retries++;
	else
		OV_retries--;
	if((flag & ERR_OVER_TEMPERATURE) == ERR_OVER_TEMPERATURE)
		OT_retries++;
	else
		OT_retries--;

	if(UV_retries > 5) UV_retries = 5;
	if(OV_retries > 5) OV_retries = 5;
	if(OT_retries > 5) OT_retries = 5;
	if(UV_retries < 0) UV_retries = 0;
	if(OV_retries < 0) OV_retries = 0;
	if(OT_retries < 0) OT_retries = 0;


	if(UV_retries == 5)
		BMS->error_flag |= ERR_UNDER_VOLTAGE;
	else if(UV_retries == 0)
		BMS->error_flag &= ~ERR_UNDER_VOLTAGE;
	if(OV_retries == 5)
		BMS->error_flag |= ERR_OVER_VOLTAGE;
	else if(OV_retries == 0)
		BMS->error_flag &= ~ERR_OVER_VOLTAGE;
	if(OT_retries == 5)
		BMS->error_flag |= ERR_OVER_TEMPERATURE;
	else if(OT_retries == 0)
		BMS->error_flag &= ~ERR_OVER_TEMPERATURE;

	if(BMS->opperating_packs < N_OF_PACKS)
		BMS->error_flag |= ERR_COMM_ERROR;  //SET FLAG
//	else
//	BMS->error_flag &= ~ERR_COMM_ERROR; //RESET FLAG

//	switch(BMS->opperation_mode){
//	case OPP_DEFAULT:
//		if(BMS->current[0] > 1800 || BMS->current[2] > 1800)
//			BMS->error_flag |= ERR_OVER_CURRENT;
//		break;
//
//	case OPP_CHARGING:
//		if(BMS->current[1] > 2000 || BMS->current[3] > 2000)
//			BMS->error_flag |= ERR_OVER_CURRENT;
//		break;
//	}


//	if(BMS->GLV_voltage < 13500)
//		BMS->error_flag |= ERR_GLV_VOLTAGE;



	HAL_GPIO_WritePin(AIR_ENABLE_GPIO_Port, AIR_ENABLE_Pin, SET);
	if(BMS->opperation_mode == OPP_CHARGING){
		HAL_GPIO_WritePin(CHARGE_ENABLE_GPIO_Port, CHARGE_ENABLE_Pin, SET);
		BMS->error_flag &= ~ERR_OVER_TEMPERATURE;
	}


	BMS->error_flag &= ~ERR_OPEN_FUSES;


	if(BMS->error_flag != ERR_NO_ERROR){
		if(BMS->opperation_mode == OPP_CHARGING){
			HAL_GPIO_WritePin(CHARGE_ENABLE_GPIO_Port, CHARGE_ENABLE_Pin, RESET);
		}
		else{
			HAL_GPIO_WritePin(AIR_ENABLE_GPIO_Port, AIR_ENABLE_Pin, RESET);
			if (BMS_AIR_status(BMS) ==  AIR_CLOSED){
				BMS->error_flag = ERR_AIR_ERROR;
			}
		}

		//Envia por can pro ECU e espera ele desligar os motores;
		//abre air}
	}

	LED_error(BMS->error_flag);
	if(BMS->opperation_mode == OPP_DEFAULT){
		if(BMS->error_flag != ERR_NO_ERROR)
			LED_write(LED_RED);
		else LED_write(LED_BLACK);
	}else{
		if(BMS->error_flag != ERR_NO_ERROR)
			LED_write(LED_RED);
		else LED_write(LED_BLUE);
	}


}

void BMS_can(BMS_struct *BMS){
	uint8_t i, j;
	uint8_t can_vector[8];

	uint32_t t_mean = 0, t_max = 0, c_mean = 0, v_min = 0;
	uint32_t v_bank = 0;

	v_min = BMS->sensor_v[0]->v_min_cell;

	for(uint8_t i = 0; i < N_OF_PACKS; i++){
		t_mean += BMS->sensor_v[i]->t_mean;

		if(BMS->sensor_v[i]->t_max > t_max)
			t_max = BMS->sensor_v[i]->t_max;

		if(BMS->sensor_v[i]->v_min_cell < v_min)
			v_min = BMS->sensor_v[i]->v_min_cell;

		v_bank += BMS->sensor_v[i]->v_sum;

	}

	c_mean = (BMS->current[0] + BMS->current[2])/200;

	v_bank = v_bank/200;
	BMS->v_bank = v_bank;

	t_mean = t_mean/4;

	for(i = 0; i < N_OF_PACKS; i++){
		if (BMS->sensor_v[i]->status == STAT_OPPERATING) {
			for (j = 0; j < 3; j++) {
				can_vector[0] = BMS->sensor_v[i]->v_cell[(j * 4)    ];
				can_vector[1] = BMS->sensor_v[i]->v_cell[(j * 4)    ] >> 8;
				can_vector[2] = BMS->sensor_v[i]->v_cell[(j * 4) + 1];
				can_vector[3] = BMS->sensor_v[i]->v_cell[(j * 4) + 1] >> 8;
				can_vector[4] = BMS->sensor_v[i]->v_cell[(j * 4) + 2];
				can_vector[5] = BMS->sensor_v[i]->v_cell[(j * 4) + 2] >> 8;
				can_vector[6] = BMS->sensor_v[i]->v_cell[(j * 4) + 3];
				can_vector[7] = BMS->sensor_v[i]->v_cell[(j * 4) + 3] >> 8;
				CAN_Transmit(can_vector, CAN_ID_TABLE[i][j]);
			}

			can_vector[0] = BMS->sensor_v[i]->t_cell[0];
			can_vector[1] = BMS->sensor_v[i]->t_cell[0] >> 8;
			can_vector[2] = BMS->sensor_v[i]->t_cell[1];
			can_vector[3] = BMS->sensor_v[i]->t_cell[1] >> 8;
			can_vector[4] = BMS->sensor_v[i]->t_cell[2];
			can_vector[5] = BMS->sensor_v[i]->t_cell[2] >> 8;
			can_vector[6] = BMS->sensor_v[i]->t_cell[3];
			can_vector[7] = BMS->sensor_v[i]->t_cell[3] >> 8;
			CAN_Transmit(can_vector, CAN_ID_TABLE[i][CAN_TEMPERATURE_ID]);

			can_vector[0] = BMS->sensor_v[i]->v_error[0];
			can_vector[1] = BMS->sensor_v[i]->v_error[1];
			can_vector[2] = BMS->sensor_v[i]->v_error[2];
			can_vector[3] = BMS->sensor_v[i]->v_error[3];
			can_vector[4] = BMS->sensor_v[i]->v_error[4];
			can_vector[5] = BMS->sensor_v[i]->v_error[5];
			can_vector[6] = BMS->sensor_v[i]->v_error[6];
			can_vector[7] = BMS->sensor_v[i]->v_error[7];
			CAN_Transmit(can_vector, CAN_ID_TABLE[i][CAN_ERROR_FLAG_ID]);

			can_vector[0] = BMS->sensor_v[i]->v_error[8];
			can_vector[1] = BMS->sensor_v[i]->v_error[9];
			can_vector[2] = BMS->sensor_v[i]->v_error[10];
			can_vector[3] = BMS->sensor_v[i]->v_error[11];
			can_vector[4] = BMS->sensor_v[i]->status;
			can_vector[5] = BMS->sensor_v[i]->address;
			can_vector[6] = 0;
			can_vector[7] = 0;
			CAN_Transmit(can_vector, CAN_ID_TABLE[i][CAN_ERROR_FLAG_ID + 1]);
		}
	}

	can_vector[0] = BMS->current[0];
	can_vector[1] = (int16_t)BMS->current[0] >> 8;
	can_vector[2] = BMS->current[1];
	can_vector[3] = (int16_t)BMS->current[1] >> 8;
	can_vector[4] = BMS->current[2];
	can_vector[5] = (int16_t)BMS->current[2] >> 8;
	can_vector[6] = BMS->current[3];
	can_vector[7] = (int16_t)BMS->current[3] >> 8;
	CAN_Transmit(can_vector, CAN_ID_TABLE[CAN_CURRENT_ID][0]);

	uint32_t percent = 10000 - ((BMS->charge + 50000000)/14400);

	can_vector[0] = BMS->GLV_voltage;
	can_vector[1] = BMS->GLV_voltage >> 8;
	can_vector[2] = (int16_t)(percent);
	can_vector[3] = (int16_t)(percent >> 8);
	can_vector[4] = BMS->opperation_mode;
	can_vector[5] = BMS->communication_mode;
	can_vector[6] = BMS->AIR;
	can_vector[7] = BMS->error_flag;
	CAN_Transmit(can_vector, CAN_ID_TABLE[CAN_GENERAL_ID][0]);


	can_vector[0] = c_mean;
	can_vector[1] = c_mean >> 8;
	can_vector[2] = v_bank;
	can_vector[3] = v_bank >> 8;
	can_vector[4] = t_mean;
	can_vector[5] = t_mean >> 8;
	can_vector[6] = t_max;
	can_vector[7] = t_max >> 8;
	CAN_Transmit(can_vector, 0x660);

	can_vector[0] = v_min;
	can_vector[1] = v_min >> 8;
	can_vector[2] = 0;
	can_vector[3] = 0;
	can_vector[4] = 0;
	can_vector[5] = 0;
	can_vector[6] = 0;
	can_vector[7] = 0;
	CAN_Transmit(can_vector, 0x670);

	can_vector[0] = BMS->AIR;
	can_vector[1] = 0;
	can_vector[2] = 0;
	can_vector[3] = 0;
	can_vector[4] = 0;
	can_vector[5] = 0;
	can_vector[6] = 0;
	can_vector[7] = 0;
	CAN_Transmit(can_vector, 0x0);
}

extern uint8_t max_retries;
extern uint16_t ADC_BUF[5];


void BMS_uart(BMS_struct *BMS){

	HAL_GPIO_WritePin(DEBUG_GPIO_Port,DEBUG_Pin, 1);


	UART_print("\n\n\n\n"  );//HEADER
	UART_print("\n\t\tONLINE PACKS: %d\t\tOPERATION MODE: %d\t\tCHARGE: %d\t\tERROR:", BMS->opperating_packs, BMS->opperation_mode , BMS->charge);//HEADER
	UART_print_error_flag(BMS->error_flag);
	UART_print("\t\tRETRIES:%d", max_retries);
	UART_print("\n"  );//HEADER

	for(int i = 0; i < N_OF_PACKS; i++){

		UART_print("\n");
		UART_print("\t\t\t\t\t\t\t\t   -- PACK %d STATUS: %x --\n", i, BMS->sensor_v[i]->status);
		UART_print("\n\t");

		if (BMS->sensor_v[i]->status == STAT_OPPERATING) {
			for(int j = 0; j < N_OF_CELLS; j++){
				UART_print("CELL %2d: ", j + 1);
				//UART_print("%d", BMS->sensor_v[i]->v_cell[j]);
				UART_print_float((float)BMS->sensor_v[i]->v_cell[j]/10000);
				//UART_print("%x", BMS->sensor_v[i]->v_error[j]);
				UART_print_error_flag(BMS->sensor_v[i]->v_error[j]);
				UART_print("\t");

				if ((j + 1) % 6 == 0){
					UART_print("\n\t");
				}
			}

			UART_print("\n\t");

			for(int j = 0; j < N_OF_THERMISTORS; j++){
				UART_print("TEMP %d: ", j);
				UART_print_float((float)BMS->sensor_v[i]->t_cell[j]/1000);
				UART_print_error_flag(BMS->sensor_v[i]->t_error[j]);
				UART_print("\t");
			}

			UART_print("V PACK %d: ", i);
			UART_print_float((float)BMS->sensor_v[i]->v_sum/10000);
			UART_print("\t");
			UART_print("VAR MAX %d: ", i);
			UART_print_float((float)BMS->sensor_v[i]->v_var/10000);

			UART_print("\n");
		}
	}

	UART_print("\n\tCURRENT 1(1): ");
	UART_print_float((float)BMS->current[0]/100);
	UART_print("\tCURRENT 1(2): ");
	UART_print_float((float)BMS->current[1]/1000);
	UART_print("\tCURRENT 2(1): ");
	UART_print_float((float)BMS->current[2]/100);
	UART_print("\tCURRENT 2(2): ");
	UART_print_float((float)BMS->current[3]/1000);
	UART_print("\n\tCGLV: ");
	UART_print_float((float)BMS->GLV_voltage/1000);
	UART_print("\n\t%d \t%d \t%d \t%d", ADC_BUF[0], ADC_BUF[1], ADC_BUF[2], ADC_BUF[3]);

	UART_print("\n\t[UV] times: %d", UV_retries);

	UART_print("\t[OV] times: %d", OV_retries);

	UART_print("\t[OT] times: %d", OT_retries);

	HAL_GPIO_WritePin(DEBUG_GPIO_Port,DEBUG_Pin, 0);
}

uint8_t BMS_mode_opp_navigate(uint8_t mode){
	switch(mode){
	case OPP_BALANCING:
		return OPP_CHARGING;
		break;
	case OPP_CHARGING:
		return OPP_DEFAULT;
		break;
	case OPP_DEFAULT:
		return OPP_TESTING;
		break;
	case OPP_TESTING:
		return OPP_BALANCING;
		break;
	default:
		return OPP_DEFAULT;
	}
}

uint8_t BMS_mode_comm_navigate(uint8_t mode){
	switch(mode){
	case COMM_FULL:
		return COMM_TC_ONLY;
		break;
	case COMM_TC_ONLY:
		return COMM_UART_DEBUG;
		break;
	case COMM_UART_DEBUG:
		return COMM_FULL;
		break;
	default:
		return OPP_DEFAULT;
	}
}

//uint8_t mode_display(uint8_t mode){
//	switch(mode){
//	case OPP_BALANCING:
//		return b;
//		break;
//	case OPP_CHARGING:
//		return c;
//		break;
//	case OPP_DEFAULT:
//		return d;
//		break;
//	case OPP_TESTING:
//		return t;
//		break;
//	default:
//		return d;
//	}
//}

//extern uint8_t mode, accept_time;


//void BMS_mode_selector(BMS_struct *BMS){
//	if(!HAL_GPIO_ReadPin(MODE_SELECT_GPIO_Port, MODE_SELECT_Pin)){
//		int i = 0, op = 0;
//
//		i2c_transmit(D);
//
//		while (!HAL_GPIO_ReadPin(MODE_SELECT_GPIO_Port, MODE_SELECT_Pin));
//
//		while(1){
//			//---MODE SELECTION ROUTINE---
//
//			i2c_transmit(mode_display(a));
//
//			if(mode_button == 1){
//				//---MODE BLINKING ROUTINE---
//
//				if(i >= 12) op = 1;
//				else if (i <= 0) op = 0;
//
//				i2c_transmit(mode_display(mode));
//				HAL_Delay(i);
//				i2c_transmit(0);
//				HAL_Delay(12 - i);
//
//				if (op == 0) i++;
//				else i--;
//
//				if(accept_time > 1000){
//					//---MODE CHANGING ROUTINE---
//
//					mode_confirm_animation();
//					return;
//
//					//---MODE CHANGING ROUTINE--- END
//				}
//
//				//---MODE BLINKING ROUTINE--- END
//			}
//
//			//---MODE SELECTION ROUTINE--- END
//		}
//	}
//}


