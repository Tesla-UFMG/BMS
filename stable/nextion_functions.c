


#include "BMS.h"
#include <stdlib.h>
#include "nextion.h"
#include "nextion_functions.h"


extern uint8_t uart_user_message[256];	/* Buffer received for user access */
uint8_t stat = 0;

void uart3MessageReceived(void)
{

	/* If the message is to change the nextion page */
	if(uart_user_message[0] == 0x71 && uart_user_message[5] == 0xFF && uart_user_message[6] == 0xFF && uart_user_message[7] == 0xFF)
	{
		switch(uart_user_message[1])
		{
		case 0:
			actual_page = N_PAGE0;
			NexPageShow(N_PAGE1);
			stat = 0;
			break;

		default:
			if(uart_user_message[1] > 0 && (uart_user_message[1] - 1) < N_OF_PACKS){
				actual_page = uart_user_message[1];
				stat = 0;
				NexPageShow(N_PAGE2);
			}else if(uart_user_message[1] > 0 && ((uart_user_message[1] - N_OF_PACKS - 1) < N_OF_PACKS)){
				actual_page = uart_user_message[1];
				stat = 1;
				NexPageShow(N_PAGE3);
			}
			//		case 5:
			//			actual_page = N_PAGE5;
			//			break;
			//
			//		case 6:
			//			actual_page = N_PAGE6;
			//			break;

		}
	}
}


//uint8_t nexSetPageError(BMS_struct *BMS) {
//
//	uint8_t var01;
//
//	if(BMS->lowest_cell < 2800) {
//		var01 = ;
//	}
////	else if(BMS->highest_cell > 3600) {
////		var01 = nex_id[1];
////	}
////	else if(BMS->temperature_max > 600) {
////		var01 = nex_id[2];
////	}
//	else var01 = 0;
//
//	return var01;
//}

int cmpfunc (const void * a, const void * b) {
	return ( *(uint16_t*)a - *(uint16_t*)b );
}

extern int aux;

void nexLoop(BMS_struct *BMS)
{

	if(NextError[0] == 1){
		NexScrollingTextSetText(0, "Under Voltage");
		NexScrollingTextSetPic(0, 11);
	}
	else if(NextError[1] == 1){
		NexScrollingTextSetText(0, "Over Voltage");
		NexScrollingTextSetPic(0, 11);
	}
	else if(NextError[2] == 1){
		NexScrollingTextSetText(0, "Over Temperature");
		NexScrollingTextSetPic(0, 11);
	}
	else if(NextError[3] == 1){
		NexScrollingTextSetText(0, "Comm Error");
		NexScrollingTextSetPic(0, 11);
	}
	else if(NextError[4] == 1){
		NexScrollingTextSetText(0, "GLV Low Voltage");
		NexScrollingTextSetPic(0, 11);
	}
	else{
		NexScrollingTextSetText(0,"ALL OK!");
		NexScrollingTextSetPic(0, 10);
	}

	NexProgressBarSetValue(0, 50);

	uint16_t buffer[N_OF_CELLS];

	switch(actual_page) /*actual_nextion_page is used to not use NexPageShow() many times*/
	{
	case N_PAGE0:



		NexVariableSetValue(0, BMS->AIR);
		NexNumberSetValue(0, 0);
		NexXfloatSetValue(0, BMS->v_TS/10);
		NexXfloatSetValue(1, BMS->v_GLV/100);
		NexXfloatSetValue(2, (int16_t)BMS->current[0]);
		NexXfloatSetValue(3, (int16_t)BMS->current[1]);
		NexXfloatSetValue(4, HAL_GPIO_ReadPin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin));
		NexXfloatSetValue(5, HAL_GPIO_ReadPin(AIR_AUX_MINUS_GPIO_Port, AIR_AUX_MINUS_Pin));

		NexPictureSetPic(0, 12 + HAL_GPIO_ReadPin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin));
		NexPictureSetPic(1, 12 + HAL_GPIO_ReadPin(AIR_AUX_MINUS_GPIO_Port, AIR_AUX_MINUS_Pin));


		break;

	default:
		if(actual_page - 1 < N_OF_PACKS ){
			for(uint8_t i = 0; i < N_OF_CELLS; i++)
				//buffer[i] = 12000 - (i * 1000);
				buffer[i] = BMS->sensor[actual_page - 1]->CxV[i];

			if(BMS->config->ORDER)
				//qsort(buffer, 12, sizeof(uint16_t), cmpfunc);

			NexVariableSetValue(1,N_OF_PACKS);
			NexNumberSetValue(0,actual_page - 1);

			for(uint8_t i = 0; i < N_OF_CELLS; i++){
				NexXfloatSetValue(i, buffer[i]);
				if(BMS->sensor[actual_page - 1]->DCC & (1 << i))
					NexXfloatSetCollor(i, 65504);
			}

			NexXfloatSetValue(12,BMS->sensor[actual_page - 1]->GxV[4]);
			NexXfloatSetValue(13,BMS->sensor[actual_page - 1]->GxV[3]);
			NexXfloatSetValue(14,BMS->sensor[actual_page - 1]->GxV[2]);
			NexXfloatSetValue(15,BMS->sensor[actual_page - 1]->GxV[1]);
		}else if(actual_page - N_OF_PACKS - 1 < N_OF_PACKS){
			NexVariableSetValue(1,N_OF_PACKS);
			NexNumberSetValue(0,actual_page - N_OF_PACKS - 1);
			NexXfloatSetValue(0, BMS->sensor[actual_page - N_OF_PACKS - 1]->V_MAX);
			NexXfloatSetValue(1, BMS->sensor[actual_page - N_OF_PACKS - 1]->V_MIN);
			NexXfloatSetValue(2, BMS->sensor[actual_page - N_OF_PACKS - 1]->V_DELTA);
			NexXfloatSetValue(3, BMS->sensor[actual_page - N_OF_PACKS - 1]->SOC);
			NexXfloatSetValue(4, BMS->sensor[actual_page - N_OF_PACKS - 1]->ITMP);
			NexXfloatSetValue(5, (int16_t)BMS->current[0]);
			NexXfloatSetValue(6, (int16_t)BMS->current[2]);
			NexXfloatSetValue(7, (int16_t)BMS->current[3]);
			NexXfloatSetValue(8, (int16_t)BMS->current[1]);
			//			BMS->config->ORDER = NexVariableGetValue(0);
			//			NexXfloatSetValue(0, NexVariableGetValue(1));
		}
		break;

	}
}

