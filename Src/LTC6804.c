/****************************************************************/
/**					Team Formula Tesla UFMG - 2019				*/
/** Microcontroller: STM32F103XXXX								*/
/** Compiler: AC6 - STM worbench								*/
/** Author: Henrique, Rodolfo, Heuller, Wellington				*/
/** License: Free - Open Source									*/
/** 															*/
/****************************************************************/

#include <LTC6804.h>

extern SPI_HandleTypeDef hspi1;

#define BYTESWAP(word) ((word >> 8) + (word << 8))

static uint16_t pec_table[256];

short CRC15_POLY = 0x4599;

uint16_t LTC_PEC(uint16_t *data , uint8_t len) {
	int32_t remainder, address;
	remainder = LTC_PEC_SEED;
	for (uint8_t i = 0; i < len; i++){
		address   = ((remainder >> 7) ^ ((data[i] >> 8) & 0xFF)) & 0xFF; //calculate PEC table address
		remainder = (remainder << 8 ) ^ PEC_TABLE[address];
		address   = ((remainder >> 7) ^ (data[i] & 0xFF)) & 0xFF;    	 //calculate PEC table address
		remainder = (remainder << 8 ) ^ PEC_TABLE[address];
	}
	return (remainder * 2); //The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
}

/*******************************************************
 Function uint16_t LTC_pec2(uint16_t*, uint8_t, uint8_t)

V1.0:
The function calculates the Package Error Code (PEC) by
following the procedure described in the LTC6804 data-
sheet, page 44.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
uint16_t LTC_pec2 (uint16_t* data, uint8_t size, uint8_t option){

	uint16_t aux, in0, pec = 16;
	int i, j;

	for(i = 0; i < size; i++){

		if (option == 0)
			data[i] = BYTESWAP(data[i]);

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
		if (option == 0)
			data[i] = BYTESWAP(data[i]);
	}
	pec = pec << 1;

	if (option == 0)
		pec = BYTESWAP(pec);

	return (pec);
}

void LTC_PEC_InitTable() {
	uint16_t remainder;
	for(int i = 0; i < 256; i++) {
		remainder = i << 7;
		for(int bit = 8; bit > 0; --bit) {
			if(remainder & 0x4000) {
				remainder = remainder << 1;
				remainder = remainder ^ 0x4599;
			}
			else
				remainder = remainder << 1;
		}
		pec_table[i] = remainder&0xFFFF;
	}
}

uint16_t pec15(char *data, int len)
{
	uint16_t remainder, address;

	remainder = 16;
	for(int i = 0; i < len; i++)
	{
		address = ((remainder >> 7) ^ data[i]) & 0xff;
		remainder = (remainder << 8) ^ pec15Table[address];
	}
	return (remainder*2);
}

//static uint16_t LTC_TransmitRecieve (uint16_t Tx_data){
//
//	uint16_t Rx_data = 0;
//	HAL_SPI_TransmitReceive(&hspi1,(uint8_t *) &Tx_data, (uint8_t *) &Rx_data, 1, 50);
//
//	return(BYTESWAP(Rx_data));
//
//}

/*******************************************************
 Function void LTC_CS(uint8_t)

V1.0:
The function is used as an auxiliary function for writing
a bit in the CS GPIO port and waiting for 10us.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void LTC_ChipSelect(uint8_t level) {
	HAL_GPIO_WritePin(ISOSPI_CS_GPIO_Port, ISOSPI_CS_Pin , level);
	DWT_Delay_us(10);
}

void LTC_StartTrasmission() { LTC_ChipSelect(LOW); }

void LTC_EndTramission() { LTC_ChipSelect(HIGH); }

uint16_t LTC_SPI(uint16_t Tx_data) {
	uint16_t Rx_data = 0;
	HAL_SPI_TransmitReceive(&hspi1,(uint8_t *) &Tx_data, (uint8_t *) &Rx_data, 1, 50);
	return(BYTESWAP(Rx_data));
}

/*******************************************************
 Function void LTC_transmit_recieve (uint16_t, uint16_t*, uint16_t*)

V1.0:
The function is responsible for doing the transmit/receive messages'
routine, following the steps described in the LTC6804 datasheet.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/


void LTC_transmit_recieve (uint16_t command, uint16_t* tx_data, uint16_t* rx_data){

	uint16_t pec = LTC_PEC(&command, 1),
			buffer[4] = {command, pec, 0, 0}; //tx buffer

	//WAKE UP ROUTINE
	LTC_StartTrasmission();
	LTC_SPI(0);
	LTC_EndTramission();

	//SEND COMMAND ROUTINE:
	LTC_StartTrasmission();
	LTC_SPI(buffer[0]);
	LTC_SPI(buffer[1]);

	//TRANSMIT/RECIEVE DATA ROUTINE:
	for (uint8_t i = 0; i < 4; ++i)
		buffer[i] = tx_data[i];

	if((command & 0x07FF) == LTC_COMMAND_WRCFG){
		pec = LTC_pec2(tx_data, 3, 1);
		buffer[3] = pec;
	}

	if((buffer[0] & 0x07FF) < LTC_COMMAND_ADCV){
		for (uint8_t i = 0; i < 4; ++i)
			rx_data[i] = LTC_SPI(buffer[i]);
	}
	LTC_EndTramission();

}


//
//void LTC_wakeup(){
//
//	HAL_GPIO_WritePin(ISOSPI_CS_GPIO_Port, ISOSPI_CS_Pin, 0);
//	HAL_Delay(1);
//	LTC_TransmitRecieve(0);
//	HAL_GPIO_WritePin(ISOSPI_CS_GPIO_Port, ISOSPI_CS_Pin, 1);
//	HAL_Delay(1);
//
//}
//
//void transmit_command(uint16_t command){
//
//	LTC_TransmitRecieve(command);
//	LTC_TransmitRecieve(LTC_pec(&command, 1, 1));
//
//}
//
//void transmit_receive_data(uint16_t* tx_data, uint16_t* rx_data){
//
//	uint16_t aux[4];
//	aux[0] = tx_data[0];
//	aux[1] = tx_data[1];
//	aux[2] = tx_data[2];
//	aux[3] = LTC_pec(tx_data, 3, 0);
//
//	HAL_SPI_TransmitReceive(&hspi1,(uint8_t *) &aux, (uint8_t *) rx_data, 4, 50);
//
//}

/*******************************************************
 Function uint16_t LTC_make_command(LTC_command*)

V1.0:
The function is responsible for returning the bits of a
command divided into different parts of bits depending
on the command about to use and its bits of interest.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
uint16_t LTC_make_command(LTC_command *command){

	switch(command->NAME){

	case LTC_COMMAND_ADCV:

		return command->NAME | command->MD | command->DCP | command->CH;

		break;

	case LTC_COMMAND_ADOW:

		return command->NAME | command->MD | command->PUP | command->DCP | command->CH;

		break;

	case LTC_COMMAND_CVST:
	case LTC_COMMAND_AXST:
	case LTC_COMMAND_STATST:

		return command->NAME | command->MD | command->ST;

		break;

	case LTC_COMMAND_ADAX	:

		return command->NAME | command->MD | command->CHG;

		break;

	case LTC_COMMAND_ADSTAT	:

		return command->NAME | command->MD | command->CHST;

		break;

	case LTC_COMMAND_ADCVAX	:

		return command->NAME | command->MD | command->CHG;

		break;

	default:
		return command->NAME;
		break;
	}
}

/*******************************************************
 Function void LTC_send_command(LTC_config*, ...)

V1.0:
The function sends the command made in the LTC_make_command
function and do the tasks accordingly to the command sent.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void LTC_send_command(LTC_config *config, ...){

	uint16_t tx_data[3] = { 0, 0, 0},
			 rx_data[3] = { 0, 0, 0};

	LTC_sensor *sensor;

	if(!config->command->BROADCAST){

		va_list list;
		va_start(list, 1);

		sensor = va_arg(list, LTC_sensor*);
		config->command->NAME  |= ((sensor->ADDR & (0x1111 * ~config->command->BROADCAST)) | ~config->command->BROADCAST << 4) << 11;

		tx_data[2] |= ((sensor->DCC & 0xff) << 8) | ((sensor->DCC & 0xf00) >> 8);

		va_end(list);
	}

	if((config->command->NAME & 0x07FF) == LTC_COMMAND_WRCFG){

		tx_data[0] = (config->ADCOPT << 8) | (config->SWTRD << 9) | (config->REFON << 10) | (config->GPIO << 11) | (config->VUV);
		tx_data[1] = (config->VUV >> 8) | (config->VOV << 4);
		tx_data[2] |= ((config->DCTO & 0xf) << 4);
		//		tx_data[2] = (config->DCTO << 12) | (config->DCC);

	}

	LTC_transmit_recieve(LTC_make_command(config->command), tx_data, rx_data);


	switch(config->command->NAME & 0x07FF){

	case LTC_COMMAND_RDCFG 	:

		//		config->ADCOPT = rx_data[0] & 0x1;
		//		config->SWTRD = (rx_data[0] >> 1) & 0x1;
		//		config->REFON = (rx_data[0] >> 2) & 0x1;
		//		config->GPIO =  (rx_data[0] >> 3) & 0x1F;
		//		config->VUV =   (rx_data[0] >> 8) | (rx_data[1] & 0x000F);
		//		config->VOV =   (rx_data[1] >> 4);
		//		config->DCC =   (rx_data[2] & 0x0FFF);
		//		config->DCTO =  (rx_data[2] >> 12);

		break;

	case LTC_COMMAND_RDCVA 	:

		sensor->CxV[0] = rx_data[0];
		sensor->CxV[1] = rx_data[1];
		sensor->CxV[2] = rx_data[2];

		break;

	case LTC_COMMAND_RDCVB 	:

		sensor->CxV[3] = rx_data[0];
		if(sensor->ADDR == 1 || sensor->ADDR == 4 || sensor->ADDR == 7){
			sensor->CxV[4] = rx_data[1];
			sensor->CxV[5] = rx_data[2];
		}

		break;

	case LTC_COMMAND_RDCVC 	:

//		if(sensor->ADDR == 1){
//			sensor->CxV[6] = (rx_data[0] + rx_data[1]) / 2;
//			sensor->CxV[7] = sensor->CxV[6];
//		}else{
			sensor->CxV[6] = rx_data[0];
			sensor->CxV[7] = rx_data[1];
//		}
		sensor->CxV[8] = rx_data[2];

		break;

	case LTC_COMMAND_RDCVD 	:

		sensor->CxV[9]  = rx_data[0];
		//sensor->CxV[10] = rx_data[1];
		//sensor->CxV[11] = rx_data[2];

		break;

	case LTC_COMMAND_RDAUXA	:
		if(sensor->ADDR != 1 && sensor->ADDR != 4 && sensor->ADDR != 7){
			sensor->GxV[0] = rx_data[0];
			sensor->GxV[1] = rx_data[1];
			sensor->GxV[2] = rx_data[2];
		}
		break;

	case LTC_COMMAND_RDAUXB	:
		if(sensor->ADDR != 1 && sensor->ADDR != 4 && sensor->ADDR != 7){
			sensor->GxV[3] = rx_data[0];
			sensor->GxV[4] = rx_data[1];
		}
		sensor->REF =    rx_data[2];

		break;

	case LTC_COMMAND_RDSTATA:

		sensor->SOC =  rx_data[0] * 0.2;
		sensor->ITMP = rx_data[1] * 7.5;
		sensor->VA =   rx_data[2];

		break;

	case LTC_COMMAND_RDSTATB:

		sensor->VD = rx_data[0];

		uint32_t flag = rx_data[1] | (uint32_t)rx_data[2] << 16;
		for (int j = 0; j < 12; j++)
			sensor->V_ERROR[j] = (flag >> (j * 2)) & 0x3;

		break;

	case LTC_COMMAND_ADCV	:
	case LTC_COMMAND_ADOW	:
	case LTC_COMMAND_CVST	:
	case LTC_COMMAND_ADAX	:
	case LTC_COMMAND_AXST	:
	case LTC_COMMAND_ADSTAT	:
	case LTC_COMMAND_STATST	:
	case LTC_COMMAND_ADCVAX	:

		config->ADC_READY = FALSE;

		break;

	case LTC_COMMAND_PLADC	:

		if(rx_data[0] == 0 || rx_data[1] == 0 || rx_data[2] == 0)
			config->ADC_READY = FALSE;
		else
			config->ADC_READY = TRUE;
		break;

	case LTC_COMMAND_DIAGN	:

		//transmit_command(config->command);

		break;

	case LTC_COMMAND_WRCOMM	:
		break;
	case LTC_COMMAND_RDCOMM	:
		break;
	case LTC_COMMAND_STCOMM	:
		break;
	default:
		break;
	}
}

/*******************************************************
 Function void LTC_init(LTC_config*)

V1.0:
The function initializes the LTC IC by assigning its initial
configuration.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void LTC_init(LTC_config *config){

	config->GPIO = 0x1F;
	config->REFON = 0;
	config->SWTRD = 0;
	config->ADCOPT = 0;
	config->VUV = 0;
	config->VOV = 0;
	config->DCTO = 0;
	config->command->MD = MD_FILTRED;
	//config->command->DCP = DCP_PERMITED;

	config->command->BROADCAST = TRUE;
	config->command->NAME = LTC_COMMAND_WRCOMM;
	LTC_send_command(config);

}

/*******************************************************
 Function void LTC_balance_test(LTC_config*, LTC_sensor*)

V1.0:
The function is an auxiliary one for balancing the battery's
cells.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void LTC_balance_test(LTC_config *config, LTC_sensor *sensor){

	//TODO implement software balance check
	sensor->DCC = 1;

	while (sensor->DCC != 0){

		config->command->BROADCAST = FALSE;
		config->command->NAME = LTC_COMMAND_WRCFG;
		LTC_send_command(config, sensor);
		sensor->DCC = sensor->DCC << 1;
		DWT_Delay_us(500000);

	}

	sensor->DCC = 0;
	LTC_send_command(config, sensor);
}

//extern int16_t THERMISTOR_ZEROS[N_OF_PACKS][5];

/*******************************************************
 Function void LTC_T_convert(LTC_sensor*)

V1.0:
The function converts the ADC value read by the LTC6804
into a temperature value in °C. It follows the steps described
in the LTC6804 datasheet, page 27.

V2.0:
The function converts the ADC value read by the LTC6804
(on the voltage divider) into a resistance value of the thermistor.
The resistance value is then converted to temperature in °C.
The mathematical expression is presented below:

	1/T = 1/T0 + 1/B * ln(R/R0)

	T is the ambient temperature in Kelvins.
	T0 is the reference temperature, also in Kelvin (usually 25°C = 298.15K).
	B is the beta constant (datasheet).
	R is the thermistor resistance at the ambient temperature.
	R0 is the thermistor resistance at temperature T0.

	T' = 10 * (T - 273)

	T' is the ambient temperature in tenths of Celsius (to improve accuracy)
	Example: T = 298 K => T' = 250 d°C (25,0°C)

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
 Version 2.0 - Initial release 20/10/2021 by Thiago Santos
*******************************************************/
static void LTC_T_convert(LTC_sensor* sensor){
	float r;

	for(uint8_t i = 0; i < N_OF_THERMISTORS; i++){
		if(sensor->GxV[i] > 1000 && sensor->GxV[i] < sensor->REF - 1000){
			r = (sensor->GxV[i]*10000) / (sensor->REF - sensor->GxV[i]);
			sensor->GxV[i] = 10 * (1 / ( 1/(float)t0 + 1/(float)B * log(r/r0) ) - 273);
		}else{
			sensor->GxV[i] = 0;
		}
	}
}

