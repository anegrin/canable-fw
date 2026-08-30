#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f0xx_hal.h"

#define USB_FLASH_START_ADDRESS 0x8010000
#define TOTAL_USB_DEVICE_SIZE 65536

#ifdef INCLUDE_USER_CONFIG_H
#include "user_config.h"
#endif

#ifndef UART_PIN
//UART_PIN on GPIOA, default is SWDCLK one
#define UART_PIN GPIO_PIN_14
#endif

#ifdef C1CAN
#ifndef RESET_CMD_PIN
//RESET_PIN on RESET_CMD_PIN_PORT, default is SWDIO one
#define RESET_CMD_PIN GPIO_PIN_13
#endif
#endif

#ifndef CAN_S_PIN
//CAN_S_PIN on CAN_S_PIN_PORT
#define CAN_S_PIN GPIO_PIN_13
#endif

#ifndef CAN_S_PIN_PORT
//CAN_S_PIN port, default is GPIOC
#define CAN_S_PIN_PORT GPIOC
#endif

#ifndef LED_TX_Pin
//LED_TX_Pin on GPIOA
#define LED_TX_Pin GPIO_PIN_1
#endif
#ifndef LED_RX_Pin
//LED_RX_Pin on GPIOA
#define LED_RX_Pin GPIO_PIN_0
#endif

#ifndef DISABLE_EXTERNAL_OSCILLATOR
#define ENABLE_EXTERNAL_OSCILLATOR
#endif

#ifndef GIUCAN_VERSION
#ifdef GIT_VERSION
#define GIUCAN_VERSION GIT_VERSION
#else
#define GIUCAN_VERSION "dev"
#endif

#endif

#ifdef SLCAN

#ifdef ELM327
#error "Can't build SLCAN+ELM327"
#endif

#ifdef BHCAN
#error "Can't build SLCAN+BHCAN"
#endif

#ifdef C1CAN
#error "Can't build SLCAN+C1CAN"
#endif

#define LEDS_ON_CAN_RX
#define LEDS_ON_CAN_TX

#endif

#ifdef ELM327

#ifdef BHCAN
#error "Can't build ELM327+BHCAN"
#endif

#ifdef C1CAN
#error "Can't build ELM327+C1CAN"
#endif

#define LEDS_ON_CAN_RX
#define LEDS_ON_CAN_TX

#endif

#ifdef BHCAN

#define XCAN

#ifdef C1CAN
#error "Can't build BHCAN+C1CAN"
#endif

// 0x02=FM radio, 0x03=AM Radio, 0x05=Aux, 0x06=left USB, 0x07=Right USB,
// 0x08=Center USB, 0x09=Bluetooth, 0x12=phone connected, 0x13=phone disconnected,
// 0x15=call in progress, 0x17=call in wait, 0x18=call terminated, 0x11=clear display, ...
#ifndef DISPLAY_INFO_CODE
#define DISPLAY_INFO_CODE 0x08
#endif

// must be a multiple of 3; suggested value for 7 inch is 24, for 3.5 inch is 18
#ifndef DASHBOARD_MESSAGE_MAX_LENGTH
#define DASHBOARD_MESSAGE_MAX_LENGTH 24
#endif

#ifndef DISABLE_DASHBOARD_FORCED_REFRESH
#define DASHBOARD_FORCED_REFRESH
#ifndef DASHBOARD_FORCED_REFRESH_MS
#define DASHBOARD_FORCED_REFRESH_MS 1500
#endif
#endif

#ifndef DASHBOARD_FRAME_QUEUE_POLLING_INTERVAL_MS
#define DASHBOARD_FRAME_QUEUE_POLLING_INTERVAL_MS 29
#endif

#ifndef DISABLE_DPF_REGEN_NOTIFICATION
#define ENABLE_DPF_REGEN_NOTIFICATION

#ifndef DISABLE_DPF_REGEN_SOUND_NOTIFICATION
#define ENABLE_DPF_REGEN_SOUND_NOTIFICATION
#endif

#endif

#ifndef CAN_BITRATE
#define CAN_BITRATE CAN_BITRATE_125K
#endif

#endif

#ifdef C1CAN
#define XCAN

#ifndef DISABLE_DASHBOARD
#define ENABLE_DASHBOARD
#ifndef RES_LONG_PRESS_DURATION_MS
#define RES_LONG_PRESS_DURATION_MS 1000
#endif
#ifndef DASHBOARD_PAGE_SIZE
#define DASHBOARD_PAGE_SIZE 10
#endif
#endif

#ifndef DISABLE_SNS_AUTO_OFF
#define ENABLE_SNS_AUTO_OFF
#ifndef SNS_AUTO_OFF_DELAY_MS
#define SNS_AUTO_OFF_DELAY_MS 20000
#endif
#endif

#ifndef TANK_CAPACITY
#define TANK_CAPACITY 52.0f
#endif

#ifndef SERVICE_INTERVAL
#define SERVICE_INTERVAL 20000.0f
#endif

#ifndef CAR_IS_ON_MIN_RPM
#define CAR_IS_ON_MIN_RPM 400
#endif

#ifndef DEFAULT_VALUES_REFRESH_MS
#define DEFAULT_VALUES_REFRESH_MS 333
#endif

#ifndef DISABLE_DPF_REGEN_NOTIFICATION
#define ENABLE_DPF_REGEN_NOTIFICATION

#ifndef DISABLE_DPF_REGEN_VISUAL_NOTIFICATION
#define ENABLE_DPF_REGEN_VISUAL_NOTIFICATION
#ifndef DPF_REGEN_VISUAL_NOTIFICATION_ITEM
#define DPF_REGEN_VISUAL_NOTIFICATION_ITEM DPF_STATUS_ITEM
#endif
#endif

#endif

#ifndef ENABLE_DASHBOARD
#ifndef ENABLE_SNS_AUTO_OFF
#ifndef ENABLE_DPF_REGEN_VISUAL_NOTIFICATION
#warning "You're building a C1CAN without any active feature :/"
#endif
#endif
#endif

#endif

#ifdef XCAN
#ifndef VALUES_TIMEOUT_MS
#define VALUES_TIMEOUT_MS 30000
#endif

#ifndef STANDBY_DELAY_MS
#define STANDBY_DELAY_MS 60000
#endif

#define PRINTF_INCLUDE_CONFIG_H

#ifndef DEBUG_MODE
#define ENABLE_USB_MASS_STORAGE
#ifdef C1CAN
#define ENABLE_RW_USB_MASS_STORAGE
#endif
#ifdef DISABLE_USB_MASS_STORAGE_ALWAYS_AWAKE
#define USB_MASS_STORAGE_CAN_SLEEP true
#else
#define USB_MASS_STORAGE_CAN_SLEEP false
#endif
#else
#define USB_MASS_STORAGE_CAN_SLEEP false
#endif
#endif

#ifndef USART2_BAUD_RATE
#define USART2_BAUD_RATE 38400
#endif

#ifndef CAN_BITRATE
#define CAN_BITRATE CAN_BITRATE_500K
#endif

#ifdef DEBUG_MODE
#ifdef ENABLE_DASHBOARD
#warning "DEBUG_MODE+ENABLE_DASHBOARD might lead to malfunction because of UART communication"
#endif
#ifdef BHCAN
#warning "DEBUG_MODE+BHCAN might lead to malfunction because of UART communication"
#endif
#endif

#endif /* __MAIN_H */
