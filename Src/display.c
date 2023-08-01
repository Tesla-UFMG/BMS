/*
 * display.c
 *
 *  Created on: 23 de fev de 2023
 *      Author: Alan Franklin & Felipe Telles
 */

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

void display_init()
{
	nexInit();
	NexPageShow(0);
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
		timer_restart(&updateTimer);

		while (!pageMessageReceived)
		{
			if (timer_wait_ms(pageTimeout, 50))
			{
				pageMessageReceived = 0;
				return;
			}
		}

		if (actual_page == PAGE0)
		{
			NexXfloatSetValue(0, BMS->maxCellVoltage/100);
			NexXfloatSetValue(1, BMS->minCellVoltage / 100);
			NexXfloatSetValue(2, 81600);
			NexXfloatSetValue(3, BMS->maxCellTemperature * 10);
			NexVariableSetValue(2, BMS->AIR);
			NexXfloatSetValue(4, ((BMS->current[2] + BMS->current[0])) * 1000);
			NexNumberSetValue(2, BMS->error);
		}
		else if (actual_page == PAGE1)
		{
			NexXfloatSetValue(0, BMS->sensor[0]->CxV[0] / (100));
			NexXfloatSetValue(1, BMS->sensor[0]->CxV[1] / (100));
			NexXfloatSetValue(2, BMS->sensor[0]->CxV[2] / (100));
			NexXfloatSetValue(3, BMS->sensor[0]->CxV[3] / (100));
			NexXfloatSetValue(4, BMS->sensor[0]->CxV[4] / (100));
			NexXfloatSetValue(5, BMS->sensor[0]->CxV[5] / (100));
			NexXfloatSetValue(6, BMS->sensor[0]->CxV[6] / (100));
			NexXfloatSetValue(7, BMS->sensor[0]->CxV[7] / (100));
			NexXfloatSetValue(8, BMS->sensor[0]->CxV[8] / (100));
			NexXfloatSetValue(9, BMS->sensor[0]->CxV[9] / (100));
			NexXfloatSetValue(10,BMS->sensor[0]->CxV[10] / (100));
			NexXfloatSetValue(11,BMS->sensor[0]->CxV[11] / (100));
			NexXfloatSetValue(12, BMS->sensor[0]->GxV[0] * 10);
			NexXfloatSetValue(13, BMS->sensor[0]->GxV[1] * 10);
			NexXfloatSetValue(14, BMS->sensor[0]->GxV[2] * 10);
			NexXfloatSetValue(15, BMS->sensor[0]->GxV[3] * 10);
			NexXfloatSetValue(16, BMS->sensor[0]->GxV[4] * 10);
		}
		else if(actual_page == PAGE2)
		{
            NexXfloatSetValue(0, BMS->sensor[1]->CxV[0]/(100));
            NexXfloatSetValue(1, BMS->sensor[1]->CxV[1]/(100));
            NexXfloatSetValue(2, BMS->sensor[1]->CxV[2]/(100));
            NexXfloatSetValue(3, BMS->sensor[1]->CxV[3]/(100));
            NexXfloatSetValue(4, BMS->sensor[1]->CxV[4]/(100));
            NexXfloatSetValue(5, BMS->sensor[1]->CxV[5]/(100));
            NexXfloatSetValue(6, BMS->sensor[1]->CxV[6]/(100));
            NexXfloatSetValue(7, BMS->sensor[1]->CxV[7]/(100));
            NexXfloatSetValue(8, BMS->sensor[1]->CxV[8]/(100));
            NexXfloatSetValue(9, BMS->sensor[1]->CxV[9]/(100));
            NexXfloatSetValue(10,BMS->sensor[1]->CxV[10]/(100));
            NexXfloatSetValue(11,BMS->sensor[1]->CxV[11]/(100));
            NexXfloatSetValue(12, BMS->sensor[1]->GxV[0]*10);
            NexXfloatSetValue(13, BMS->sensor[1]->GxV[1]*10);
            NexXfloatSetValue(14, BMS->sensor[1]->GxV[2]*10);
            NexXfloatSetValue(15, BMS->sensor[1]->GxV[3]*10);
            NexXfloatSetValue(16, BMS->sensor[1]->GxV[4]*10);
		}
		else if(actual_page == PAGE3)
		{
            NexXfloatSetValue(0, BMS->sensor[2]->CxV[0]/(100));
            NexXfloatSetValue(1, BMS->sensor[2]->CxV[1]/(100));
            NexXfloatSetValue(2, BMS->sensor[2]->CxV[2]/(100));
            NexXfloatSetValue(3, BMS->sensor[2]->CxV[3]/(100));
            NexXfloatSetValue(4, BMS->sensor[2]->CxV[4]/(100));
            NexXfloatSetValue(5, BMS->sensor[2]->CxV[5]/(100));
            NexXfloatSetValue(6, BMS->sensor[2]->CxV[6]/(100));
            NexXfloatSetValue(7, BMS->sensor[2]->CxV[7]/(100));
            NexXfloatSetValue(8, BMS->sensor[2]->CxV[8]/(100));
            NexXfloatSetValue(9, BMS->sensor[2]->CxV[9]/(100));
            NexXfloatSetValue(10,BMS->sensor[2]->CxV[10]/(100));
            NexXfloatSetValue(11,BMS->sensor[2]->CxV[11]/(100));
            NexXfloatSetValue(12, BMS->sensor[2]->GxV[0]*10);
            NexXfloatSetValue(13, BMS->sensor[2]->GxV[1]*10);
            NexXfloatSetValue(14, BMS->sensor[2]->GxV[2]*10);
            NexXfloatSetValue(15, BMS->sensor[2]->GxV[3]*10);
            NexXfloatSetValue(16, BMS->sensor[2]->GxV[4]*10);
		}
		else if(actual_page == PAGE4)
		{
            NexXfloatSetValue(0, BMS->sensor[3]->CxV[0]/(100));
            NexXfloatSetValue(1, BMS->sensor[3]->CxV[1]/(100));
            NexXfloatSetValue(2, BMS->sensor[3]->CxV[2]/(100));
            NexXfloatSetValue(3, BMS->sensor[3]->CxV[3]/(100));
            NexXfloatSetValue(4, BMS->sensor[3]->CxV[4]/(100));
            NexXfloatSetValue(5, BMS->sensor[3]->CxV[5]/(100));
            NexXfloatSetValue(6, BMS->sensor[3]->CxV[6]/(100));
            NexXfloatSetValue(7, BMS->sensor[3]->CxV[7]/(100));
            NexXfloatSetValue(8, BMS->sensor[3]->CxV[8]/(100));
            NexXfloatSetValue(9, BMS->sensor[3]->CxV[9]/(100));
            NexXfloatSetValue(10,BMS->sensor[3]->CxV[10]/(100));
            NexXfloatSetValue(11,BMS->sensor[3]->CxV[11]/(100));
            NexXfloatSetValue(12, BMS->sensor[3]->GxV[0]*10);
            NexXfloatSetValue(13, BMS->sensor[3]->GxV[1]*10);
            NexXfloatSetValue(14, BMS->sensor[3]->GxV[2]*10);
            NexXfloatSetValue(15, BMS->sensor[3]->GxV[3]*10);
            NexXfloatSetValue(16, BMS->sensor[3]->GxV[4]*10);
		}
	}
}
