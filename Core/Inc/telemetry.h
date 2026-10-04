// Implementation based on:
// https://github.com/plc2man/hobbywing-dual-esc-telemetry
// https://github.com/plc2man/hobbywing-ezrun-max4-hv-telemetry-protocol

#ifndef __TELEMETRY_H__
#define __TELEMETRY_H__

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define DMA_RX_BUF_SIZE  128
#define ESC_FRAME_SIZE   32
#define ESC_FRAME_HEADER 0xFE
#define CRC_DATA_LENGTH  30

void telemetry_IRQ_handler(UART_HandleTypeDef *huart);
void telemetry_init(UART_HandleTypeDef *huart);

#pragma pack(push, 1)
typedef struct {
    uint8_t header;
    uint8_t pad_1[8];
    uint8_t requested_throttle;
    uint8_t accutial_throttle;
    uint8_t rotation_active;
    uint8_t pad_2[1];
    uint16_t motor_speed;
    uint16_t input_voltage;
    uint16_t input_current;
    uint8_t temperature;
    uint8_t pad_3[10];
    uint16_t crc;
} esc_telemetry_frame_t;
#pragma pack(pop)

extern volatile esc_telemetry_frame_t telemetry_frame;
extern volatile uint16_t peak_current;

#endif /* __TELEMETRY_H__ */