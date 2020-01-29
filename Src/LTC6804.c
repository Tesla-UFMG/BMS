/****************************************************************/
/**					Team Formula Tesla UFMG - 2018				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Eric, Virginia							*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/

#include "LTC6804.h"
#include "stm32f1xx_hal.h"
#include "dwt_stm32_delay.h"
#include "main.h"
#include "usart.h"
#include <stdlib.h>

uint8_t max_retries = 0;
static uint16_t v_pup[N_OF_PACKS][N_OF_CELLS], v_pdw[N_OF_PACKS][N_OF_CELLS];

static const uint16_t GLOBAL_COMMANDS_TABLE[8] = {
		0x0000, //DUMMY
		0x0360,	//COMMAND_ADCV
		0x0328,	//COMMAND_ADOW_ALL_PUP_0
		0x0368,	//COMMAND_ADOW_ALL_PUP_1
		0x0560,	//COMMAND_ADAX_ALL
		0x0711,	//CLEAR
};

static const uint16_t GLOBAL_COMMANDS_PEC[8] = {
		0xB65C, //DUMMY_PEC
		0xf46c,	//COMMAND_ADCV
		0xF8E8,	//COMMAND_ADOW_ALL_PUP_0
		0x1C62,	//COMMAND_ADOW_ALL_PUP_1
		0xD3A0,	//COMMAND_ADAX_ALL
		0xc9c0	//COMMAND_ADAX_ALL
};

static const uint16_t ADDRESSED_COMMANDS_TABLE[4][8] = {{
		0x8004, //READ  S0 V_GROUP A
		0x8006, //READ  S0 V_GROUP B
		0x8008, //READ  S0 V_GROUP C
		0x800A, //READ  S0 V_GROUP D
		0x800C, //READ  S0 T_GROUP A
		0x800E, //READ  S0 T_GROUP B
		0x8002, //READ  S0 CONFIG
		0x8001	//WRITE S0 CONFIG
},{
		0x8804, //READ  S1 V_GROUP A
		0x8806, //READ  S1 V_GROUP B
		0x8808, //READ  S1 V_GROUP C
		0x880A, //READ  S1 V_GROUP D
		0x880C, //READ  S1 T_GROUP A
		0x880E, //READ  S1 T_GROUP B
		0x8802, //READ  S1 CONFIG
		0x8801	//WRITE S1 CONFIG
},{
		0x9004, //READ  S2 V_GROUP A
		0x9006, //READ  S2 V_GROUP B
		0x9008, //READ  S2 V_GROUP C
		0x900A, //READ  S2 V_GROUP D
		0x900C, //READ  S2 T_GROUP A
		0x900E, //READ  S2 T_GROUP B
		0x9002, //READ  S2 CONFIG
		0x9001	//WRITE S2 CONFIG
},{
		0x9804, //READ  S3 V_GROUP A
		0x9806, //READ  S3 V_GROUP B
		0x9808, //READ  S3 V_GROUP C
		0x980A, //READ  S3 V_GROUP D
		0x980C, //READ  S3 T_GROUP A
		0x980E, //READ  S3 T_GROUP B
		0x9802, //READ  S3 CONFIG
		0x9801  //WRITE S3 CONFIG
}
};

static uint16_t ADDRESSED_COMMANDS_PEC[4][8];

static int16_t THERMISTOR_ZEROS[N_OF_PACKS][N_OF_THERMISTORS];

extern SPI_HandleTypeDef hspi1;

static void v_cs(uint8_t level){
	HAL_GPIO_WritePin(ISOSPI_CS_GPIO_Port, ISOSPI_CS_Pin , level);
}

static uint16_t byte_swap(uint16_t word){
	return (word >> 8) + (word << 8);
}

//static uint16_t LTC6804_pec_generator (uint16_t* data, uint8_t size, uint8_t option){
//
//	uint16_t aux, in0, pec = SEED_PEC;
//	int i, j;
//
//	for(i = 0; i < size; i++){
//
//		if (option == READ)
//			data[i] = byte_swap(data[i]);
//
//		for(j = 15; j >= 0; j--){
//			aux = 0x00;
//			in0 =  ((data[i] >> j) & 0x01) ^ ((pec >> 14) & 0x01);
//			aux =  in0;
//			aux =  aux << 4;
//			aux += in0;
//			aux =  aux << 2;
//			aux += in0;
//			aux =  aux << 1;
//			aux += in0;
//			aux =  aux << 3;
//			aux += in0;
//			aux =  aux << 1;
//			aux += in0;
//			aux =  aux << 3;
//			aux += in0;
//			pec =  aux ^ (pec << 1);
//		}
//	}
//	pec = pec << 1;
//
//	if (option == READ)
//		pec = byte_swap(pec);
//
//	return (pec);
//}

static uint16_t LTC6804_pec_generator (uint16_t* data, uint8_t size, uint8_t option){

	uint16_t aux, in0, pec = SEED_PEC;
	int i, j;

	for(i = 0; i < size; i++){

		if (option == READ)
			data[i] = byte_swap(data[i]);

		for(j = 15; j >= 0; j--){
			aux = 0x00;
			in0 =  ((data[i] >> j) & 0x01) ^ ((pec >> 14) & 0x01);
			aux =  in0;
			aux =  aux << 4;
			aux += in0;
			aux =  aux << 2;
			aux += in0;
			aux =  aux << 1;
			aux += in0;
			aux =  aux << 3;
			aux += in0;
			aux =  aux << 1;
			aux += in0;
			aux =  aux << 3;
			aux += in0;
			pec =  aux ^ (pec << 1);
		}
		if (option == READ)
			data[i] = byte_swap(data[i]);
	}
	pec = pec << 1;

	if (option == READ)
		pec = byte_swap(pec);

	return (pec);
}

static uint16_t LTC6804_transmit_recieve (uint16_t Tx_data){

	uint16_t Rx_data = 0;

	HAL_SPI_TransmitReceive(&hspi1,(uint8_t *) &Tx_data, (uint8_t *) &Rx_data, 1, 50);

	return(byte_swap(Rx_data));

}

static void LTC6804_wakeup(){

	v_cs(0);
	DWT_Delay_us(10);
	LTC6804_transmit_recieve(DUMMY);
	v_cs(1);
	DWT_Delay_us(T_WAKEUP);

}

//static uint8_t LTC6804_send_addressed_command(uint8_t address, uint8_t command, uint16_t data[3], uint8_t option){
//	uint16_t pec_vector[4];
//	uint8_t retries = 0;
//
//	//UART_print("TC_ERR[%d]", max_err);
//
//	while (retries < MAX_RETRIES){
//
//		if(max_retries < retries){
//			max_retries = retries;
//		}
//
//		LTC6804_wakeup();
//		v_cs(0);
//		DWT_Delay_us(10);
//		LTC6804_transmit_recieve(ADDRESSED_COMMANDS_TABLE[address][command]);
//		LTC6804_transmit_recieve(ADDRESSED_COMMANDS_PEC  [address][command]);
//		pec_vector[0] = LTC6804_transmit_recieve(DUMMY);
//		pec_vector[1] = LTC6804_transmit_recieve(DUMMY);
//		pec_vector[2] = LTC6804_transmit_recieve(DUMMY);
//		pec_vector[3] = LTC6804_transmit_recieve(DUMMY);
//		v_cs(1);
//
//		if(pec_vector[3] == LTC6804_pec_generator(pec_vector, 3, READ)){ //Mensagem sem erros
//			data[0] = pec_vector[0];
//			data[1] = pec_vector[1];
//			data[2] = pec_vector[2];
//
////			UART_print("%d\n", data[0]);
////			UART_print("%d\n", data[1]);
////			UART_print("%d\n", data[2]);
////			UART_print("\n");
//
//			return STAT_OPPERATING;
//		}
//		else{
//			retries++;
//		}
//	}
//
//	return STAT_OFFLINE;
//}

static uint8_t LTC6804_send_addressed_command(uint8_t address, uint8_t command, uint16_t data[3], uint8_t option){
	uint16_t rx_vector[4], tx_vector[4];
	uint8_t retries = 0;

	if(option == WRITE){

		tx_vector[0] = data[0];
		tx_vector[1] = data[1];
		tx_vector[2] = data[2];
		tx_vector[3] = LTC6804_pec_generator(data, 3, WRITE);
	}else{
		tx_vector[0] = DUMMY;
		tx_vector[1] = DUMMY;
		tx_vector[2] = DUMMY;
		tx_vector[3] = DUMMY;
	}

	while (retries < MAX_RETRIES){

		LTC6804_wakeup();
		v_cs(0);
		DWT_Delay_us(10);
		LTC6804_transmit_recieve(ADDRESSED_COMMANDS_TABLE[address][command]);
		LTC6804_transmit_recieve(ADDRESSED_COMMANDS_PEC  [address][command]);
		rx_vector[0] = LTC6804_transmit_recieve(tx_vector[0]);
		rx_vector[1] = LTC6804_transmit_recieve(tx_vector[1]);
		rx_vector[2] = LTC6804_transmit_recieve(tx_vector[2]);
		rx_vector[3] = LTC6804_transmit_recieve(tx_vector[3]);
		v_cs(1);

		if (option == READ) {
			if(rx_vector[3] == LTC6804_pec_generator(rx_vector, 3, READ)){ //Mensagem sem erros

				//UART_print("%d, %d,%d,%d\n" ,address, rx_vector[0], rx_vector[1], rx_vector[2]);

				data[0] = rx_vector[0];
				data[1] = rx_vector[1];
				data[2] = rx_vector[2];
				return STAT_OPPERATING;
			}
			else{
				retries++;
			}
		}else{
			retries = MAX_RETRIES;
		}
	}

	//UART_print("ERR[%d]", retries);
	return STAT_OFFLINE;
}

static void LTC6804_send_global_command(uint8_t command){
	LTC6804_wakeup();
	v_cs(0);
	DWT_Delay_us(10);
	LTC6804_transmit_recieve(GLOBAL_COMMANDS_TABLE[command]);
	LTC6804_transmit_recieve(GLOBAL_COMMANDS_PEC  [command]);
	v_cs(1);
	DWT_Delay_us(5000);
}

void LTC6804_set_thermistor_zeros(){
	uint32_t mean = 0;
	uint16_t t_aux[N_OF_PACKS][6];
	LTC6804_send_global_command(COMMAND_ADAX_ALL);

	HAL_Delay(1000);

	for (int i = 0; i < N_OF_PACKS; i++) {
		LTC6804_read_registers(t_aux[i], i, READ_TEMPERATURES);
		mean += t_aux[i][0] + t_aux[i][1] + t_aux[i][2] + t_aux[i][3];
	}

	mean = (uint32_t)((float)mean/(N_OF_PACKS*N_OF_THERMISTORS));

	for (int i = 0; i < N_OF_PACKS; i++)
		for (int j = 0; j < N_OF_THERMISTORS; ++j)
			THERMISTOR_ZEROS[i][j] = mean - t_aux[i][j];
}

uint8_t LTC6804_init(LTC6804 *sensor, uint8_t address){

	uint16_t data = 0x0360;
	UART_print("%x", LTC6804_pec_generator(&data, 1, WRITE));
	uint16_t aux, test[3];
	sensor->address = address;
	for(uint8_t i = 0; i < 7; i++){
		aux = ADDRESSED_COMMANDS_TABLE[address][i];
		ADDRESSED_COMMANDS_PEC[address][i] = LTC6804_pec_generator(&aux, 1, WRITE);
	}
	aux = ADDRESSED_COMMANDS_TABLE[address][7];
	ADDRESSED_COMMANDS_PEC        [address][7] = LTC6804_pec_generator(&aux, 1, WRITE);

	for(uint8_t i = 0; i < N_OF_CELLS; i++){
		sensor->v_error[i] = ERR_NO_ERROR;
	}

	v_cs(1);

	sensor->status = LTC6804_send_addressed_command(address, COMMAND_READ_CONFIG, test, READ);


	return sensor->status;
}

void LTC6804_convert_all(){

	//LTC6804_send_global_command(COMMAND_CLEAR_V);

	//CONVERTE ADC's DE TENSÃO
	LTC6804_send_global_command(COMMAND_ADCV);

	//CONVERTE ADC's DE TEMPERATURA
	LTC6804_send_global_command(COMMAND_ADAX_ALL);

}

void LTC6804_convert_fuses(){
	uint8_t i;
	for(i = 0; i < 10; i++)
		LTC6804_send_global_command(COMMAND_ADOW_ALL_PUP_0);
	for(i = 0; i < 4; i++)
		LTC6804_read_registers(v_pdw[i], i, READ_VOLTAGES);
	for(i = 0; i < 10; i++)
		LTC6804_send_global_command(COMMAND_ADOW_ALL_PUP_1);
	for(i = 0; i < 4; i++)
		LTC6804_read_registers(v_pup[i], i, READ_VOLTAGES);
}

uint8_t LTC6804_check_status(LTC6804* sensor){
	uint16_t aux[3];

	if (LTC6804_send_addressed_command(sensor->address, 6, aux, READ) == STAT_OPPERATING){
		sensor->status = STAT_OPPERATING;
		return TRUE;
	}else{
		sensor->status = STAT_OFFLINE;
		return FALSE;
	}
}

uint8_t LTC6804_read_registers(uint16_t* data, uint8_t address, uint8_t option){
	uint8_t comm_error = STAT_OPPERATING;
	uint16_t aux[3];

	switch(option){
	case READ_VOLTAGES:
		for(uint8_t i = 0; i < 4; i++){

			comm_error |= LTC6804_send_addressed_command(address, i, aux, READ);

			data[(i * 3) + 0] = aux[0];
			data[(i * 3) + 1] = aux[1];
			data[(i * 3) + 2] = aux[2];

			//EXCLUIR ISSO O MAIS RAPIDO POSSÌVEL
			if(address == 1){
				data[0] = data[2];
				data[1] = data[2];
			}
			//CONSERTAR FUSIVEL RAPIDO, TIPO PRA JÀ

		}
		break;
	case READ_TEMPERATURES:
		for(uint8_t i = 0; i < 2; i++){

			comm_error |= LTC6804_send_addressed_command(address, i + 4, aux, READ);

			data[(i * 3) + 0] = aux[0];
			data[(i * 3) + 1] = aux[1];
			data[(i * 3) + 2] = aux[2];

		}

		//CONVERT THE TEMPERATURE VALUE< USING data[5] AS REFERENCE
		data[0] = LTC6804_convert_to_degrees(data[0], data[5], THERMISTOR_ZEROS[address][0]);
		data[1] = LTC6804_convert_to_degrees(data[1], data[5], THERMISTOR_ZEROS[address][1]);
		data[2] = LTC6804_convert_to_degrees(data[2], data[5], THERMISTOR_ZEROS[address][2]);
		data[3] = LTC6804_convert_to_degrees(data[3], data[5], THERMISTOR_ZEROS[address][3]);

		break;
	}

	return comm_error;
}

uint8_t LTC6804_get_error(LTC6804* sensor){

	uint8_t i, error_flag = 0;

	sensor->v_sum = 0;
	sensor->v_var = 0;
	sensor->v_min_cell = sensor->v_cell[0];
	sensor->v_max_cell = sensor->v_cell[0];

	for(i = 0; i < N_OF_CELLS; i++){

		sensor->v_sum += (uint32_t)sensor->v_cell[i];
		if(sensor->v_cell[i] < sensor->v_min_cell)
			sensor->v_min_cell = sensor->v_cell[i];

		if(sensor->v_cell[i] > sensor->v_max_cell)
			sensor->v_max_cell = sensor->v_cell[i];

		if(sensor->v_cell[i] > MAX_CELL_V_DISCHARGE)
			sensor->v_error[i] |= ERR_OVER_VOLTAGE;
		else if(sensor->v_cell[i] < MIN_CELL_V)
			sensor->v_error[i] |= ERR_UNDER_VOLTAGE;
		if(sensor->v_cell[i] < 34000)
			sensor->v_error[i] &= ~ERR_OVER_VOLTAGE;
		else if(sensor->v_cell[i] > 30000)
			sensor->v_error[i] &= ~ERR_UNDER_VOLTAGE;


////		//EXCLUIR ISSO O MAIS RAPIDO POSSÌVEL
//		if(sensor->address == 3){
//			sensor->v_error[i] = 0;
//		}
////		//CONSERTAR FUSIVEL RAPIDO, TIPO PRA JÀ

		error_flag |= sensor->v_error[i];
	}

	sensor->t_max = 0;
	sensor->t_mean = 0;

	for(i = 0; i < N_OF_CELLS; i++){

		if(sensor->v_var < sensor->v_cell[i] - sensor->v_min_cell)
			sensor->v_var = sensor->v_cell[i] - sensor->v_min_cell;

	}

	for(i = 0; i < N_OF_THERMISTORS; i++){

		sensor->t_mean += sensor->t_cell[i];

		if(sensor->t_cell[i] > sensor->t_max)
			sensor->t_max = sensor->t_cell[i];

		if(sensor->t_cell[i] > MAX_TEMPERATURE)
			sensor->t_error[i] = ERR_OVER_TEMPERATURE;
		else{
			sensor->t_error[i] = ERR_NO_ERROR;
		}

		error_flag |= sensor->t_error[i];
	}



	sensor->t_mean = sensor->t_mean/4;


	return error_flag;
}

uint16_t LTC6804_convert_to_degrees(uint16_t value, uint16_t v_ref, uint16_t zero){
	int32_t res;
	double temp;

	//UART_print("%d\n", value);
	value = 60000 - v_ref - value;

	//UART_print("%d\n", v_ref);
	res = (300000000/value) - 10000;


	temp = log(res);
	temp = 1/(0.0013116461 + (0.0002129564 * temp) + (0.0000001036 * temp * temp * temp));
	temp = (temp - 273.15)*1000 + zero;

	return (uint16_t)temp;
}

uint8_t LTC6804_check_fuses(LTC6804 *sensor){
	int16_t v_delta[12];
	uint8_t error = ERR_NO_ERROR;
	uint8_t i;

	for(i = 1; i < 12; i++){
		v_delta[i] = (signed)v_pup[sensor->address][i] - (signed)v_pdw[sensor->address][i];


		if (v_delta[i] < -4000){
			sensor->v_error[i] |= ERR_OPEN_FUSES;
			error |= ERR_OPEN_FUSES;
			UART_print("[%d]: " , i);
		}
		else
			sensor->v_error[i] &= ~ERR_OPEN_FUSES;
	}

	// Imprime teste dos fusíveis
	//	UART_print("\r\n FUSE CHECK \r\n");
	//	for(int i=0; i<N_OF_CELLS; i++){
	//		UART_print(" FUSE");
	//		UART_print("[%d]: " , i);
	//		if(sensor->v_error[i])
	//			UART_print("OPEN -- ");
	//		else
	//			UART_print("ok -- ");
	//	}
	//	UART_print("\r\n");

	if (v_pup[sensor->address][0]  == 0){
		sensor->v_error[0]  |= ERR_OPEN_FUSES;
		error |= ERR_OPEN_FUSES;
		//UART_print("[%d]: open " , 0);
	}
	else
		sensor->v_error[0]  |= ERR_NO_ERROR;

	if (v_pdw[sensor->address][11] == 0){
		sensor->v_error[11] |= ERR_OPEN_FUSES;
		error |= ERR_OPEN_FUSES;
		//UART_print("[%d]: open " , 11);
	}
	else
		sensor->v_error[11] |= ERR_NO_ERROR;

	return error;
}


void LTC6804_balance_init(LTC6804 *sensor){

	for(uint8_t i = 0; i < N_OF_CELLS; i++){
			sensor->v_error[i] |= ERR_BALANCE;
			LTC6804_balance(sensor);
			HAL_Delay(50);
			LTC6804_balance_reset_flag(sensor);
			LTC6804_balance(sensor);
	}

}

void LTC6804_balance_set_flag(LTC6804 *sensor){

	for(uint8_t i = 0; i < N_OF_CELLS; i++){

		if(sensor->v_cell[i] - sensor->v_min_cell > MIN_V_DELTA){
			sensor->v_error[i] |= ERR_BALANCE;
		}
		else{
			//UART_print("cell %d balanceada\n", i);
			sensor->v_error[i] &= ~ERR_BALANCE;
		}
	}

}

void LTC6804_balance_reset_flag(LTC6804 *sensor){

	for(uint8_t i = 0; i < N_OF_CELLS; i++){
		sensor->v_error[i] &= ~ERR_BALANCE;
	}

}

void LTC6804_start_balancing(LTC6804 *sensor){
	uint16_t data[3], balance_flag = 0;

	for(uint8_t i = 0; i < N_OF_CELLS; i++){
		if((sensor->v_error[i] & ERR_BALANCE) == ERR_BALANCE){
			balance_flag |= 1 << i;
		}
	}

	balance_flag |= 1 << 12;

	data[0] = byte_swap(0x0000);
	data[1] = byte_swap(0x0000);
	data[2] = byte_swap(balance_flag);

	LTC6804_send_addressed_command(sensor->address, COMMAND_WRITE_CONFIG, data, WRITE);

	DWT_Delay_us(1000);
	//
	//	UART_print("%x\n", balance_flag);
	//	UART_print("%x\n", pec);
	//	UART_print("%x\n", LTC6804_pec_generator(data, 1));
	//	UART_print("%x\n", ADDRESSED_COMMANDS_TABLE[0][COMMAND_WRITE_CONFIG]);
	//	UART_print("%x\n", ADDRESSED_COMMANDS_PEC[0][COMMAND_WRITE_CONFIG]);
	//	uint16_t aux = ADDRESSED_COMMANDS_TABLE[0][COMMAND_WRITE_CONFIG];
	//	UART_print("%x\n", LTC6804_pec_generator(&aux, 1));

	//LTC6804_send_addressed_command(sensor->address, COMMAND_READ_CONFIG, data);

	//	UART_print("\n%x", data[0]);
	//	UART_print("\n%x", data[1]);
	//	UART_print("\n%x", data[2]);


}

void LTC6804_balance(LTC6804 *sensor){

	uint16_t data[3], balance_flag = 0;

	for(uint8_t i = 0; i < N_OF_CELLS; i++){
		if((sensor->v_error[i] & ERR_BALANCE) == ERR_BALANCE){
			balance_flag |= 1 << i;
		}
	}

	balance_flag |= 1 << 12;

	data[0] = byte_swap(0x0000);
	data[1] = byte_swap(0x0000);
	data[2] = byte_swap(balance_flag);

	LTC6804_send_addressed_command(sensor->address, COMMAND_WRITE_CONFIG, data, WRITE);

}
void LTC6804_pause_balancing(LTC6804 *sensor){
	uint16_t data[3];


	data[0] = byte_swap(0x0000);
	data[1] = byte_swap(0x0000);
	data[2] = byte_swap(0x0000);

	LTC6804_send_addressed_command(sensor->address, COMMAND_WRITE_CONFIG, data, WRITE);

	//UART_print("%d\n", data[2]);

	DWT_Delay_us(1000);

}
