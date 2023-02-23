#include "display.h"
#include "nextion.h"
#include "stdbool.h"
#include "stm32f1xx.h"
#include "timer_handler.h"

#define DISPLAY_CURRENT_PAGE_COMMAND 0x66

extern UART_HandleTypeDef huart3;

extern uint8_t uart_user_message[256]; /* Buffer received for user access */
extern bool error_flag[NUMBER_OF_ERRORS];

volatile static bool pageMessageReceived;

/* Nextion Variables */
volatile static  NextionPage_e actual_page;
// NextionPage_e

void display_init()
{
	nexInit();
}
void uart3MessageReceived(BMS_struct *BMS)
{

	/* If the message is to change the nextion page */
	if (uart_user_message[0] == DISPLAY_CURRENT_PAGE_COMMAND)
	{
		pageMessageReceived = 1;
		actual_page = (NextionPage_e)uart_user_message[1];
	}
}

int cmpfunc(const void *a, const void *b)
{
	return (*(uint16_t *)a - *(uint16_t *)b);
}
static uint32_t pageTimeout;
static uint32_t updateTimer;
void display_show(BMS_struct *BMS)
{
	if (timer_wait_ms(updateTimer, 500))
	{
		// Command to request current page
		pageMessageReceived = 0;
		timer_restart(&pageTimeout);
		sendCommand("sendme");
		while (!pageMessageReceived)
		{
			if (timer_wait_ms(pageTimeout, 50))
			{
				pageMessageReceived = 0;
				return;
			}
		}
		if (actual_page == 0)
		{
			NexXfloatSetValue(0, actual_page);
			NexXfloatSetValue(1, BMS->minCellVoltage / 100);
			NexXfloatSetValue(2, BMS->tractiveSystemVoltage);
			NexXfloatSetValue(3, BMS->maxCellTemperature * 10);
			NexVariableSetValue(2, BMS->AIR);
			NexXfloatSetValue(4, ((BMS->current[1] + BMS->current[2]) / 2) * 1000);
			NexNumberSetValue(2, BMS->error);
		}
		else if (actual_page == 1)
		{
			NexXfloatSetValue(0, actual_page);
			NexXfloatSetValue(1, (int16_t)BMS->sensor[0]->CxV[1] / (-100));
			NexXfloatSetValue(2, (int16_t)BMS->sensor[0]->CxV[2] / (-100));
			NexXfloatSetValue(3, (int16_t)BMS->sensor[0]->CxV[3] / (-100));
			NexXfloatSetValue(4, (int16_t)BMS->sensor[0]->CxV[4] / (-100));
			NexXfloatSetValue(5, (int16_t)BMS->sensor[0]->CxV[5] / (-100));
			NexXfloatSetValue(6, (int16_t)BMS->sensor[0]->CxV[6] / (-100));
			NexXfloatSetValue(7, (int16_t)BMS->sensor[0]->CxV[7] / (-100));
			NexXfloatSetValue(8, (int16_t)BMS->sensor[0]->CxV[8] / (-100));
			NexXfloatSetValue(9, (int16_t)BMS->sensor[0]->CxV[9] / (-100));
			NexXfloatSetValue(10, (int16_t)BMS->sensor[0]->CxV[10] / (-100));
			NexXfloatSetValue(11, (int16_t)BMS->sensor[0]->CxV[11] / (-100));
			NexXfloatSetValue(12, BMS->sensor[0]->GxV[0] * 10);
			NexXfloatSetValue(13, BMS->sensor[0]->GxV[1] * 10);
			NexXfloatSetValue(14, BMS->sensor[0]->GxV[2] * 10);
			NexXfloatSetValue(15, BMS->sensor[0]->GxV[3] * 10);
			NexXfloatSetValue(16, BMS->sensor[0]->GxV[4] * 10);
		}
	}
	timer_restart(&updateTimer);
}

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
// NexXfloatSetValue(0, BMS->maxCellVoltage/100);
// NexXfloatSetValue(1, BMS->minCellVoltage/100);
// NexXfloatSetValue(2, BMS->tractiveSystemVoltage);
// NexVariableSetValue(2, BMS->AIR);
// NexXfloatSetValue(3, BMS->maxCellTemperature*10);

//	NexPageShow(N_PAGE0);

// HAL_UART_DMAResume(&huart3);
