/****************************************************************/
/**					Team Formula Tesla UFMG - 2018				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Eric, Virginia							*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/

#include "BMS.h"

static int8_t UV_retries, OV_retries, OT_retries;

//uint8_t balance_enable = 1;


//extern uint8_t mode_button;
//extern I2C_HandleTypeDef hi2c2;





static const uint16_t CAN_ID_TABLE[8][5] = {

		{
				256,		//PACK 2 V_GROUP 0 		(cells 0, 1, 2, 3)
				257,		//PACK 2 V_GROUP 1		(cells 4, 5, 6, 7)
				258,		//PACK 2 V_GROUP 2      (cells 8, 9, 10, 11)
				259,		//PACK 2 T_GROUP        (sensor 0, 1, 2, 3)
		},
		{
				260,		//PACK 3 V_GROUP 0 		(cells 0, 1, 2, 3)
				261,		//PACK 3 V_GROUP 1      (cells 4, 5, 6, 7)
				262,		//PACK 3 V_GROUP 2      (cells 8, 9, 10, 11)
				263,		//PACK 3 T_GROUP        (sensor 0, 1, 2, 3)
		},
		{
				264,		//PACK 4 V_GROUP 0 		(cells 0, 1, 2, 3)
				265,		//PACK 4 V_GROUP 1      (cells 4, 5, 6, 7)
				266,		//PACK 4 V_GROUP 2      (cells 8, 9, 10, 11)
				267,		//PACK 4 T_GROUP        (sensor 0, 1, 2, 3)
		},
		{
				268,		//PACK 5 V_GROUP 0      (cells 0, 1, 2, 3)
				269,		//PACK 5 V_GROUP 1      (cells 4, 5, 6, 7)
				270,		//PACK 5 V_GROUP 2      (cells 8, 9, 10, 11)
				271,		//PACK 5 T_GROUP        (sensor 0, 1, 2, 3)
		},
		{
				272,		//PACK 0 V_GROUP 0 		(cells 0, 1, 2, 3)
				273,		//PACK 0 V_GROUP 1		(cells 4, 5, 6, 7)
				274,		//PACK 0 V_GROUP 2      (cells 8, 9, 10, 11)
				275,		//PACK 0 T_GROUP        (sensor 0, 1, 2, 3)
		},

		{
				276,		//PACK 1 V_GROUP 0 		(cells 0, 1, 2, 3)
				277,		//PACK 1 V_GROUP 1		(cells 4, 5, 6, 7)
				278,		//PACK 1 V_GROUP 2      (cells 8, 9, 10, 11)
				279,		//PACK 1 T_GROUP        (sensor 0, 1, 2, 3)
		},
		{
				50,		//CHARGING CURRENT (bytes 0, 1, 2, 3) AND DISCHARGING CURRENT(bytes 4, 5, 6, 7)
		},
		{
				51,		//BMS MODE(bytes 0),COMM MODE(bytes 1), AIR STATUS(bytes 2 & 3), BATTERY %(bytes 4 & 5), GLOBAL ERROR FLAG(byte 6)
				52,
				53
		},
};

#define BMS_CONVERT_CELL 	1
#define BMS_CONVERT_GPIO	2
#define BMS_CONVERT_STAT	4
#define BMS_CONVERT_CONFIG	8

 int16_t THERMISTOR_ZEROS[N_OF_PACKS][5];

/*******************************************************
  Function void BMS_set_thermistor_zeros(BMS_struct*)

V1.0:
The function calculates the thermistors' zeros for more
accurate readings.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void BMS_set_thermistor_zeros(BMS_struct *BMS){
	uint32_t mean = 0;

	for (int i = 0; i < N_OF_SLAVES; i++){
		if(i!=1 && i!=4 && i!=7){
			for (int j = 0; j < N_OF_THERMISTORS; ++j)
				THERMISTOR_ZEROS[i][j] = 0;
		}
	}

	BMS_convert(BMS_CONVERT_GPIO, BMS);

	for (int i = 0; i < N_OF_SLAVES; i++){
		if(i!=1 && i!=4 && i!=7){
			for (int j = 0; j < N_OF_THERMISTORS; ++j)
				mean += BMS->sensor[i]->GxV[j];
		}
	}

	mean = (uint32_t)((float)mean/(N_OF_PACKS*N_OF_THERMISTORS));

	for (int i = 0; i < N_OF_SLAVES; i++){
		if(i!=1 && i!=4 && i!=7){
			for (int j = 0; j < N_OF_THERMISTORS; ++j)
				THERMISTOR_ZEROS[i][j] = mean - BMS->sensor[i]->GxV[j];
		}
	}
}

/*******************************************************
  Function void BMS_init(BMS_struct*)

V1.0:
The function is responsible for initializing the Battery
Management System (BMS). It sets up all the configuration
needed for the BMS boot.

  Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void BMS_init(BMS_struct *BMS){


	BMS->config = (LTC_config*) calloc(1 ,sizeof(LTC_config));
	BMS->config->command = (LTC_command*) calloc(1 ,sizeof(LTC_command));

	for(int i = 0; i < N_OF_SLAVES; i++){
		BMS->sensor[i] = (LTC_sensor*) calloc(1, sizeof(LTC_sensor));
		BMS->sensor[i]->ADDR = i;
		LTC_init(BMS->config);
	}

	BMS->error = ERR_NO_ERROR;
	BMS->v_min = 50000;
	BMS->v_max = 0;

	BMS->mode = 0;


	uint16_t aux;
	EE_ReadVariable(0x0, &aux);
	BMS->charge = ((uint32_t)aux) << 16; 	//load upper bytes
	EE_ReadVariable(0x1, &aux);
	BMS->charge += aux;		//load lower bytes

	EE_ReadVariable(0x2, &aux);
	BMS->charge_min = ((uint32_t)aux) << 16; 	//load upper bytes
	EE_ReadVariable(0x3, &aux);
	BMS->charge_min += aux;		//load lower bytes

	EE_ReadVariable(0x4, &aux);
	BMS->charge_max = ((uint32_t)aux) << 16; 	//load upper bytes
	EE_ReadVariable(0x5, &aux);
	BMS->charge_max += aux;		//load lower bytes

	LTC_init(BMS->config);

	BMS_initial_SOC(BMS);

//	BMS_set_thermistor_zeros(BMS);

}

/*******************************************************
  Function void BMS_convert(uint8_t, BMS_struct*)

V1.0:
The function converts the cells' voltage or the GPIO pins'
analogic values into digital values.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void BMS_convert(uint8_t BMS_CONVERT, BMS_struct *BMS){

	if (BMS_CONVERT&BMS_CONVERT_CELL) {

		BMS->config->command->NAME = LTC_COMMAND_ADCV;
		BMS->config->command->BROADCAST = TRUE;
		LTC_send_command(BMS->config);

		BMS->v_min = MAX_CELL_V_DISCHARGE;

		for(uint8_t i = 0; i < N_OF_SLAVES; i++){

			LTC_read(LTC_READ_CELL, BMS->config, BMS->sensor[i]);

			if(BMS->sensor[i]->V_MIN < BMS->v_min)
				BMS->v_min = BMS->sensor[i]->V_MIN;
			if(BMS->sensor[i]->V_MAX > BMS->v_max)
				BMS->v_max = BMS->sensor[i]->V_MAX;

			BMS->v_TS += BMS->sensor[i]->SOC;

			//TODO Verificar necessidade desse loop
			for(uint8_t j = 0; j < 4; j++){
				if(BMS->sensor[i]->GxV[j] > BMS->t_max)
					BMS->t_max = BMS->sensor[i]->GxV[j];
			}
		}

	}
	if (BMS_CONVERT&BMS_CONVERT_GPIO) {

		BMS->config->command->NAME = LTC_COMMAND_ADAX;
		BMS->config->command->BROADCAST = TRUE;
		LTC_send_command(BMS->config);

		BMS->t_max = 30000;

		for(uint8_t i = 0; i < N_OF_SLAVES; i++){

			LTC_read(LTC_READ_GPIO, BMS->config, BMS->sensor[i]);

			for(uint8_t j = 0; j < N_OF_THERMISTORS; j++){

				if(BMS->sensor[i]->GxV[j] < BMS->t_max && BMS->sensor[i]->GxV[j] > 1000)
					BMS->t_max = BMS->sensor[i]->GxV[j];

			}
		}
	}
	if (BMS_CONVERT&BMS_CONVERT_STAT) {

		BMS->config->command->NAME = LTC_COMMAND_ADSTAT;
		BMS->config->command->BROADCAST = TRUE;
		LTC_send_command(BMS->config);

		for(uint8_t i = 0; i < N_OF_SLAVES; i++){

			LTC_read(LTC_READ_STATUS, BMS->config, BMS->sensor[i]);

			BMS->v_TS += BMS->sensor[i]->SOC;
		}

		BMS->v_TS /= (N_OF_PACKS / PACKS_IN_SERIES);

	}
}

/*******************************************************
  Function void BMS_monitoring(BMS_struct*)

V1.0:
The function monitors the main aspects of the BMS, such as
the maximum and minimum voltages, the BMS charge percent and
the need of balancing the cells.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void BMS_monitoring(BMS_struct *BMS){


	BMS->v_min = BMS->sensor[0]->V_MIN;
	BMS->v_max = BMS->sensor[0]->V_MAX;
	BMS->v_TS = 0;
	BMS->t_max = 0;

	//TODO Verificar necessidade dessa atribuição
	BMS->v_min = 50000;
	BMS->v_max = 0;

	BMS->charge_percent = 0;

	BMS_convert(BMS_CONVERT_CELL|BMS_CONVERT_GPIO|BMS_CONVERT_STAT, BMS);


	for(uint8_t i = 0; i < N_OF_PACKS; i++){
		if (BMS->mode & BMS_BALANCING)
			LTC_set_balance_flag(BMS->config, BMS->sensor[i]);
		else
			LTC_reset_balance_flag(BMS->config, BMS->sensor[i]);

		LTC_balance(BMS->config, BMS->sensor[i]);

		BMS->charge_percent += BMS->sensor[i]->TOTAL_CHARGE;

	}

	BMS->charge_percent /= N_OF_PACKS;


	if(BMS->charge < BMS->charge_min)
		BMS->charge_min = BMS->charge;
	if(BMS->charge > BMS->charge_max)
		BMS->charge_max = BMS->charge;

	if(HAL_GPIO_ReadPin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin) == 0)
		BMS->AIR = AIR_OPEN;
	else
		BMS->AIR = AIR_CLOSED;

	EE_WriteVariable(0x0, (uint16_t) (BMS->charge >> 16));
	EE_WriteVariable(0x1, (uint16_t) BMS->charge);

	EE_WriteVariable(0x2, (uint16_t) (BMS->charge_min >> 16));
	EE_WriteVariable(0x3, (uint16_t) BMS->charge_min);

	EE_WriteVariable(0x4, (uint16_t) (BMS->charge_max >> 16));
	EE_WriteVariable(0x5, (uint16_t) BMS->charge_max);


}

uint16_t flag = 0;

/*******************************************************
 Function void BMS_error(BMS_struct*)

V1.0:
The function tests a series of conditions and sets the right
error flag depending on what may happen through the BMS opera-
tion.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void BMS_error(BMS_struct *BMS){

	if(BMS->v_min <= MIN_CELL_V)
		flag |= ERR_UNDER_VOLTAGE;

	if(BMS->v_max >= MAX_CELL_V_DISCHARGE)
		flag |= ERR_OVER_VOLTAGE;

	if(BMS->t_max < 10000)
		flag |= ERR_OVER_TEMPERATURE;

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


	if(UV_retries == 5){
		NextError[0] = 1;
		BMS->error |= ERR_UNDER_VOLTAGE;
	}
	else if(UV_retries == 0){
		NextError[0] = 0;
		BMS->error &= ~ERR_UNDER_VOLTAGE;
	}
	if(OV_retries == 5){
		NextError[1] = 1;
		BMS->error |= ERR_OVER_VOLTAGE;
	}
	else if(OV_retries == 0){
		NextError[1] = 0;
		BMS->error &= ~ERR_OVER_VOLTAGE;
	}


	if(BMS->v_GLV < 13500){
		BMS->error |= ERR_GLV_VOLTAGE;
		NextError[4] = 1;
	}else if(BMS->v_GLV < 13500){
		BMS->error &= ~ERR_GLV_VOLTAGE;
		NextError[4] = 0;
	}



	if(BMS->error != ERR_NO_ERROR){
		HAL_GPIO_WritePin(AIR_ENABLE_GPIO_Port, AIR_ENABLE_Pin, RESET);
		HAL_GPIO_WritePin(ERR_LED_GPIO_Port, ERR_LED_Pin, SET);
		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, SET);
	}else{
		flag &= ERR_NO_ERROR;
		HAL_GPIO_WritePin(AIR_ENABLE_GPIO_Port, AIR_ENABLE_Pin, SET);
		HAL_GPIO_WritePin(ERR_LED_GPIO_Port, ERR_LED_Pin, RESET);
		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, RESET);
	}
}

void can_buf(uint8_t buffer[8], uint16_t word1, uint16_t word2, uint16_t word3, uint16_t word4){

	buffer[0] = word1;
	buffer[1] = word1 >> 8;
	buffer[2] = word2;
	buffer[3] = word2 >> 8;
	buffer[4] = word3;
	buffer[5] = word3 >> 8;
	buffer[6] = word4;
	buffer[7] = word4 >> 8;

}

/*******************************************************
 Function void BMS_can(BMS_struct*)

V1.0:
The function sends essential informations about the BMS such
as the cells' voltages, temperature, charge percent, maximum
and minimum voltages through CAN communication.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void BMS_can(BMS_struct *BMS){
	uint8_t can_buffer[8];

	for(uint8_t i = 0; i < N_OF_PACKS; i++){
		//if (BMS->sensor[i]->status == STAT_OPPERATING) {
		for (uint8_t j = 0; j < 3; j++) {

			can_buffer[0] = BMS->sensor[i]->CxV[4 * j + 0];
			can_buffer[1] = BMS->sensor[i]->CxV[4 * j + 0] >> 8;
			can_buffer[2] = BMS->sensor[i]->CxV[4 * j + 1];
			can_buffer[3] = BMS->sensor[i]->CxV[4 * j + 1] >> 8;
			can_buffer[4] = BMS->sensor[i]->CxV[4 * j + 2];
			can_buffer[5] = BMS->sensor[i]->CxV[4 * j + 2] >> 8;
			can_buffer[6] = BMS->sensor[i]->CxV[4 * j + 3];
			can_buffer[7] = BMS->sensor[i]->CxV[4 * j + 3] >> 8;

			CAN_Transmit(can_buffer, CAN_ID_TABLE[i][j]);

			//			sendString(CAN_ID_TABLE[i][j], 	BMS->sensor[i]->CxV[(j * 4)],
			//					BMS->sensor[i]->CxV[(j * 4) +1],
			//					BMS->sensor[i]->CxV[(j * 4) +2],
			//					BMS->sensor[i]->CxV[(j * 4) +3]);

		}

		can_buffer[0] = BMS->sensor[i]->GxV[0];
		can_buffer[1] = BMS->sensor[i]->GxV[0] >> 8;
		can_buffer[2] = BMS->sensor[i]->GxV[1];
		can_buffer[3] = BMS->sensor[i]->GxV[1] >> 8;
		can_buffer[4] = BMS->sensor[i]->GxV[2];
		can_buffer[5] = BMS->sensor[i]->GxV[2] >> 8;
		can_buffer[6] = BMS->sensor[i]->GxV[3];
		can_buffer[7] = BMS->sensor[i]->GxV[3] >> 8;



		CAN_Transmit(can_buffer, CAN_ID_TABLE[i][CAN_TEMPERATURE_ID]);

		//		sendString(CAN_ID_TABLE[i][CAN_TEMPERATURE_ID],
		//				BMS->sensor[i]->GxV[0],
		//				BMS->sensor[i]->GxV[1],
		//				BMS->sensor[i]->GxV[2],
		//				BMS->sensor[i]->GxV[3]);

	}

	can_buffer[0] = ((int16_t)BMS->current[0]);
	can_buffer[1] = ((int16_t)BMS->current[0]) >> 8;
	can_buffer[2] = ((int16_t)BMS->current[1]);
	can_buffer[3] = ((int16_t)BMS->current[1]) >> 8;
	can_buffer[4] = ((int16_t)BMS->current[2]);
	can_buffer[5] = ((int16_t)BMS->current[2]) >> 8;
	can_buffer[6] = ((int16_t)BMS->current[3]);
	can_buffer[7] = ((int16_t)BMS->current[3]) >> 8;

	CAN_Transmit(can_buffer, 50);
	//	CAN_Transmit(can_buffer, 352);



	//	sendString(CAN_ID_TABLE[CAN_CURRENT_ID][0],
	//			BMS->current[0],
	//			BMS->current[1],
	//			BMS->current[2],
	//			BMS->current[3]);

	can_buf(can_buffer, BMS->v_GLV,
			(uint16_t)(BMS->charge_percent/10),
			0,
			BMS->AIR);

	CAN_Transmit(can_buffer, 51);

	//	sendString(CAN_ID_TABLE[CAN_GENERAL_ID][0],
	//			BMS->v_GLV,
	//			(216000 - BMS->charge/100)/2160,
	//			BMS->AIR,
	//			BMS->error);


	can_buffer[0] = 0;
	can_buffer[1] = 0;
	can_buffer[2] = BMS->v_TS;
	can_buffer[3] = BMS->v_TS >> 8;
	can_buffer[4] = 0;
	can_buffer[5] = 0;
	can_buffer[6] = BMS->t_max;
	can_buffer[7] = BMS->t_max >> 8;

	CAN_Transmit(can_buffer, 52);

	//	sendString(CAN_ID_TABLE[CAN_GENERAL_ID][1],
	//			0,
	//			BMS->v_TS,
	//			0,
	//			BMS->t_max);

	can_buf(can_buffer, BMS->v_min,
			BMS->v_max,
			0,
			0);

	CAN_Transmit(can_buffer, 53);
	//
	//	//sendString(CAN_ID_TABLE[CAN_GENERAL_ID][2], BMS->v_min,0,0,0);
	//
	//	can_buf(can_buffer, BMS->AIR,
	//			0,
	//			0,
	//			0);
	//
	//	can_buffer[7] = 0;
	//
	//	CAN_Transmit(can_buffer, 0);

	//sendString(0, BMS->AIR,0,0);

}


/*******************************************************
 Function void BMS_initial_SOC(LTC_sensor)

V1.0:
The function reads all cells' voltage and takes the least
charged cell as the Open Circuit Voltage (OCV). Depending
on the value taken as the OCV, the SoC will be determined
by a linear equation.

 Version 1.0 - Initial release 13/05/2020 by Tesla UFMG
*******************************************************/
void BMS_initial_SOC(BMS_struct *BMS){
	/*for(uint8_t i = 0; i < N_OF_PACKS; i++)
		LTC_read(LTC_READ_CELL, BMS->config, BMS->sensor[i]);

	if(BMS->sensor->V_MIN < 33680 && BMS->sensor->V_MIN >= 33400)			//98,00% -- 96,00%
		BMS->charge_percent = 71.43*(BMS->sensor->V_MIN/10000) - 142.57;
	else if(BMS->sensor->V_MIN < 33400 && BMS->sensor->V_MIN >= 33310)		//96,61% -- 76,16%
		BMS->charge_percent = 2297.96*(BMS->sensor->V_MIN/10000) - 7578.34;
	else if(BMS->sensor->V_MIN < 33310 && BMS->sensor->V_MIN >= 32990)		//75,50% -- 68,61%
		BMS->charge_percent = 216.09*(BMS->sensor->V_MIN/10000) - 644.27;
	else if(BMS->sensor->V_MIN < 32990 && BMS->sensor->V_MIN >= 32880)		//71,30% -- 40,39%
		BMS->charge_percent = 2835.23*(BMS->sensor->V_MIN/10000) - 9281.84;
	else if(BMS->sensor->V_MIN < 32880 && BMS->sensor->V_MIN >= 31980)		//38,38% -- 10,01%
		BMS->charge_percent = 315.54*(BMS->sensor->V_MIN/10000) - 999.08;
	else if(BMS->sensor->V_MIN < 31980 && BMS->sensor->V_MIN >= 31290)		//9,53% -- 5,86%
		BMS->charge_percent = 53.23*(BMS->sensor->V_MIN/10000) - 160.69;
	else
		BMS->charge_percent = 0;*/

//	if(BMS->sensor->V_MIN < 34000 && BMS->sensor->V_MIN >= 33400)
//		BMS->charge_percent = 2*(BMS->sensor->V_MIN/10000) + 94;
//	else if(BMS->sensor->V_MIN < 33400 && BMS->sensor->V_MIN >= 33320)
//		BMS->charge_percent = 2*(BMS->sensor->V_MIN/10000) + 76;
//	else if(BMS->sensor->V_MIN < 33320 && BMS->sensor->V_MIN >= 33000)
//		BMS->charge_percent = 2*(BMS->sensor->V_MIN/10000) + 46;
//	else if(BMS->sensor->V_MIN < 33000 && BMS->sensor->V_MIN >= 32895)
//		BMS->charge_percent = 2*(BMS->sensor->V_MIN/10000) + 8;
//	else if(BMS->sensor->V_MIN < 32895 && BMS->sensor->V_MIN >= 31980)
//		BMS->charge_percent = 2*(BMS->sensor->V_MIN/10000) + 8;
//	else if(BMS->sensor->V_MIN < 31980 && BMS->sensor->V_MIN >= 31280)
//		BMS->charge_percent = 2*(BMS->sensor->V_MIN/10000) + 4;
//	else
//		BMS->charge_percent = 0;

	if(BMS->charge_percent > 100)
		BMS->charge_percent = 100;
	if(BMS->charge_percent < 0)
		BMS->charge_percent = 0;
}

