


#include "display.h"
#include "nextion.h"
#include "stdbool.h"
#include "stm32f1xx.h"

extern UART_HandleTypeDef huart3;

extern uint8_t uart_user_message[256];	/* Buffer received for user access */
extern bool error_flag[NUMBER_OF_ERRORS];

//uint8_t flag_information_to_send = 0;
//NextionPage_e previous_page = PAGE0;
//NextionPage_e actual_page   = PAGE0;
//NextionAdvice_e actual_advice = NO_ADVICE;

void display_init() {
	nexInit();
	NexPageShow(0);
}

void uart3MessageReceived(BMS_struct *BMS)
{

	/* If the message is to change the nextion page */
	if(uart_user_message[0] == 0x71 && uart_user_message[5] == 0xFF && uart_user_message[6] == 0xFF && uart_user_message[7] == 0xFF)
	{
		switch(uart_user_message[1])
		{
		case 0:
			actual_page = N_PAGE0;
			NexPageShow(N_PAGE0);
			break;
		}
	}
}
//		case 50:
//			actual_page = N_PAGE0;
//			NexPageShow(N_PAGE0);
//			break;
//
//		case 51:
//		case 52:
//			BMS->config->ORDER = uart_user_message[1] - 51;
//			break;
//
//		case 53:
//			BMS->mode &= ~BMS_BALANCING;
//			for (int i = 0; i < N_OF_PACKS; ++i) {
//				LTC_reset_balance_flag(BMS->config, BMS->sensor[i]);
//				LTC_balance(BMS->config, BMS->sensor[i]);
//			}
//			break;
//
//		case 54:
//			BMS->mode |= BMS_BALANCING;
//			BMS->discharging = TRUE;
//			break;
//
//		default:
//			if(uart_user_message[1] > 0 && (uart_user_message[1] - 1) < N_OF_PACKS){
//				actual_page = uart_user_message[1];
//				NexPageShow(N_PAGE0);
//			}else if(uart_user_message[1] > 0 && ((uart_user_message[1] - N_OF_PACKS - 1) < N_OF_PACKS)){
//				actual_page = uart_user_message[1];
//				NexPageShow(N_PAGE0);
//			}
//			break;
//			actual_page = N_PAGE0;
//			NexPageShow(N_PAGE0);
//
//			break;
//		}
//	}
//}


int cmpfunc (const void * a, const void * b) {
	return ( *(uint16_t*)a - *(uint16_t*)b );
}