/*******************************************************
 Function void LTC_wait(LTC_config*, LTC_sensor*)

V1.0:
The function is used as an auxiliary function for confirming
whether the ADC is ready or not.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void LTC_wait(LTC_config *config, LTC_sensor *sensor){

	do{

		config->command->NAME = LTC_COMMAND_PLADC;
		config->command->BROADCAST = FALSE;
		LTC_send_command(config, sensor);

	}while(!config->ADC_READY);

}

/*******************************************************
 Function void LTC_read(uint8_t, LTC_config*, LTC_sensor*)

V1.0:
The function reads the values stored in the LTC6804's registers.
Those values can be the cells' voltages, the NTC resistors' tem-
peratures or the IC's status and configuration. Also, it returns
the calculation of the most and the least charged cell and the
difference between them.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void LTC_read(uint8_t LTC_READ, LTC_config *config, LTC_sensor *sensor){

	config->command->BROADCAST = FALSE;

	if (LTC_READ&LTC_READ_CELL) {

		LTC_wait(config, sensor);

		config->command->NAME = LTC_COMMAND_RDCVA;
		LTC_send_command(config, sensor);
		config->command->NAME = LTC_COMMAND_RDCVB;
		LTC_send_command(config, sensor);
		config->command->NAME = LTC_COMMAND_RDCVC;
		LTC_send_command(config, sensor);
		config->command->NAME = LTC_COMMAND_RDCVD;
		LTC_send_command(config, sensor);

		sensor->V_MIN = MAX_CELL_V_DISCHARGE;
		sensor->V_MAX = MIN_CELL_V;

		for(uint8_t i = 0; i < N_OF_CELLS; i++){
			if(sensor->CxV[i] != 0 && sensor->CxV[i] < sensor->V_MIN)
				sensor->V_MIN = sensor->CxV[i];
			if(sensor->CxV[i] > sensor->V_MAX)
				sensor->V_MAX = sensor->CxV[i];
		}

		sensor->V_DELTA = sensor->V_MAX - sensor->V_MIN;

	}
	if (LTC_READ&LTC_READ_GPIO) {

		LTC_wait(config, sensor);

		if(sensor->ADDR != 1 || sensor->ADDR != 4 || sensor->ADDR != 7){

			config->command->NAME = LTC_COMMAND_RDAUXA;
			LTC_send_command(config, sensor);
			config->command->NAME = LTC_COMMAND_RDAUXB;
			LTC_send_command(config, sensor);
			LTC_T_convert(sensor);

		}

	}
	if (LTC_READ&LTC_READ_STATUS) {

		LTC_wait(config, sensor);

		config->command->NAME = LTC_COMMAND_RDSTATA;
		LTC_send_command(config, sensor);
		config->command->NAME = LTC_COMMAND_RDSTATB;
		LTC_send_command(config, sensor);

	}
	if (LTC_READ&LTC_READ_CONFIG){

		LTC_wait(config, sensor);

		config->command->NAME = LTC_COMMAND_RDCFG;
		LTC_send_command(config, sensor);

	}
}

/*******************************************************
 Function void LTC_set_balance_flag(LTC_config*, LTC_sensor*)

V1.0:
The function sets the balance flag if the analyzed cell's voltage
is at a level which it should be balanced.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void LTC_set_balance_flag(LTC_config *config, LTC_sensor *sensor){

	sensor->DCC = 0;

	if(sensor->V_DELTA > 10)
		for (uint8_t i = 0; i < N_OF_CELLS; ++i) {
			if((sensor->CxV[i] - sensor->V_MIN) > sensor->V_DELTA * 0.4){
				sensor->DCC |= 1 << i;
			}
		}

}

/*******************************************************
 Function void LTC_reset_balance_flag(LTC_config*, LTC_sensor*)

V1.0:
The function resets the balance flag set in the LTC_set_balance_flag
function.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void LTC_reset_balance_flag(LTC_config *config, LTC_sensor *sensor){

	sensor->DCC = 0;

	config->command->BROADCAST = FALSE;
	config->command->NAME = LTC_COMMAND_WRCFG;
	LTC_send_command(config, sensor);

}

/*******************************************************
 Function void LTC_balance(LTC_config*, LTC_sensor*)

V1.0:
The function is an auxiliary function for setting up
the cells' balance.

 Version 1.0 - Initial release 01/01/2018 by Tesla UFMG
*******************************************************/
void LTC_balance(LTC_config *config, LTC_sensor *sensor){

	config->command->BROADCAST = FALSE;
	config->command->NAME = LTC_COMMAND_WRCFG;
	LTC_send_command(config, sensor);

}

