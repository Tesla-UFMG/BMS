//#include "obsolete.h"
//
//
//// ********************************************************* BMS.c **********************************************************
//
//// ------------------------------------------ FUNCOES INCOMPLETAS DE DEBBUG - UART (ARDUINO) ----------------------------
//uint8_t BMS_AIR_status(BMS_struct *BMS){
////	HAL_GPIO_WritePin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin, RESET);
//	if (HAL_GPIO_ReadPin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin) == 1)
//		return AIR_CLOSED;
//
//	//HAL_GPIO_WritePin(AIR_AUX_PLUS_GPIO_Port, AIR_AUX_PLUS_Pin, SET);
//	else if (HAL_GPIO_ReadPin(AIR_AUX_MINUS_GPIO_Port, AIR_AUX_MINUS_Pin) == 1)
//		return AIR_CLOSED;
//	else
//		return AIR_OPEN;
//}
//
//int BMS_charging(BMS_struct BMS){
//	//EM CONSTRU��O...
//	return 0;
//}
//
//int BMS_discharging(BMS_struct BMS){
//	//EM CONSTRU��O...
//	return 0;
//}
//
//int BMS_balance(BMS_struct *BMS){
//	for(uint8_t i = 0; i < N_OF_PACKS; i++){
//		if(LTC6804_check_status(BMS->sensor[i])){
//			LTC6804_balance(BMS->sensor[i]);
//		}
//	}
//	//EM CONSTRU��O...
//	return 0;
//}
//
///*int BMS_communication(BMS_struct *BMS){
//	//EM CONSTRU��O...
//
//
//	UART_print("\n\n%d\n\n", BMS->charge);
//	EE_WriteVariable(0x0, (uint16_t) (BMS->charge >> 16));
//	EE_WriteVariable(0x1, (uint16_t) BMS->charge);
//	nextLoop(BMS);
//
//	switch(BMS->communication_mode){
//	case COMM_TC_ONLY:
//		BMS_can(BMS);
//		break;
//
//	case COMM_FULL:
//
//		BMS_can(BMS);
//		if ((HAL_GetTick() - comm_time) > UART_RATE){
//			BMS_uart(BMS);
//			comm_time = HAL_GetTick();
//		}
//		break;
//
//	case COMM_UART_DEBUG:
//
//		if ((HAL_GetTick() - comm_time) > UART_RATE){
//			BMS_uart(BMS);
//			comm_time = HAL_GetTick();
//		}
//
//		break;
//	}
//	return 0;
//}
//*/
//// -------------------------------------------- FIM-FUNCOES DE DEBBUG UART (ARDUINO) - 1 ------------------------------------------
//
//
//// --------------------------------------------FUNCOES DE DEBBUG UART - 2 -----------------------
//
//extern uint8_t max_retries;
//extern uint16_t ADC_BUF[5];
//
//
//void BMS_uart(BMS_struct *BMS){
//
//	HAL_GPIO_WritePin(DEBUG_GPIO_Port,DEBUG_Pin, 1);
//
//
//	UART_print("\n\n\n\n"  );//HEADER
////	UART_print("\n\t\tONLINE PACKS: %d\t\tGLV: %d\t\tCHARGE: %d\t\tERROR:", BMS->opperating_packs, BMS->GLV_voltage , BMS->charge);//HEADER
////	UART_print_error_flag(BMS->error_flag);
//	UART_print("\t\tRETRIES:%d", max_retries);
//	UART_print("\n"  );//HEADER
//
//	for(int i = 0; i < N_OF_PACKS; i++){
//
//		UART_print("\n");
////		UART_print("\t\t\t\t\t\t\t\t   -- PACK %d STATUS: %x --\n", i, BMS->sensor[i]->status);
//		UART_print("\n\t");
//
//	//	if (BMS->sensor[i]->status == STAT_OPPERATING) {
//			for(int j = 0; j < N_OF_CELLS; j++){
//				UART_print("CELL %2d: ", j + 1);
//			UART_print("%d", BMS->sensor[i]->CxV[j]);
//				//UART_print_float((float)BMS->sensor[i]->CxV[j]/10000);
//				//UART_print("%x", BMS->sensor[i]->v_error[j]);
////				UART_print_error_flag(BMS->sensor[i]->v_error[j]);
//				UART_print("\t");
//
//				if ((j + 1) % 6 == 0){
//					UART_print("\n\t");
//				}
//			}
//
//			UART_print("\n\t");
//
//			for(int j = 0; j < N_OF_THERMISTORS; j++){
//				UART_print("TEMP %d: ", j);
////				UART_print_float((float)BMS->sensor[i]->t_cell[j]/1000);
////				UART_print_error_flag(BMS->sensor[i]->t_error[j]);
//				UART_print("\t");
//			}
//
//			UART_print("V PACK %d: ", i);
////			UART_print("%d", BMS->sensor[i]->v_sum);
//			UART_print("\t");
//			UART_print("VAR MAX %d: ", i);
////			UART_print("%d", BMS->sensor[i]->v_var);
//
//			UART_print("\n");
//		//}
//	}
//
//	UART_print("\n\tCURRENT 1(1): ");
//	UART_print_float((float)BMS->current[0]/100);
//	UART_print("\tCURRENT 1(2): ");
//	UART_print_float((float)BMS->current[1]/1000);
//	UART_print("\tCURRENT 2(1): ");
//	UART_print_float((float)BMS->current[2]/100);
//	UART_print("\tCURRENT 2(2): ");
//	UART_print_float((float)BMS->current[3]/1000);
//	UART_print("\n\tCGLV: ");
////	UART_print_float((float)BMS->GLV_voltage/1000);
//	UART_print("\n\t%d \t%d \t%d \t%d", ADC_BUF[0], ADC_BUF[1], ADC_BUF[2], ADC_BUF[3]);
//
////	UART_print("\n\t[UV] times: %d", UV_retries);
//
////	UART_print("\t[OV] times: %d", OV_retries);
//
////	UART_print("\t[OT] times: %d", OT_retries);
//
//	HAL_GPIO_WritePin(DEBUG_GPIO_Port,DEBUG_Pin, 0);
//}
//
//
//uint8_t BMS_mode_comm_navigate(uint8_t mode){
//	switch(mode){
//	case COMM_FULL:
//		return COMM_TC_ONLY;
//		break;
//	case COMM_TC_ONLY:
//		return COMM_UART_DEBUG;
//		break;
//	case COMM_UART_DEBUG:
//		return COMM_FULL;
//		break;
//	default:
//		return OPP_DEFAULT;
//	}
//}
//
//uint8_t mode_display(uint8_t mode){
////	switch(mode){
////	case OPP_BALANCING:
//	//	return b;
////		break;
////	case OPP_CHARGING:
////		return c;
////		break;
////	case OPP_DEFAULT:
////		return d;
////		break;
////	case OPP_TESTING:
////		return t;
////		break;
////	default:
////		return d;
//	}
//}
//
//extern uint8_t mode, accept_time;
//
//
//void BMS_mode_selector(BMS_struct *BMS){
//	if(!HAL_GPIO_ReadPin(MODE_SELECT_GPIO_Port, MODE_SELECT_Pin)){
//		int i = 0, op = 0;
//
////		i2c_transmit(D);
//
//		while (!HAL_GPIO_ReadPin(MODE_SELECT_GPIO_Port, MODE_SELECT_Pin));
//
//		while(1){
//			//---MODE SELECTION ROUTINE---
//
////			i2c_transmit(mode_display(a));
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
//				//---MODE CHANGING ROUTINE--- END
//				}
//
//				//---MODE BLINKING ROUTINE--- END
//			}
//
//			//---MODE SELECTION ROUTINE--- END
//		}
//	}
//}
//
//
//
//// ------------------------------------------- FIM FUNCOES DE DEBBUG UART - 2 -------------------
