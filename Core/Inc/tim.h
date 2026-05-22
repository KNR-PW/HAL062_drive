/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "pid.h"
/* USER CODE END Includes */

extern TIM_HandleTypeDef htim1;

extern TIM_HandleTypeDef htim2;

extern TIM_HandleTypeDef htim3;

extern TIM_HandleTypeDef htim4;

/* USER CODE BEGIN Private defines */
extern volatile uint16_t last_encoder_cnt; 
extern volatile int16_t current_speed;     
extern volatile int16_t target_speed;
extern volatile float error;
extern uint16_t comm_wchdg;
extern pid_reg PID;
#define PI 3.14159265f
#define TICKS_PER_ROTATION 4096.0f
#define WHEEL_RADIUS_C 15 
#define WHEEL_CIRCUMFERENCE (2.0f * PI * WHEEL_RADIUS_C) 
#define MAX_PWM_STEP 8
#define MAX_STEP 1
extern uint8_t ramp;
// 1. Define the available wheels
#define WHEEL_FRONT_LEFT   0
#define WHEEL_FRONT_RIGHT  1
#define WHEEL_REAR_LEFT    2
#define WHEEL_REAR_RIGHT   3

// =================================================================
// 2. SELECT THE CURRENT WHEEL HERE BEFORE PROGRAMMING
#define CURRENT_WHEEL WHEEL_FRONT_LEFT
// =================================================================

// 3. Automatically calculate the CAN IDs based on the selected wheel
// This creates an offset of 0x10 (16) for each wheel:
// Front Left  base = 0x30 -> Frames: 0x32, 0x33, 0x34
// Front Right base = 0x40 -> Frames: 0x42, 0x43, 0x44
// Rear Left   base = 0x50 -> Frames: 0x52, 0x53, 0x54
// Rear Right  base = 0x60 -> Frames: 0x62, 0x63, 0x64

#define BASE_CAN_ID       (0x30 + (CURRENT_WHEEL * 0x10))

#define CAN_ID_FRAME_1    (BASE_CAN_ID + 0x02) // Speeds
#define CAN_ID_FRAME_2    (BASE_CAN_ID + 0x03) // Target & Kp
#define CAN_ID_FRAME_3    (BASE_CAN_ID + 0x04) // Ki & Kd
/* USER CODE END Private defines */

void MX_TIM1_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