uint16_t SOC_LOOKUP[11] = {
		28594,
		31845,
		32220,
		32490,
		32705,
		32831,
		32907,
		32999,
		33149,
		33272,
		33865
};


void LTC_SOC(LTC_config *config, LTC_sensor *sensor){

	uint8_t j;

	for(uint8_t i = 0; i < N_OF_CELLS; i++){
		for(j = 0; j < 10; j++){

			if(sensor->CxV[i] > SOC_LOOKUP[10]){
				sensor->CxV[i] = 100;
				break;
			}
			if(sensor->CxV[i] < SOC_LOOKUP[0]){
				sensor->CxV[i] = 0;
				break;
			}
			if(sensor->CxV[i] < SOC_LOOKUP[j]){
				sensor->CHARGE[i] = (j - 1)*100 + ((float)100/(SOC_LOOKUP[j] - SOC_LOOKUP[j - 1])) * (sensor->CxV[i] - SOC_LOOKUP[j - 1]);
				break;
			}

		}

		sensor->CHARGE[i] = (j - 1)*100 + ((float)100/(SOC_LOOKUP[j] - SOC_LOOKUP[j - 1])) * (sensor->CxV[i] - SOC_LOOKUP[j - 1]);

		sensor->TOTAL_CHARGE += sensor->CHARGE[i];
	}

	sensor->TOTAL_CHARGE /= N_OF_CELLS;
	if(sensor->TOTAL_CHARGE >= 1000){
		sensor->TOTAL_CHARGE = 999;
	}

}


void LTC_open_wire(uint8_t LTC_READ, LTC_config *config, LTC_sensor *sensor){

	//	LTC_send_command(LTC_COMMAND_ADOW, FALSE, config, sensor);

}
