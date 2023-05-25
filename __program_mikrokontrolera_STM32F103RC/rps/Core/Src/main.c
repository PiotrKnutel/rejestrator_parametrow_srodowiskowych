/* USER CODE BEGIN Header */
/* Rejestrator parametrów środowiskowych; Piotr Knutel; v1.0; main.c 		  */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "rtc.h"
#include "sdio.h"
#include "spi.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "rps.h"
#include "sensor_bme280.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;
extern RTC_DateTypeDef DateComapare;
RTC_TimeTypeDef LastMeasTime;
uint8_t measurementInterval;
_Bool receivedDataFlag;
_Bool measurementAndSaveFlag;
_Bool  communicationModeFlag;
float t,p,rh;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void setAlarm(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void setAlarm()
{
	  HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
	  HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
	  uint8_t minutesAfterLastMeas = Time.Minutes % measurementInterval;
	  uint8_t minutesToNextMeas = measurementInterval - minutesAfterLastMeas;
	  RTC_AlarmTypeDef alarm = {0};
	  alarm.AlarmTime.Seconds = 0;
	  alarm.AlarmTime.Hours = Time.Hours;
	  alarm.AlarmTime.Minutes = Time.Minutes + minutesToNextMeas;
	  if (alarm.AlarmTime.Minutes > 59) {
		  ++(alarm.AlarmTime.Hours);
		  alarm.AlarmTime.Minutes -= 60;
	  }
	  if (alarm.AlarmTime.Hours > 23) {
		  alarm.AlarmTime.Hours -= 24;
	  }
	  HAL_RTC_SetAlarm(&hrtc, &alarm, RTC_FORMAT_BIN);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  receivedDataFlag = 0;
  measurementAndSaveFlag = 0;
  communicationModeFlag = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /*Reset of all peripherals, Initializes the Flash interface and the Systick.*/
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_USB_DEVICE_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  measurementInterval = readMeasurementIntervalFromBackupRegister();
  if(measurementInterval == 0) {												//jeśli rejestr jest wyzerowany, np. na skutek zaniku Vbat
	  measurementInterval = DEFAULT_MEASUREMENT_INTERVAL;
	  writeMeasurementIntervalToBackupRegister(measurementInterval);
  }

  if (__HAL_PWR_GET_FLAG(PWR_FLAG_WU) == 1) {
	  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

	  HAL_GPIO_WritePin(GPIOC, LED_R_Pin, LED_ON);
	  HAL_Delay(500);
	  HAL_GPIO_WritePin(GPIOC, LED_R_Pin, LED_OFF);

	  HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
	  HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);
	  writeDateToBackupRegister();

	  readLastMeasTimeFromBKUP(&LastMeasTime);
	  if ((Time.Minutes != LastMeasTime.Minutes)
			  || (Time.Hours != LastMeasTime.Hours)) {
		  BME280measurement(&t, &p, &rh);
		  saveDataToSDCard(t, p, rh);
		  writeLastMeasTimeToBKUP(&Time);
	  }
	  setAlarm();
	  HAL_PWR_EnterSTANDBYMode();
  }

  /* Czas na naciśniecie przycisku II, w celu wejścia w tryb komunikacji.
     Czas na ewentualne rozpoczęcie wgrywania nowego programu. */
  HAL_GPIO_WritePin(GPIOC, LED_R_Pin, LED_ON);	//
  HAL_Delay(2000);								//
  HAL_GPIO_WritePin(GPIOC, LED_R_Pin, LED_OFF);	//
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  do
  {
	  HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
	  HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);
	  if (Date.Date != DateComapare.Date) {
		writeDateToBackupRegister();
	    DateComapare = Date;
	  }
	  readLastMeasTimeFromBKUP(&LastMeasTime);
	  if (((Time.Minutes % measurementInterval) == 0) && ((Time.Minutes
			  != LastMeasTime.Minutes) || (Time.Hours != LastMeasTime.Hours))) {
			  	  measurementAndSaveFlag = 1;
	  }
	  if (measurementAndSaveFlag) {
		 BME280measurement(&t, &p, &rh);
		 saveDataToSDCard(t, p, rh);
		 writeLastMeasTimeToBKUP(&Time);
		 measurementAndSaveFlag = 0;
	  }
	  if(communicationModeFlag && receivedDataFlag) {
	  	tasksWithReceivedDataFromPC();
	  	receivedDataFlag = 0;
	  }

  } while (measurementAndSaveFlag || communicationModeFlag);

  setAlarm();
  HAL_PWR_EnterSTANDBYMode();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE
		  |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USB;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t button)
{
	if (button == BUTTON_A_Pin) {
		measurementAndSaveFlag = 1;
	}
	if (button == BUTTON_B_Pin) {
		if(!communicationModeFlag) {
			HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, LED_ON);
			communicationModeFlag = 1;
		} else {
			HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, LED_OFF);
			communicationModeFlag = 0;
		}
	}
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