void display_show(BMS_struct *BMS){

	actual_page = N_PAGE0;

	HAL_UART_DMAPause(&huart3);

//	if(error_flag[0]){
//		NexScrollingTextSetText(0, "Under Voltage");
//		NexScrollingTextSetPic(0, 11);
//	}
//	else if(error_flag[1]){
//		NexScrollingTextSetText(0, "Over Voltage");
//		NexScrollingTextSetPic(0, 11);
//	}
//	else if(error_flag[2]){
//		NexScrollingTextSetText(0, "Over Temperature");
//		NexScrollingTextSetPic(0, 11);
//	}
//	else{
//		NexScrollingTextSetText(0,"ALL OK!");
//		NexScrollingTextSetPic(0, 10);
//	}

//	uint16_t buffer[N_OF_CELLS];

//	switch(actual_page) /*actual_nextion_page is used to not use NexPageShow() many times*/
//	{
//	case N_PAGE0:
//
//		NexXfloatSetValue(0, BMS->v_max/10000);
//		NexXfloatSetValue(1, BMS->v_min/10000);
//		NexXfloatSetValue(2, BMS->v_TS/10);
//		NexVariableSetValue(2, BMS->AIR);
//		NexXfloatSetValue(3, BMS->t_max/10);
//		NexNumberSetValue(0, 0);
//		NexXfloatSetValue(1, BMS->v_GLV/100);
//		NexXfloatSetValue(4, 1);
//		NexProgressBarSetValue(0, BMS->charge_percent/10);

//		NexPictureSetPic(0, 12 + HAL_GPIO_ReadPin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin));
//		NexPictureSetPic(1, 12 + HAL_GPIO_ReadPin(AIR_AUX_MINUS_GPIO_Port, AIR_AUX_MINUS_Pin));


//		break;

//	case 50:
//		NexWaveformAdd(1, 0, (int16_t)((float)(BMS->c_adc[0] - 2048) * 256/2048) + 128);
//		NexWaveformAdd(1, 1, (int16_t)((float)(BMS->c_adc[2] - 2048) * 256/2048) + 128);
//		NexWaveformAdd(1, 2, (int16_t)((float)(BMS->c_adc[3] - 2048) * 256/2048) + 128);
//		//NexWaveformAdd(1, 3, (int16_t)((float)(BMS->c_adc[3] - 2048) * 256/2048) + 128);
//		NexXfloatSetValue(0, (int16_t)BMS->current[0]);
//		NexXfloatSetValue(1, (int16_t)BMS->current[2]);
//		NexXfloatSetValue(2, (int16_t)BMS->current[3]);
//		NexXfloatSetValue(3, (int16_t)BMS->current[1]/10);
//
//		break;

//	default:
//		if(actual_page - 1 < N_OF_PACKS ){
//
//			NexProgressBarSetValue(0, BMS->sensor[actual_page - 1]->TOTAL_CHARGE/10);
//
//			for(uint8_t i = 0; i < N_OF_CELLS; i++)
//				buffer[i] = BMS->sensor[actual_page - 1]->CxV[i];
//
//			if(BMS->config->ORDER)
//				qsort(buffer, 12, sizeof(uint16_t), cmpfunc);
//
//			NexVariableSetValue(1,N_OF_PACKS);
//			NexNumberSetValue(0,actual_page - 1);
//
//			for(uint8_t i = 0; i < N_OF_CELLS; i++){
//				NexXfloatSetValue(i, buffer[i]);
//				if((BMS->sensor[actual_page - 1]->DCC & (1 << i)) && !BMS->config->ORDER)
//					NexXfloatSetCollor(i, 65504);
//			}
//
//			NexXfloatSetValue(12,BMS->sensor[actual_page - 1]->GxV[4]);
//			NexXfloatSetValue(13,BMS->sensor[actual_page - 1]->GxV[3]);
//			NexXfloatSetValue(14,BMS->sensor[actual_page - 1]->GxV[2]);
//			NexXfloatSetValue(15,BMS->sensor[actual_page - 1]->GxV[1]);
//
//		}else if(actual_page - N_OF_PACKS - 1 < N_OF_PACKS){
//
//			NexProgressBarSetValue(0, BMS->sensor[actual_page - N_OF_PACKS - 1]->TOTAL_CHARGE/10);
//
//			NexVariableSetValue(1,N_OF_PACKS);
//			NexNumberSetValue(0,actual_page - N_OF_PACKS - 1);
//			NexXfloatSetValue(0, BMS->sensor[actual_page - N_OF_PACKS - 1]->V_MAX);
//			NexXfloatSetValue(1, BMS->sensor[actual_page - N_OF_PACKS - 1]->V_MIN);
//			NexXfloatSetValue(2, BMS->sensor[actual_page - N_OF_PACKS - 1]->V_DELTA);
//			NexXfloatSetValue(3, BMS->sensor[actual_page - N_OF_PACKS - 1]->SOC);
//			NexXfloatSetValue(4, BMS->sensor[actual_page - N_OF_PACKS - 1]->ITMP);
//			NexXfloatSetValue(5, (int16_t)BMS->current[0]);
//			NexXfloatSetValue(6, (int16_t)BMS->current[2]);
//			NexXfloatSetValue(7, (int16_t)BMS->current[3]);
//			NexXfloatSetValue(8, (int16_t)BMS->current[1]);
//
//			//			NexXfloatSetValue(0, NexVariableGetValue(1));
//		}
//		break;
//	}
//	if(flag_information_to_send == 0){
//		NexXfloatSetValue(0, 1);
//		}
//	else if(flag_information_to_send == 1){
//			NexXfloatSetValue(1, BMS->v_min/10000);
//		}
//	else if(flag_information_to_send == 2){
//			NexXfloatSetValue(2, BMS->v_TS/10);
//		}
//		else if(flag_information_to_send == 3){
//			NexVariableSetValue(2, 1);
//		}
//		else if(flag_information_to_send == 4){
//			NexXfloatSetValue(3, BMS->t_max/10);
//		}
//		else{
//			flag_information_to_send = -1;
//		}
//		flag_information_to_send++;
	NexXfloatSetValue(0, BMS->maxCellVoltage/100);
	NexXfloatSetValue(1, BMS->minCellVoltage/100);
	NexXfloatSetValue(2, BMS->tractiveSystemVoltage);
	NexVariableSetValue(2, BMS->AIR);
	NexXfloatSetValue(3, BMS->maxCellTemperature*10);

//	NexPageShow(N_PAGE0);

	HAL_UART_DMAResume(&huart3);
}


