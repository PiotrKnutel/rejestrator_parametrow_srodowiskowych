/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
#define LED_ON							RESET
#define LED_OFF 						SET
#define DEFAULT_MEASUREMENT_INTERVAL	10;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_G_Pin GPIO_PIN_1
#define LED_G_GPIO_Port GPIOC
#define LED_Y_Pin GPIO_PIN_2
#define LED_Y_GPIO_Port GPIOC
#define LED_R_Pin GPIO_PIN_3
#define LED_R_GPIO_Port GPIOC
#define BME280_CS_Pin GPIO_PIN_0
#define BME280_CS_GPIO_Port GPIOB
#define SD_CD_Pin GPIO_PIN_5
#define SD_CD_GPIO_Port GPIOB
#define BUTTON_A_Pin GPIO_PIN_6
#define BUTTON_A_GPIO_Port GPIOB
#define BUTTON_A_EXTI_IRQn EXTI9_5_IRQn
#define BUTTON_B_Pin GPIO_PIN_7
#define BUTTON_B_GPIO_Port GPIOB
#define BUTTON_B_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
