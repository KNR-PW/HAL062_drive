/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#define LEFT_SIDE 0
#define RIGHT_SIDE 1
#define BOARD_SIDE RIGHT_SIDE


static CAN_HandleTypeDef *can_handle = &hcan;

static CAN_TxHeaderTypeDef can_txHeader;
static CAN_RxHeaderTypeDef can_rxHeader;
static uint32_t can_txMailbox;

static union Message RX_payload;

//static void CAN_recivedCallback(CAN_HandleTypeDef *hcan);
//static void CAN_errorCallback(CAN_HandleTypeDef *hcan);

uint8_t target_speed = 0;

void CAN_init(void) {
	// setup TX header
	can_txHeader.StdId = 0;
	can_txHeader.IDE = CAN_ID_STD;
	can_txHeader.RTR = CAN_RTR_DATA;
	can_txHeader.DLC = 8;

	// setup filter 
	CAN_FilterTypeDef sFilterConfig;
	sFilterConfig.FilterBank = 0u;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
	sFilterConfig.FilterIdHigh = (0x14 << 5); // Pierwszy filtr na ID 0x14 (przesunięcie o 5 bitów)
	sFilterConfig.FilterIdLow = (0x16 << 5); // Drugi filtr na ID 0x15 (przesunięcie o 5 bitów)
	sFilterConfig.FilterMaskIdHigh = (0xFE << 5); // Maska dla obu filtrów (ignoruje ostatni bit)
	sFilterConfig.FilterMaskIdLow = (0xFE << 5);
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 0;
	HAL_CAN_ConfigFilter(can_handle, &sFilterConfig);

	//HAL_CAN_RegisterCallback(can_handle, HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID, CAN_recivedCallback);
	//HAL_CAN_RegisterCallback(can_handle, HAL_CAN_ERROR_CB_ID, CAN_errorCallback);

	HAL_CAN_ActivateNotification(can_handle, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR);

	HAL_CAN_Start(can_handle);

	if ((can_handle->State != HAL_CAN_STATE_READY) && 
		(can_handle->State != HAL_CAN_STATE_LISTENING)) {
		// TODO errorHandler();	
	}
}
/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */
  CAN_init();
  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void CAN_transmit(uint8_t id, uint8_t *data, uint8_t data_lenght) {
	if (HAL_CAN_GetTxMailboxesFreeLevel(can_handle) == 0) return;

	can_txHeader.StdId = id;
	can_txHeader.DLC = data_lenght;

	HAL_CAN_AddTxMessage(can_handle, &can_txHeader, data, &can_txMailbox);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &can_rxHeader, RX_payload.u8);

	switch (can_rxHeader.StdId) {
	case 20:
		if (BOARD_SIDE == LEFT_SIDE)
			target_speed = -((int8_t) RX_payload.u8[0]);
		else
			target_speed = ((int8_t) RX_payload.u8[1]);
		break;

	// case 51:
	// 	TP = RX_payload.f32[0];
	// 	break;
	// case 52:
	// 	PID_K = RX_payload.f32[0];
	// 	break;
	// case 53:
	// 	PID_TD = RX_payload.f32[0];
	// 	break;
	// case 54:
	// 	PID_TI = RX_payload.f32[0];
	// 	break;

	default:
		break;
	}
	//LED_TOGGLE(LED_STATUS);
	//__HAL_TIM_SET_COUNTER(tim_comwdg, 0);
}
/* USER CODE END 1 */
