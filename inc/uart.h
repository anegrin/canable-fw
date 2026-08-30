#ifndef _UART_H
#define _UART_H

#include <stdbool.h>
#include "config.h"
#include "stm32f0xx_hal.h"
#include "model.h"

#if defined(SLCAN) || defined(ELM327)
#ifdef DEBUG_MODE
#define UART_QUEUE_SIZE 8
#define MESSAGE_SIZE 128
#else
// Minimal values: standalone USB CDC variants do not use UART unless debugging.
#define UART_QUEUE_SIZE 1
#define MESSAGE_SIZE 3
#endif
#endif

#ifdef C2CAN
#define UART_QUEUE_SIZE 1
#define MESSAGE_SIZE 3
#endif

#ifdef XCAN
#define UART_QUEUE_SIZE 64
#define MESSAGE_SIZE 12
#endif

#ifdef C1CAN
bool send_state(GlobalState *state);
#endif
#ifdef SLCAN
#ifdef DEBUG_MODE
uint8_t print_to_uart(char* message);
uint8_t printf_to_uart(const char* format, ...);
#endif
#endif
void uart_init(void);
void uart_deinit(void);
void uart_process(GlobalState *state);

#endif // _UART_H
