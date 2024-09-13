/*
 *      Comunicação via FDCAN - FDCAN.c
 *
 *      Data: 01 de novembro, 2023
 *      Autor: Gabriel Luiz
 *      Contato: (31) 97136-4334 || gabrielluiz.eletro@gmail.com
 */
/*
 * 		- Links Úteis -
 *
 *      DATASHEET: https://www.st.com/content/ccc/resource/training/technical/product_training/group0/35/ed/76/ef/91/30/44/f7/STM32H7-Peripheral-Flexible_Datarate_Controller_Area_Network_FDCAN/files/STM32H7-Peripheral-Flexible_Datarate_Controller_Area_Network_FDCAN.pdf/_jcr_content/translations/en.STM32H7-Peripheral-Flexible_Datarate_Controller_Area_Network_FDCAN.pdf
 *      FDCAN normal DOCUMENTO: https://controllerstech.com/fdcan-normal-mode-stm32/
 *      FDCAN normal mode VÍDEO: https://www.youtube.com/watch?v=sY1ie-CnOR0&t=7s
 */

#include <CAN.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

extern CAN_HandleTypeDef hcan; /* Variável externa de configuração da CAN */
/* USER CODE END EV */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

CAN_HandleTypeDef *hCAN = &hcan; /* Handler de configuração da CAN */

#define TIME_TO_BREAK 10 /* Tempo máximo para envio de mensagem na CAN (ms) */

#define INT_SIZE 4

#define FLOAT_SIZE 5

#define DOUBLE_SIZE 8

/* USER CODE END PD */

/* External functions ------------------------------------------------------------*/
/* USER CODE BEGIN EF */

extern void Error_Handler(); /* Função utilizada para tratamento de erros */

/* USER CODE END EF */

/* Private variables --------------------------------------------------------*/
/* USER CODE BEGIN PV */
CAN_Buffer_t CAN_stream; /*Vetor para armazenamento de
 todos os dados da CAN*/

CAN_TxHeaderTypeDef TxHeader; /*Struct de armazenamento temporario de
 de informações e dados para envio na CAN - Não inclui os dados */

CAN_RxHeaderTypeDef RxHeader; /*Struct de armazenamento temporario de
 de informações recebidas pela CAN - Não inclui os dados */

uint8_t RxData[8]; /*Vetor para armazenamento temporario de dados recebidos
 pela CAN*/

uint32_t TxMailBox;

CAN_FilterTypeDef sFilterConfig;

uint16_t CONT_Telemetry;
uint16_t CONT_Datalogger;
uint16_t CONT_BMS;
uint16_t CONT_ECU;

uint8_t CONT_DEBUG = 0;

FDCAN_StatusTypedef FDCAN_Status = FDCAN_RESET;
/* USER CODE END PV */

/* Private functions ------------------------------------------------------------*/
/* USER CODE BEGIN PF */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*---- CALLBACK DE RECEBIMENTO DA CAN --------------------------------------------------------------------------------*/

/**
 * @brief  Função chamada quando detectado uma mensagem no barramento da CAN
 * @param  hcan: Handle da CAN || normalmente "hcan1"
 * @param  RxFifo0ITs: FIFO em que foi detectado a mensagem
 * @retval ***NONE***
 */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {

//	/* Pisca o  LED 2 caso tenha algo para receber pela CAN */
#ifdef CHIP_LED_DEBUG
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
#endif // CHIP_LED_DEBUG

	/* Pega as informações e dados da CAN, e armazena respectivamente em RxHeader e RxData */
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData);
	/* USER CODE BEGIN: CAN MSG */

	/* USER CODE END: CAN MSG */

	/* Chama a função de tratamento de dados */
	CAN_Stream_ReceiveCallback(&RxHeader, RxData);

	/* Ativa novamente a notificação para caso haja algo a receber */
	HAL_CAN_ActivateNotification(hCAN, CAN_IT_RX_FIFO0_MSG_PENDING);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- TRATAMENTO DE MENSAGENS RECEBIDAS -----------------------------------------------------------------------------*/