/*******************************************************
 Function void BMS_charging(BMS_struct)

V1.0:
The function updates the battery's charge percent with the
current integration function, determining the battery's SoC
along with the BMS_Initial_SOC function.

 Version 1.0 - Initial release 14/05/2020 by Tesla UFMG
*******************************************************/
void BMS_charging(BMS_struct BMS){
	/*BMS->charge = BMS->charge + DHAB_currentIntegration();
	BMS->charge_variation_percent = (BMS->charge/BMS->charge_max)*100;
	BMS->charge_percent = BMS->charge_percent + BMS->charge_variation_percent;
	BMS->discharge_percent = 100 - BMS->charge_percent;

	if(BMS->error == ERR_OVER_VOLTAGE)
		BMS->charge_max = BMS->charge;*/
}

/*******************************************************
 Function void BMS_discharging(BMS_struct)

V1.0:
The function updates the battery's discharge percent with
the current integration function. Finally, it calculates
the battery's charge percent with the relation
charge = 100 - discharge

 Version 1.0 - Initial release 14/05/2020 by Tesla UFMG
*******************************************************/
void BMS_discharging(BMS_struct BMS){
	/*BMS->charge = BMS->charge + DHAB_currentIntegrarion();
	BMS->discharge_variation_percent = (BMS->charge/BMS->charge_max)*100;
	BMS->discharge_percent = BMS->discharge_percent + BMS->discharge_variation_percent;
	BMS->charge_percent = 100 - BMS->discharge_percent;

	if(BMS->error == ERR_UNDER_VOLTAGE)
		BMS->charge_min = BMS->charge;*/
}