//void testnexLoop(void) {
//
//	int aux_ten = 0, aux_temp = 0, aux_corrente = 0, aux_ten_total = 0, percent = 0, air = 0, aux_ten1 = 0, aux_txt = 0;
//
//	if(aux_ten>=5000) aux_ten = 0;
//	else aux_ten = aux_ten + 50;
//
//	if(aux_ten1>=500) aux_ten1 = 0;
//	else aux_ten1 = aux_ten1 + 5;
//
//	if(aux_temp>=1000) aux_temp = 0;
//	else aux_temp = aux_temp + 10;
//
//	if(aux_ten_total>=10000) aux_ten_total = 0;
//	else aux_ten_total = aux_ten_total + 100;
//
//	if(aux_corrente>=1000) aux_corrente = 0;
//	else aux_corrente = aux_corrente + 10;
//
//	if(air != 1) air = 1;
//	else air = 0;
//
//	if(percent >= 100) percent = 0;
//	else percent = percent + 1;
//
//	if(aux_txt >= 6) aux_txt = 0;
//	else aux_txt = aux_txt + 1;
//
//	NexProgressBarSetValue(0,percent);
//	NexVariableSetValue(1,aux_txt);
//
//	switch(actual_page) /*actual_nextion_page is used to not use NexPageShow() many times*/
//	{
//	case N_PAGE0:
//
//		NexVariableSetValue(2,air);
//		NexNumberSetValue(0, aux_corrente);
//		NexNumberSetValue(1, percent);
//		NexXfloatSetValue(0, aux_ten1);
//		NexXfloatSetValue(1, aux_ten1);
//		NexXfloatSetValue(2, aux_ten_total);
//		NexXfloatSetValue(3, aux_temp);
//
//		break;
//
//	case N_PAGE1:
//
//		NexXfloatSetValue(0, 1);
//		NexXfloatSetValue(1, 1);
//		NexXfloatSetValue(2, 1);
//		NexXfloatSetValue(3, aux_ten);
//		NexXfloatSetValue(4, aux_ten);
//		NexXfloatSetValue(5, aux_ten);
//		NexXfloatSetValue(6, aux_ten);
//		NexXfloatSetValue(7, aux_ten);
//		NexXfloatSetValue(8, aux_ten);
//		NexXfloatSetValue(9, aux_ten);
//		NexXfloatSetValue(10, aux_ten);
//		NexXfloatSetValue(11, aux_ten);
//		NexXfloatSetValue(12, aux_temp);
//		NexXfloatSetValue(13, aux_temp);
//		NexXfloatSetValue(14, aux_temp);
//		NexXfloatSetValue(15, aux_temp);
//		break;
//
//	case N_PAGE2:
//
//		NexXfloatSetValue(0, 2);
//		NexXfloatSetValue(1, 2);
//		NexXfloatSetValue(2, 2);
//		NexXfloatSetValue(3, aux_ten);
//		NexXfloatSetValue(4, aux_ten);
//		NexXfloatSetValue(5, aux_ten);
//		NexXfloatSetValue(6, aux_ten);
//		NexXfloatSetValue(7, aux_ten);
//		NexXfloatSetValue(8, aux_ten);
//		NexXfloatSetValue(9, aux_ten);
//		NexXfloatSetValue(10, aux_ten);
//		NexXfloatSetValue(11, aux_ten);
//		NexXfloatSetValue(12, aux_temp);
//		NexXfloatSetValue(13, aux_temp);
//		NexXfloatSetValue(14, aux_temp);
//		NexXfloatSetValue(15, aux_temp);
//		break;
//
//	case N_PAGE3:
//
//		NexXfloatSetValue(0, 3);
//		NexXfloatSetValue(1, 3);
//		NexXfloatSetValue(2, 3);
//		NexXfloatSetValue(3, aux_ten);
//		NexXfloatSetValue(4, aux_ten);
//		NexXfloatSetValue(5, aux_ten);
//		NexXfloatSetValue(6, aux_ten);
//		NexXfloatSetValue(7, aux_ten);
//		NexXfloatSetValue(8, aux_ten);
//		NexXfloatSetValue(9, aux_ten);
//		NexXfloatSetValue(10, aux_ten);
//		NexXfloatSetValue(11, aux_ten);
//		NexXfloatSetValue(12, aux_temp);
//		NexXfloatSetValue(13, aux_temp);
//		NexXfloatSetValue(14, aux_temp);
//		NexXfloatSetValue(15, aux_temp);
//		break;
//
//	case N_PAGE4:
//
//		NexXfloatSetValue(0, 4);
//		NexXfloatSetValue(1, aux_ten);
//		NexXfloatSetValue(2, aux_ten);
//		NexXfloatSetValue(3, aux_ten);
//		NexXfloatSetValue(4, aux_ten);
//		NexXfloatSetValue(5, aux_ten);
//		NexXfloatSetValue(6, aux_ten);
//		NexXfloatSetValue(7, aux_ten);
//		NexXfloatSetValue(8, aux_ten);
//		NexXfloatSetValue(9, aux_ten);
//		NexXfloatSetValue(10, aux_ten);
//		NexXfloatSetValue(11, aux_ten);
//		NexXfloatSetValue(12, aux_temp);
//		NexXfloatSetValue(13, aux_temp);
//		NexXfloatSetValue(14, aux_temp);
//		NexXfloatSetValue(15, aux_temp);
//		break;
//
//	case N_PAGE5:
//
//		NexXfloatSetValue(0, 5);
//		NexXfloatSetValue(1, aux_ten);
//		NexXfloatSetValue(2, aux_ten);
//		NexXfloatSetValue(3, aux_ten);
//		NexXfloatSetValue(4, aux_ten);
//		NexXfloatSetValue(5, aux_ten);
//		NexXfloatSetValue(6, aux_ten);
//		NexXfloatSetValue(7, aux_ten);
//		NexXfloatSetValue(8, aux_ten);
//		NexXfloatSetValue(9, aux_ten);
//		NexXfloatSetValue(10, aux_ten);
//		NexXfloatSetValue(11, aux_ten);
//		NexXfloatSetValue(12, aux_temp);
//		NexXfloatSetValue(13, aux_temp);
//		NexXfloatSetValue(14, aux_temp);
//		NexXfloatSetValue(15, aux_temp);
//		break;
//
//	case N_PAGE6:
//
//		NexXfloatSetValue(0, 6);
//		NexXfloatSetValue(1, aux_ten);
//		NexXfloatSetValue(2, aux_ten);
//		NexXfloatSetValue(3, aux_ten);
//		NexXfloatSetValue(4, aux_ten);
//		NexXfloatSetValue(5, aux_ten);
//		NexXfloatSetValue(6, aux_ten);
//		NexXfloatSetValue(7, aux_ten);
//		NexXfloatSetValue(8, aux_ten);
//		NexXfloatSetValue(9, aux_ten);
//		NexXfloatSetValue(10, aux_ten);
//		NexXfloatSetValue(11, aux_ten);
//		NexXfloatSetValue(12, aux_temp);
//		NexXfloatSetValue(13, aux_temp);
//		NexXfloatSetValue(14, aux_temp);
//		NexXfloatSetValue(15, aux_temp);
//		break;
//
//		//  			case PAGE7:
//			//
//			//
//			//  				break;
//
//
//	}
//
//}