/**
 * @brief  Função de tratamento das mensagens recebidas
 * @param  hRxFDCAN: Handler com as innformações do flame recebido
 * @param  Buffer: Buffer com os dados e informações da mensagem
 * @retval Status de execução da função
 */
FDCAN_StatusTypedef CAN_Stream_ReceiveCallback(CAN_RxHeaderTypeDef *hRxCAN,
		uint8_t *Buffer) {
	/* Caso o indentificador não faça parte dos ID's utilizados a função quebra */
	if (hRxCAN->StdId > CAN_IDS_NUMBER)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;

	/* Variavel para armazenamento do tamanho dos dados */
	uint8_t SIZE_DATA = hRxCAN->DLC;

	for (int i = SIZE_DATA; i < 8; i++)
		CAN_stream.Data_buf[hRxCAN->StdId][i] = 0;

	if (SIZE_DATA <= INT_SIZE)
		SIZE_DATA = INT_SIZE;

	/*Switch case para armazenar os dados de forma correta na memória*/
	switch (SIZE_DATA) {
	case 0:
		/*Tratamento de valores inteiros positivos*/
		CAN_Storage_INT(hRxCAN->StdId, Buffer);
		break;
	case INT_SIZE:
		/*Tratamento de valores inteiros positivos*/
		CAN_Storage_INT(hRxCAN->StdId, Buffer);
		break;
	case FLOAT_SIZE:
		/*Tratamento de valores floats*/
		CAN_Storage_FLOAT(hRxCAN->StdId, Buffer);
		break;
	case DOUBLE_SIZE:
		/*Tratamento de valores doubles*/
		CAN_Storage_DOUBLE(hRxCAN->StdId, Buffer);
		break;
	default:
		break;
	}

	for (int i = 0; i < 8; i++)
		Buffer[i] = 0;

	/* Caso de tudo certo, retorna ok */
	return FDCAN_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- PARAMETROS DE CONFIGURAÇÃO DA CAN --------------------------------------------------------------*/

/**
 * @brief  Configura a CAN, overwrite das configurações do .IOC
 * @param  ***NONE***
 * @retval Status de execução da função
 */
FDCAN_StatusTypedef CAN_Configure_Init() {
	/* Configura os parâmetros da CAN - LEITURA DO RELATÓRIO */
	hCAN->Instance = CAN1;
	hCAN->Init.Prescaler = 4;
	hCAN->Init.Mode = CAN_MODE_NORMAL;
	hCAN->Init.SyncJumpWidth = CAN_SJW_4TQ;
	hCAN->Init.TimeSeg1 = CAN_BS1_7TQ;
	hCAN->Init.TimeSeg2 = CAN_BS2_1TQ;
	hCAN->Init.TimeTriggeredMode = DISABLE;
	hCAN->Init.AutoBusOff = DISABLE;
	hCAN->Init.AutoWakeUp = DISABLE;
	hCAN->Init.AutoRetransmission = ENABLE;
	hCAN->Init.ReceiveFifoLocked = DISABLE;
	hCAN->Init.TransmitFifoPriority = DISABLE;

	/* Inicializa a CAN com os parâmetros definidos */
	if (HAL_CAN_Init(hCAN) != HAL_OK)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;
	else
		/* Caso de tudo certo, retorna ok */
		return FDCAN_OK;

	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;
	else
		/* Caso de tudo certo, retorna ok */
		return FDCAN_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- LIMPEZA DOS BUFFERS DE ARMAZENAMENTO --------------------------------------------------------------*/

/**
 * @brief  Inicialização dos buffers de armazenamento das mensagens da CAN
 * @param  ***NONE***
 * @retval ***NONE***
 */
void CAN_Clean_Buffers(void) {
	/* Zera cada posição do vetor de dados - Redundância */
	for (uint16_t i = 0; i < CAN_IDS_NUMBER; i++) {
		/*Limpa os lixos de memória que podem existir*/
		for (uint8_t j = 0; j < 8; j++)
			CAN_stream.Data_buf[i][j] = 0U;
		/*Garante que não seja possível recuperar valores que não foram alocados na memória*/
		//		CAN_stream->Type_buf[i] = FDCAN_FREE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- INÍCIO DA COMUNICAÇÃO VIA FDCAN --------------------------------------------------------------*/

/**
 * @brief  Inicialização da comunicação via FDCAN
 * @param  ***NONE***
 * @retval Status de execução da função
 */
FDCAN_StatusTypedef CAN_Init(void) {
	/* Chama a função de limpeza do vetor de armazenamento de dados */
	CAN_Clean_Buffers();

	/* Chama a função de configuração dos parâmetros da CAN */
	if (CAN_Configure_Init() != FDCAN_OK)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;

	/* Começa a comunicação via CAN */
	if (HAL_CAN_Start(&hcan) != HAL_OK)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;

	/* Ativa a notificação para caso haja algo a receber */
	if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING)
			!= HAL_OK)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;

	/* Configura os parametros para envio de mensagem */
	TxHeader.ExtId = 0U;  					  // IDENTIFICADOR EXTENDED
	TxHeader.StdId = 0U;					  // IDENTIFICADOR STANDARD
	TxHeader.IDE = CAN_ID_STD;	// TIPO DE IDENTIFICADOR - STANDARD OU EXTENDED
	TxHeader.RTR = CAN_RTR_DATA;			  // TIPO DE FLAME - DATA OU REMOTE
	TxHeader.DLC = 8;		  				 // TAMANHO DOS DADOS - 0 A 64 Bytes
	TxHeader.TransmitGlobalTime = DISABLE;

	/* Caso de tudo certo, retorna ok */
	return FDCAN_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ARMAZENAMENTO DE VALORES INTEIROS POSITIVOS --------------------------------------------------------------*/

/**
 * @brief  Função para armazenamento de valores inteiros positivos, assim como informação de tipo e tamanho
 * @param  Identifier: Identificador da mensagem
 * @param  Size: Espaço necessário para armazenamento da mensagem
 * @param  Buffer: Ponteiro para os buffer que contém os dados e as informações para seu armazenamento
 * @retval ***NONE***
 */
void CAN_Storage_INT(uint16_t Identifier, uint8_t *Buffer) {
	/* Armazenando o tipo da variável no buffer da CAN */
	CAN_stream.Type_buf[Identifier] = FDCAN_INT;
	/* Armazena o valor na memória alocada, sendo necessário armazenar byte por byte*/
	for (int i = 0; i < INT_SIZE; i++)
		CAN_stream.Data_buf[Identifier][i] = Buffer[i];

	/* Armazena o valor na memória alocada, sendo necessário armazenar byte por byte*/
	for (int i = INT_SIZE; i < 8; i++)
		CAN_stream.Data_buf[Identifier][i] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ARMAZENAMENTO DE VALORES INTEIROS NEGATIVOS --------------------------------------------------------------*/

/**
 * @brief  Função para armazenamento de valores inteiros negativos, assim como informação de tipo e tamanho
 * @param  Identifier: Identificador da mensagem
 * @param  Size: Espaço necessário para armazenamento da mensagem
 * @param  Buffer: Ponteiro para os buffer que contém os dados e as informações para seu armazenamento
 * @retval ***NONE***
 */
__weak void CAN_Storage_NEGATIVE(uint16_t Identifier, uint8_t *Buffer) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ARMAZENAMENTO DE VALORES FLOATS --------------------------------------------------------------*/

/**
 * @brief  Função para armazenamento de valores floats, assim como informação de tipo e tamanho
 * @param  Identifier: Identificador da mensagem
 * @param  Size: Espaço necessário para armazenamento da mensagem
 * @param  Buffer: Ponteiro para os buffer que contém os dados e as informações para seu armazenamento
 * @retval ***NONE***
 */
void CAN_Storage_FLOAT(uint16_t Identifier, uint8_t *Buffer) {
	/* Armazenando o tipo da variável no buffer da CAN */
	CAN_stream.Type_buf[Identifier] = FDCAN_FLOAT;
	/* Armazena o valor na memória alocada, sendo necessário armazenar byte por byte*/
	for (int i = 0; i < FLOAT_SIZE; i++)
		CAN_stream.Data_buf[Identifier][i] = Buffer[i];

	/* Armazena o valor na memória alocada, sendo necessário armazenar byte por byte*/
	for (int i = FLOAT_SIZE; i < 8; i++)
		CAN_stream.Data_buf[Identifier][i] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ARMAZENAMENTO DE VALORES DOUBLES --------------------------------------------------------------*/

/**
 * @brief  Função para armazenamento de valores doubles, assim como informação de tipo e tamanho
 * @param  Identifier: Identificador da mensagem
 * @param  Size: Espaço necessário para armazenamento da mensagem
 * @param  Buffer: Ponteiro para os buffer que contém os dados e as informações para seu armazenamento
 * @retval ***NONE***
 */
void CAN_Storage_DOUBLE(uint16_t Identifier, uint8_t *Buffer) {
	/* Armazenando o tipo da variável no buffer da CAN */
	CAN_stream.Type_buf[Identifier] = FDCAN_DOUBLE;
	/* Armazena o valor na memória alocada, sendo necessário armazenar byte por byte*/
	for (int i = 0; i < DOUBLE_SIZE; i++)
		CAN_stream.Data_buf[Identifier][i] = Buffer[i];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ACESSO AOS VALORES INTEIROS ARMAZENADOS --------------------------------------------------------------*/

/**
 * @brief  Função para acesso aos valores inteiros armazenados na memória
 * @param  Identifier: Identificador da mensagem
 * @retval Valor inteiro armazenado, caso o valor não seja um inteiro retorna "FDCAN_ERRO"
 */
int32_t CAN_Get_value(uint16_t Identifier) {
	if (CAN_stream.Type_buf[Identifier] != FDCAN_INT)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;

	/*Variável para armazenar o valor guardado na memória*/
	int32_t VALUE = 0;

	memcpy(&VALUE, CAN_stream.Data_buf[Identifier], 4);

	/*Retorna o valor armazenado na memória já com o sinal correto*/
	return VALUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ACESSO AOS VALORES FLOATS ARMAZENADOS --------------------------------------------------------------*/

/**
 * @brief  Função para acesso aos valores floats armazenados na memória
 * @param  Identifier: Identificador da mensagem
 * @retval Valor float armazeenado, caso o valor não seja um float retorna "FDCAN_ERRO"
 */
float CAN_Get_value_FLOAT(uint16_t Identifier) {
	if (CAN_stream.Type_buf[Identifier] != FDCAN_FLOAT)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;

	float VALUE = 0;

	/*Recupera os dados armazenados na memória*/
	memcpy(&VALUE, CAN_stream.Data_buf[Identifier], 4);

	return VALUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ACESSO AOS VALORES DOUBLES ARMAZENADOS --------------------------------------------------------------*/

/**
 * @brief  Função para acesso aos valores doubles armazenados na memória
 * @param  Identifier: Identificador da mensagem
 * @retval Valor double armazenado, caso o valor não seja um double retorna "FDCAN_ERRO"
 */
double CAN_Get_value_DOUBLE(uint16_t Identifier) {
	if (CAN_stream.Type_buf[Identifier] != FDCAN_DOUBLE)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;

	double VALUE = 0;

	/*Recupera os dados armazenados na memória*/
	memcpy(&VALUE, CAN_stream.Data_buf[Identifier], 8);

	return VALUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- INCLUSÃO DE MENSAGENS NO BUFFER DE ENVIO --------------------------------------------------------------*/

/**
 * @brief  Função para adicionar uma mensagem no buffer de envio
 * @param  Identifier: Identificador da mensagem
 * @param  Buffer: Buffer de dados para envio
 * @param  Size: Quantidade de byte para envio
 * @retval Status de execução da função
 */
FDCAN_StatusTypedef CAN_TxData(uint16_t Identifier, uint64_t Buffer,
		uint8_t Size) {
	uint64_t TIME = HAL_GetTick();
	uint64_t *pBuffer = &Buffer;

	//	if (Size == INT_SIZE) {
	//		for (int i = 0; i < 4; i++)
	//			if (Buffer >> 8 * i == 0) {
	//				SIZE_DATA = i << 16U;
	//				break;
	//			}
	//	}

	/* Armazena o identificador da mensagem no struct de informação (TxHeader) */
	TxHeader.StdId = Identifier;

	TxHeader.DLC = Size;

	while (HAL_CAN_GetTxMailboxesFreeLevel(hCAN) == 0)
		if (HAL_GetTick() - TIME > TIME_TO_BREAK)
			return FDCAN_TIMEOUT;

	/* Envia os dados recebidos na chamada (data) pela CAN, de acordo com as informações de TxHeader */

	if (HAL_CAN_AddTxMessage(hCAN, &TxHeader, (uint8_t*) pBuffer, &TxMailBox) != HAL_OK)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;

	/* Caso de tudo certo, retorna ok */
	return FDCAN_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- TRATAMENTO E ENVIO DE INTEIROS --------------------------------------------------------------*/

/**
 * @brief  Função para tratamento e envio de valores inteiros pelo barramento
 * @param  Identifier: Identificador da mensagem
 * @param  Value: Valor inteiro para envio pelo barrameto
 * @retval Status de execução da função
 */
FDCAN_StatusTypedef CAN_Send(uint16_t Identifier, int32_t Value) {
	uint32_t Value_Buf = 0;

	memcpy(&Value_Buf, &Value, 4);

	if (CAN_TxData(Identifier, Value_Buf, INT_SIZE) != FDCAN_OK)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;

	/* Caso de tudo certo, retorna ok */
	return FDCAN_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- TRATAMENTO E ENVIO DE FLOATS --------------------------------------------------------------*/

/**
 * @brief  Função para tratamento e envio de valores inteiros pelo barramento
 * @param  Identifier: Identificador da mensagem
 * @param  Value: Valor float para envio pelo barrameto
 * @retval Status de execução da função
 */
FDCAN_StatusTypedef CAN_Send_Float(uint16_t Identifier, float Value) {

	uint32_t Value_Buf = 0;

	memcpy(&Value_Buf, &Value, 4);

	if (CAN_TxData(Identifier, Value_Buf, FLOAT_SIZE) != FDCAN_OK)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;

	/* Caso de tudo certo, retorna ok */
	return FDCAN_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- TRATAMENTO E ENVIO DE DOUBLES --------------------------------------------------------------*/

/**
 * @brief  Função para tratamento e envio de valores inteiros pelo barramento
 * @param  Identifier: Identificador da mensagem
 * @param  Value: Valor double para envio pelo barrameto
 * @retval Status de execução da função
 */
FDCAN_StatusTypedef CAN_Send_Double(uint16_t Identifier, double Value) {
	uint64_t Value_Buf = 0;

	memcpy(&Value_Buf, &Value, 8);

	if (CAN_TxData(Identifier, Value_Buf, DOUBLE_SIZE) != FDCAN_OK)
		/* Caso de errado, retorna erro */
		return FDCAN_ERROR;

	/* Caso de tudo certo, retorna ok */
	return FDCAN_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* USER CODE END PF */
