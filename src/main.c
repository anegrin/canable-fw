#include "main.h"
#include "dashboard.h"
#include "can.h"
#include "uart.h"
#include "led.h"
#include "model.h"
#ifdef SLCAN
#include "slcan.h"
#endif
#ifdef ELM327
#include "elm327.h"
#endif
#include "processing.h"
#include "usb_device.h"
#ifdef XCAN
#include "usbd_storage_if.h"
#else
#include "usbd_cdc_if.h"
#endif
#include "storage.h"

DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_memtomem_dma1_channel1;

void state_init(GlobalState *state
#ifdef C1CAN
    , Settings *settings
#endif
);
void SystemClock_Config(void);
void GPIO_Init(void);
void GPIO_DeInit(void);
static void MX_DMA_Init(void);
#ifdef XCAN
static void MX_DMA_DeInit(void);
#endif

#ifdef XCAN
static const uint32_t goToBedDelay = 5000;
static const uint32_t blinkInterval = goToBedDelay / 4;
static uint32_t nextBlinkBeforeSleeping = 0;
#endif

static GlobalState state = {};
#ifdef C1CAN
static Settings settings = {};
#endif

int main(void)
{
    HAL_Init();
    SystemClock_Config();
#ifdef C2CAN
    __disable_irq();
    HAL_SuspendTick();
    HAL_PWR_EnterSTANDBYMode();
#endif
    GPIO_Init();
    MX_DMA_Init();
    uart_init();

    storage_init();
#ifdef C1CAN
    load_settings(&settings);
#endif

    MX_USB_DEVICE_Init();

#ifdef XCAN
    can_set_bitrate(CAN_BITRATE);
    can_enable();
#endif

#ifdef DEBUG_MODE
    leds_blink(5, 50);
#endif
#ifdef SLCAN
    leds_blink(4, 100);
#endif
#ifdef ELM327
    elm327_init();
    leds_blink(4, 100);
#endif
#ifdef C1CAN
    leds_blink(3, 250);
#endif
#ifdef BHCAN
    leds_blink(2, 500);
#endif

    state_init(&state
#ifdef C1CAN
        , &settings
#endif
);

    while (1)
    {
        state.board.now = HAL_GetTick();
#ifdef XCAN
        if (!STORAGE_Accessed_FS() || USB_MASS_STORAGE_CAN_SLEEP)
        {
#endif
#ifdef C1CAN
            if (state.board.goingToBedAt == 0 && state.car.canIsOnAt + STANDBY_DELAY_MS < state.board.now)
            {
                HAL_GPIO_WritePin(CAN_S_PIN_PORT, CAN_S_PIN, GPIO_PIN_SET);//can silent pin
                state.board.goingToBedAt = state.board.now + goToBedDelay + blinkInterval;
                nextBlinkBeforeSleeping = state.board.now + blinkInterval;
                send_state(&state);
            }

            if (!state.board.sleeping && state.board.goingToBedAt != 0 && state.board.goingToBedAt < state.board.now)
            {
                state.board.sleeping = true;
                // do not de init USB or it will mess up with interrupts
                can_readonly();
                uart_deinit();
                MX_DMA_DeInit();
                GPIO_DeInit();
                HAL_SuspendTick();
                HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
                break;
            }

#endif
#ifdef BHCAN
            if (state.board.goingToBedAt == 1 /*|| (state.board.goingToBedAt == 0 && (state.car.canIsOnAt + STANDBY_DELAY_MS + goToBedDelay) < state.board.now)*/)
            {
                //state.car.canIsOnAt = 0;
                HAL_GPIO_WritePin(CAN_S_PIN_PORT, CAN_S_PIN, GPIO_PIN_SET);//can silent pin
                state.board.goingToBedAt = state.board.now + goToBedDelay - blinkInterval;
                nextBlinkBeforeSleeping = state.board.now + blinkInterval;
            }

            if (!state.board.sleeping && state.board.goingToBedAt > 1 && state.board.goingToBedAt < state.board.now)
            {
                state.board.sleeping = true;
                // do not de init USB or it will mess it up with consumption
                can_disable();
                uart_deinit();
                MX_DMA_DeInit();
                GPIO_DeInit();
                HAL_DeInit();
                __disable_irq();
                HAL_SuspendTick();
                HAL_PWR_EnterSTANDBYMode();
                break;
            }
#endif
#ifdef XCAN
            if (!state.board.sleeping && nextBlinkBeforeSleeping != 0 && nextBlinkBeforeSleeping < state.board.now)
            {
                led_tx_on();
                led_rx_on();
                nextBlinkBeforeSleeping = state.board.now + blinkInterval;
            }
        }
#endif

#if defined(SLCAN) || defined(ELM327)
        cdc_process();
#endif
#ifdef ELM327
        elm327_process();
#endif
        led_process();
        can_process();

#ifdef C1CAN
        state_process(&state, &settings);
#endif
#ifdef BHCAN
        state_process(&state);
#endif
        uart_process(&state);
    }
}

CAN_RxHeaderTypeDef rx_msg_header; // msg header
uint8_t rx_msg_data[8] = {
    0,
}; // msg data
#ifdef SLCAN
uint8_t msg_buf[SLCAN_MTU];
#endif
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
#ifdef C1CAN
    if (state.board.sleeping)
    {
        HAL_NVIC_SystemReset();
    }
