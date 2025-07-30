/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define DEFAULT_BLINK_TIME    500
#define OP_BLINK_TIME         50

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define ITM_CH_1       		0
#define ITM_CH_2			    1

#define LOG_ITM_1(fmt, ...)  itm_printf(ITM_CH_1, fmt, ##__VA_ARGS__)
#define LOG_ITM_2(fmt, ...)  itm_printf(ITM_CH_2, fmt, ##__VA_ARGS__)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern void itm_printf(uint32_t port, const char* format, ...);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPEEDOMETER_SIG_Pin GPIO_PIN_0
#define SPEEDOMETER_SIG_GPIO_Port GPIOA
#define TEST_LED_Pin GPIO_PIN_0
#define TEST_LED_GPIO_Port GPIOB
#define OIL_PRESURE_SIG_Pin GPIO_PIN_7
#define OIL_PRESURE_SIG_GPIO_Port GPIOE
#define HIGH_BEAM_SIG_Pin GPIO_PIN_8
#define HIGH_BEAM_SIG_GPIO_Port GPIOE
#define NEUTRAL_SIG_Pin GPIO_PIN_9
#define NEUTRAL_SIG_GPIO_Port GPIOE
#define OIL_TEMP_SIG_Pin GPIO_PIN_10
#define OIL_TEMP_SIG_GPIO_Port GPIOE
#define ENGINE_TEMP_SIG_Pin GPIO_PIN_11
#define ENGINE_TEMP_SIG_GPIO_Port GPIOE
#define RIGHT_TURN_SIG_Pin GPIO_PIN_12
#define RIGHT_TURN_SIG_GPIO_Port GPIOE
#define LEFT_TURN_SIG_Pin GPIO_PIN_13
#define LEFT_TURN_SIG_GPIO_Port GPIOE
#define ECU_CAN_RX_Pin GPIO_PIN_12
#define ECU_CAN_RX_GPIO_Port GPIOB
#define ECU_CAN_TX_Pin GPIO_PIN_13
#define ECU_CAN_TX_GPIO_Port GPIOB
#define GEAR_4_SIG_Pin GPIO_PIN_8
#define GEAR_4_SIG_GPIO_Port GPIOD
#define GEAR_5_SIG_Pin GPIO_PIN_9
#define GEAR_5_SIG_GPIO_Port GPIOD
#define TACHOMETER_SIG_Pin GPIO_PIN_8
#define TACHOMETER_SIG_GPIO_Port GPIOA
#define ADC_RDY_Pin GPIO_PIN_12
#define ADC_RDY_GPIO_Port GPIOC
#define ADC_RDY_EXTI_IRQn EXTI15_10_IRQn
#define DISPLAY_CAN_RX_Pin GPIO_PIN_0
#define DISPLAY_CAN_RX_GPIO_Port GPIOD
#define DISPLAY_CAN_TX_Pin GPIO_PIN_1
#define DISPLAY_CAN_TX_GPIO_Port GPIOD
#define DISPLAY_CAN_STB_Pin GPIO_PIN_2
#define DISPLAY_CAN_STB_GPIO_Port GPIOD
#define DISPLAY_CAN_SHDN_Pin GPIO_PIN_3
#define DISPLAY_CAN_SHDN_GPIO_Port GPIOD
#define GEAR_1_SIG_Pin GPIO_PIN_5
#define GEAR_1_SIG_GPIO_Port GPIOD
#define GEAR_2_SIG_Pin GPIO_PIN_6
#define GEAR_2_SIG_GPIO_Port GPIOD
#define GEAR_3_SIG_Pin GPIO_PIN_7
#define GEAR_3_SIG_GPIO_Port GPIOD
#define DEBUG_TX_Pin GPIO_PIN_6
#define DEBUG_TX_GPIO_Port GPIOB
#define DEBUG_RX_Pin GPIO_PIN_7
#define DEBUG_RX_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define __ITM_PORT_MASK_1(p)           (1U << (p))
#define __ITM_PORT_MASK_2(p, ...)      ((1U << (p)) | __ITM_PORT_MASK_1(__VA_ARGS__))
#define __ITM_PORT_MASK_3(p, ...)      ((1U << (p)) | __ITM_PORT_MASK_2(__VA_ARGS__))
#define __ITM_PORT_MASK_4(p, ...)      ((1U << (p)) | __ITM_PORT_MASK_3(__VA_ARGS__))
#define __ITM_PORT_MASK_5(p, ...)      ((1U << (p)) | __ITM_PORT_MASK_4(__VA_ARGS__))
#define __ITM_PORT_MASK_6(p, ...)      ((1U << (p)) | __ITM_PORT_MASK_5(__VA_ARGS__))
#define __ITM_PORT_MASK_7(p, ...)      ((1U << (p)) | __ITM_PORT_MASK_6(__VA_ARGS__))
#define __ITM_PORT_MASK_8(p, ...)      ((1U << (p)) | __ITM_PORT_MASK_7(__VA_ARGS__))

#define GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,NAME,...) NAME
#define __ITM_PORT_MASK(...) GET_MACRO(__VA_ARGS__, \
      __ITM_PORT_MASK_8, __ITM_PORT_MASK_7, __ITM_PORT_MASK_6, __ITM_PORT_MASK_5, \
      __ITM_PORT_MASK_4, __ITM_PORT_MASK_3, __ITM_PORT_MASK_2, __ITM_PORT_MASK_1)(__VA_ARGS__)


#define INIT_ITM_DEBUG(...)                                       \
  do {                                                            \
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;              \
    ITM->LAR = 0xC5ACCE55;                                       \
    ITM->TCR = ITM_TCR_ITMENA_Msk |                              \
               ITM_TCR_SYNCENA_Msk |                             \
               ITM_TCR_TSENA_Msk   |                             \
               (1 << ITM_TCR_TraceBusID_Pos);                    \
    ITM->TER = __ITM_PORT_MASK(__VA_ARGS__);                     \
  } while (0)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
