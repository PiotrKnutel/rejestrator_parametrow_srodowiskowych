/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
extern RTC_DateTypeDef DateComapare;
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
  HAL_PWR_EnableBkUpAccess();
  uint8_t dateFromBackup[4];
  uint16_t bufBKP_DR1, bufBKP_DR2;
  bufBKP_DR1 = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
  bufBKP_DR2 = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
  dateFromBackup[0] = bufBKP_DR2 >> 8;	//rok
  dateFromBackup[1] = bufBKP_DR2;		//mies.
  dateFromBackup[2] = bufBKP_DR1 >> 8;	//dzień
  dateFromBackup[3] = bufBKP_DR1;		//dz. tyg.
  setDate(dateFromBackup);

  return;
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 1;
  DateToUpdate.Year = 20;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    HAL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void setTime(uint8_t* time)
{
  RTC_TimeTypeDef Time = {0};

  Time.Hours = time[0];
  Time.Minutes = time[1];
  Time.Seconds = time[2];

  if (HAL_RTC_SetTime(&hrtc, &Time, RTC_FORMAT_BIN) != HAL_OK) {
    Error_Handler();
  }
}

void setDate(uint8_t* date)
{
  RTC_DateTypeDef NewDate = {0};

  NewDate.Year = date[0];
  NewDate.Month = date[1];
  NewDate.Date = date[2];
  NewDate.WeekDay = date[3];
  DateComapare = NewDate;
  if (HAL_RTC_SetDate(&hrtc, &NewDate, RTC_FORMAT_BIN) != HAL_OK) {
    Error_Handler();
  }

}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
