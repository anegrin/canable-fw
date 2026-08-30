/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "usbd_cdc_if.h"
#include "error.h"
#include "slcan.h"
#ifdef ELM327
#include "elm327.h"
#endif

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */
USBD_CDC_LineCodingTypeDef LineCoding = {
  115200,                       /* baud rate */
  0x00,                         /* stop bits-1 */
  0x00,                         /* parity - none */
  0x08                          /* nb. of bits 8 */
};
/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
//uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
//uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
static usbrx_buf_t rxbuf = {0};
static uint8_t txbuf[TX_BUF_SIZE];
#ifdef ELM327
#define CDC_COMMAND_MTU ELM327_LINE_MTU
#else
#define CDC_COMMAND_MTU SLCAN_MTU
#endif
static uint8_t command_str[CDC_COMMAND_MTU];
static uint8_t command_str_index = 0;

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, txbuf, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, rxbuf.buf[rxbuf.head]);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:
        LineCoding.bitrate = (uint32_t) (pbuf[0] | (pbuf[1] << 8) |
                                         (pbuf[2] << 16) | (pbuf[3] << 24));
        LineCoding.format = pbuf[4];
        LineCoding.paritytype = pbuf[5];
        LineCoding.datatype = pbuf[6];
    break;

    case CDC_GET_LINE_CODING:
        pbuf[0] = (uint8_t) (LineCoding.bitrate);
        pbuf[1] = (uint8_t) (LineCoding.bitrate >> 8);
        pbuf[2] = (uint8_t) (LineCoding.bitrate >> 16);
        pbuf[3] = (uint8_t) (LineCoding.bitrate >> 24);
        pbuf[4] = LineCoding.format;
        pbuf[5] = LineCoding.paritytype;
        pbuf[6] = LineCoding.datatype;
    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  // Check for overflow!
  // If when we increment the head we're going to hit the tail
  // (if we're filling the last spot in the queue)
  // FIXME: Use a "full" variable instead of wasting one
  // spot in the cirbuf as we are doing now
  if( ((rxbuf.head + 1) % NUM_RX_BUFS) == rxbuf.tail)
  {
    error_assert(ERR_FULLBUF_USBRX);

    // Listen again on the same buffer. Old data will be overwritten.
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, rxbuf.buf[rxbuf.head]);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return HAL_ERROR;
  }
  else
  {
    // Save off length
    rxbuf.msglen[rxbuf.head] = *Len;
    rxbuf.head = (rxbuf.head + 1) % NUM_RX_BUFS;

    // Start listening on next buffer. Previous buffer will be processed in main loop.
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, rxbuf.buf[rxbuf.head]);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return (USBD_OK);
  }
}


/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
    // Attempt to transmit on USB, wait until not busy
    // Future: implement TX buffering
    uint32_t start_wait = HAL_GetTick();
    while( ((USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData)->TxState)
    {
      // If no TX within timeout, abort.
      if(HAL_GetTick() - start_wait >= 10)
      {
          error_assert(ERR_USBTX_BUSY);
          return USBD_BUSY;
      }
    }

    // Ensure message will fit in buffer
    if(Len > TX_BUF_SIZE)
    {
        return 0;
    }

    // Copy data into buffer
    for (uint32_t i=0; i < Len; i++)
    {
        txbuf[i] = Buf[i];
    }

    // Set transmit buffer and start TX
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, txbuf, Len);
    return USBD_CDC_TransmitPacket(&hUsbDeviceFS);
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
void system_irq_disable(void)
{
    __disable_irq();
    __DSB();
    __ISB();
}


// Enable all interrupts
void system_irq_enable(void)
{
    __enable_irq();
}


uint8_t cdc_process(void)
{
    uint8_t processed = 0;
    system_irq_disable();
    if(rxbuf.tail != rxbuf.head)
    {
        //  Process one whole buffer
        for (uint32_t i = 0; i < rxbuf.msglen[rxbuf.tail]; i++)
        {
            if (rxbuf.buf[rxbuf.tail][i] == '\r')
            {
#ifdef ELM327
                elm327_command(command_str, command_str_index);
#else
                int8_t result = slcan_parse_str(command_str, command_str_index);
                UNUSED(result);
#endif

                // Success
                //if(result == 0)
                //    CDC_Transmit_FS("\n", 1);
                // Failure
                //else
                //    CDC_Transmit_FS("\a", 1);

                command_str_index = 0;
                processed = 1;
            }
            else
            {
                // Check for overflow of buffer
                if(command_str_index >= CDC_COMMAND_MTU)
                {
                    // TODO: Return here and discard this CDC buffer?
                    command_str_index = 0;
                }

                command_str[command_str_index++] = rxbuf.buf[rxbuf.tail][i];
            }
        }

        // Move on to next buffer
        rxbuf.tail = (rxbuf.tail + 1) % NUM_RX_BUFS;
    }
    system_irq_enable();
    return processed;
}


#ifdef DEBUG_MODE
uint8_t print_to_usb(char* message)
{
    uint16_t msg_len = strlen(message);
    return CDC_Transmit_FS((uint8_t*)message, msg_len < SLCAN_MTU ? msg_len : SLCAN_MTU);
}

uint8_t printf_to_usb(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int written = vsnprintf_((char*)command_str, CDC_COMMAND_MTU, format, args);
    va_end(args);
    return CDC_Transmit_FS(command_str, written < CDC_COMMAND_MTU ? (uint8_t) written : CDC_COMMAND_MTU);
  }
#endif
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
