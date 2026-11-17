#include "telemetry.h"
#include "board_params.h"
#include "can.h"
#include "main.h"
#include "stm32f1xx_hal_gpio.h"
#include "tim.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint8_t rx_dma_buffer[DMA_RX_BUF_SIZE];
static uint16_t rx_read_index = 0;
volatile esc_telemetry_frame_t telemetry_frame;
volatile uint16_t peak_current;
volatile uint16_t avarage_current;

uint16_t calculateCRC16(uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x0001) ? (crc >> 1) ^ 0xA001 : crc >> 1;
        }
    }
    return crc;
}

void telemetry_init(UART_HandleTypeDef *huart) {
    __HAL_UART_CLEAR_OREFLAG(huart);
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
    HAL_GPIO_WritePin(TELEM_DIR_GPIO_Port, TELEM_DIR_Pin, 0);

    HAL_UART_Receive_DMA(huart, rx_dma_buffer, DMA_RX_BUF_SIZE);
}

void read_single_frame() {
    static uint16_t buff[16] = {0};
    static uint8_t cnt = 0;
    uint32_t sum = 0;

    esc_telemetry_frame_t temp_frame;
    uint16_t bytes_to_end = DMA_RX_BUF_SIZE - rx_read_index;

    // Liniarize from circular buffer
    if (bytes_to_end >= ESC_FRAME_SIZE) {
        memcpy(&temp_frame, &rx_dma_buffer[rx_read_index], ESC_FRAME_SIZE);
    } else {
        uint16_t first_part = bytes_to_end;
        uint16_t second_part = ESC_FRAME_SIZE - bytes_to_end;

        memcpy(&temp_frame, &rx_dma_buffer[rx_read_index], first_part);
        memcpy(((uint8_t *)&temp_frame) + first_part, &rx_dma_buffer[0], second_part);
    }

    if (temp_frame.crc != calculateCRC16((uint8_t *)&temp_frame, CRC_DATA_LENGTH)) {
        return;
    }

    telemetry_frame = temp_frame;

    if (ENABLE_TELEMETRY) {
        uint8_t tx_frame[8] = {
            avarage_current >> 8,
            avarage_current & 0xFF,
            telemetry_frame.input_current >> 8,
            telemetry_frame.input_current & 0xFF,
            telemetry_frame.input_voltage >> 8,
            telemetry_frame.input_voltage & 0xFF,
            last_encoder_cnt >> 8,
            last_encoder_cnt & 0xFF,
        };
        CAN_transmit(TELEMETRY_TX_ID, tx_frame, 8);
        peak_current = 0;
    }

    if (telemetry_frame.input_current > peak_current)
        peak_current = telemetry_frame.input_current;
}

void telemetry_IRQ_handler(UART_HandleTypeDef *huart) {
    if (!(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) &&
          __HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE))) {
        return;
    }

    __HAL_UART_CLEAR_IDLEFLAG(huart);

    uint16_t rx_write_index = DMA_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);

    while (rx_read_index != rx_write_index) {
        // Header Sync
        if (rx_dma_buffer[rx_read_index] != ESC_FRAME_HEADER) {
            rx_read_index = (rx_read_index + 1) % DMA_RX_BUF_SIZE;
            continue;
        }
        uint16_t available_bytes = (rx_write_index >= rx_read_index)
                                       ? (rx_write_index - rx_read_index)
                                       : (DMA_RX_BUF_SIZE - rx_read_index + rx_write_index);

        if (available_bytes < ESC_FRAME_SIZE) {
            // Header found, waiting for the rest
            break;
        }

        read_single_frame();
        rx_read_index = (rx_read_index + ESC_FRAME_SIZE) % DMA_RX_BUF_SIZE;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        // 1. Clear error flags (Read SR then DR)
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_NEFLAG(huart);
        __HAL_UART_CLEAR_FEFLAG(huart);

        // 2. Restart circular DMA reception
        HAL_UART_Receive_DMA(huart, rx_dma_buffer, DMA_RX_BUF_SIZE);
    }
}