#endif

    if (can_rx(&rx_msg_header, rx_msg_data) == HAL_OK)
    {
#ifdef SLCAN
        uint16_t msg_len = slcan_parse_frame((uint8_t *)&msg_buf, &rx_msg_header, rx_msg_data);
        if (msg_len)
        {
            CDC_Transmit_FS(msg_buf, msg_len);
        }
#endif
#ifdef ELM327
        elm327_on_can_frame(&rx_msg_header, rx_msg_data);
#endif
#ifdef XCAN
        if (rx_msg_header.RTR == CAN_RTR_DATA)
        {
            state.car.canIsOnAt = state.board.now;
            switch (rx_msg_header.IDE)
            {
            case CAN_ID_STD:
                handle_standard_frame(&state
                    #ifdef C1CAN
                    , &settings
                    #endif
                    , rx_msg_header, rx_msg_data);
                break;
            case CAN_ID_EXT:
                handle_extended_frame(&state
                    #ifdef C1CAN
                    , &settings
                    #endif
                    , rx_msg_header, rx_msg_data);
                break;
            default:
            }
        }
#endif
    }
}

void state_init(GlobalState *state
#ifdef C1CAN
    , Settings *settings
#endif
)
{
#ifdef C1CAN
    state->board.collectingMultiframeResponse = -1;
    state->board.dashboardExternallyUpdatedAt = 0;
    state->board.snsRequestOffAt = 0;
    state->board.dashboardState.carouselShowNextItemAt = settings->bootCarouselEnabled ? HAL_GetTick() + settings->bootCarouselDelay : 0;
#endif
    state->board.dpfRegenSoundNotificationRequestAt = 0;
    state->board.dashboardState.itemsCount = DASHBOARD_ITEMS_COUNT;
    state->board.dashboardState.currentItemIndex = 0;
    state->board.dashboardState.values[0] = -1.0f;
    state->board.dashboardState.values[1] = -1.0f;
    state->board.goingToBedAt = 0;
    state->board.sleeping = false;

    state->car.dpf.regenerating = false;
    state->car.dpf.regenMode = 0;
    state->car.canIsOnAt = 0;
#ifdef C1CAN
    state->car.battery.chargePercent = 0;
    state->car.battery.current = 0.0f;
    state->car.ccActive = false;
    state->car.gear = '-';
    state->car.oil.pressure = 0.0f;
    state->car.oil.temperature = 0;
    state->car.engineIsOnAt = 0;
    state->car.rpm = 0;
    state->car.sns.active = true;
    state->car.sns.snsOffAt = 0;
    state->car.torque = 0;
#endif
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

/** Initializes the RCC Oscillators according to the specified parameters
 * in the RCC_OscInitTypeDef structure.
 */
#ifdef ENABLE_EXTERNAL_OSCILLATOR
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
    RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
#else
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
#endif
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
#ifdef ENABLE_EXTERNAL_OSCILLATOR
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
#else
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
#endif
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB | RCC_PERIPHCLK_USART2;
#ifdef ENABLE_EXTERNAL_OSCILLATOR
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
#else
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
#endif

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

void GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(CAN_S_PIN_PORT, CAN_S_PIN, GPIO_PIN_RESET);
#ifdef C1CAN
    HAL_GPIO_WritePin(GPIOA, RESET_CMD_PIN, GPIO_PIN_RESET);
#endif
    HAL_GPIO_WritePin(GPIOA, LED_RX_Pin | LED_TX_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : PC13 */
    GPIO_InitStruct.Pin = CAN_S_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CAN_S_PIN_PORT, &GPIO_InitStruct);

#ifdef C1CAN
    /*Configure GPIO pin : PA13 */
    GPIO_InitStruct.Pin = RESET_CMD_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif

    /*Configure GPIO pins : PA0 PA1 */
    GPIO_InitStruct.Pin = LED_RX_Pin | LED_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* reset pins */
#ifdef C1CAN
    HAL_GPIO_WritePin(GPIOA, RESET_CMD_PIN, GPIO_PIN_RESET); // gnd
    HAL_Delay(250);
    HAL_GPIO_WritePin(GPIOA, RESET_CMD_PIN, GPIO_PIN_SET); // float
#endif
}

void GPIO_DeInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    //GPIO_PIN_11 -> USB N, GPIO_PIN_12 -> USB P
    GPIO_InitStruct.Pin = LED_RX_Pin | LED_TX_Pin | UART_PIN | GPIO_PIN_11 | GPIO_PIN_12;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

#ifdef C1CAN
    //GPIO_PIN_2 -> boot, GPIO_PIN_5, GPIO_PIN_6 -> resistor/3.3v
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6;
#endif
#ifdef BHCAN
    GPIO_InitStruct.Pin = GPIO_PIN_All;
#endif
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_All;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

/* Disable GPIOs clock */
#ifdef BHCAN
    __HAL_RCC_GPIOB_CLK_DISABLE();
#endif
    __HAL_RCC_GPIOC_CLK_DISABLE();
    __HAL_RCC_GPIOF_CLK_DISABLE();
}

/**
 * Enable DMA controller clock
 * Configure DMA for memory to memory transfers
 *   hdma_memtomem_dma1_channel1
 */
static void MX_DMA_Init(void)
{

    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Channel4_5_6_7_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);
}

#ifdef XCAN
static void MX_DMA_DeInit(void)
{

    /* DMA interrupt deinit */
    HAL_NVIC_DisableIRQ(DMA1_Channel4_5_6_7_IRQn);
    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_DISABLE();
}
#endif

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    leds_on_error();
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}
