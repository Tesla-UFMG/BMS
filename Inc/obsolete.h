// This module contains all the obsolete functions of old implementations and also some debbug functions

// Human comments are those that use two sequential bars.
#ifndef OBSOLETE_H
#define OBSOLETE_H

#include "BMS.h"


// ***************************************** INICIO DO MODULO **********************************************************************

//Date : 23/03/2020

// ************************************************* BMS  ***********************************************************************

//------------------------------------------------- BMS_monitoring ----------------------------------------------------------------


// Antecessor: EE_WriteVariable(0x5, (uint16_t) BMS->charge_max);
// Funcao Originaria: void BMS_monitoring(BMS_struct *BMS)

/*   - CODE -

//	if (BMS->error_flag != ERR_NO_ERROR) {
	//		LTC_convert_fuses();
	//
	//		for(i = 0; i < N_OF_PACKS; i++){
	//
	//			if (BMS->sensor[i]->status == STAT_OPPERATING) {
	//
	//				if(LTC_check_fuses(BMS->sensor[i]) == ERR_OPEN_FUSES)
	//					BMS->error_flag |= ERR_OPEN_FUSES;
	//				//			else
	//				//				BMS->error_flag &= ~ERR_OPEN_FUSES;
	//			}
	//
	//		}
	//	}

	//BMS->opperating_packs = opperating_packs;

	//BMS->AIR = BMS_AIR_status(BMS);
*/

//Sucessor: uint16_t flag = 0;

//-------------------------------------------------- FIM BMS-MONITORING --------------------------------------------------------


// ------------------------------------------ FUNCOES INCOMPLETAS DE DEBBUG - UART (ARDUINO) ----------------------------

// Antecessor: "}" CHAVE DE FECHAMENTO DA FUNCAO BMS_monitoring
// Função: Debbug de funcoes via UART


uint8_t BMS_AIR_status(BMS_struct *BMS);

int BMS_charging(BMS_struct BMS);

int BMS_discharging(BMS_struct BMS);

//Faz o balanceamento das celulas
int BMS_balance(BMS_struct *BMS);

int BMS_communication(BMS_struct *BMS);

//Sucesssor: uint16_t flag = 0;

// -------------------------------------------- FIM-FUNCOES DE DEBBUG UART (ARDUINO) - 1 ------------------------------------------

// -------------------------------------------- CLASSIFICACAO DE ERROS ----------------------------
//Antecessor: else if(OV_retries == 0){
//NextError[1] = 0;
//BMS->error &= ~ERR_OVER_VOLTAGE;
//}
//Funcao: Classificacao e contagem de erros.

/* CODE
//	if(OT_retries == 5){
	//		NextError[2] = 1;
	//		BMS->error_flag |= ERR_OVER_TEMPERATURE;}
	//	else if(OT_retries == 0){
	//		NextError[2] = 0;
	//		BMS->error_flag &= ~ERR_OVER_TEMPERATURE;}

	//	if(BMS->opperating_packs < N_OF_PACKS){
	//		NextError[3] = 1;
	//		BMS->error |= ERR_COMM_ERROR;}  //SET FLAG
	//	else{
	//		NextError[3] = 0;
	//		BMS->error &= ~ERR_COMM_ERROR; //RESET FLAG
	//}
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

*/
// ------------------------------------------ FIM CLASSIFICACAO DE ERROS ------------------------


// -------------------------------------------- FUNCOES DE DEBBUG UART (ARDUINO) - 2 ------------------------------------------

//Antecessor: CAN_Transmit(can_buffer, 54);
//}

//Funcao: Debbug BMS via uart

void BMS_uart(BMS_struct *BMS);

uint8_t BMS_mode_comm_navigate(uint8_t mode);

uint8_t mode_display(uint8_t mode);

void BMS_mode_selector(BMS_struct *BMS);


// -------------------------------------------- FIM-FUNCOES DE DEBBUG UART (ARDUINO) - 2 ------------------------------------------

#endif //End OBSOLETE_H
