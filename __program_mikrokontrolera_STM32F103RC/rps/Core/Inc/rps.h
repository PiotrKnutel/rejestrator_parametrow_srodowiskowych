/* Rejestrator parametrów środowiskowych; Piotr Knutel; v1.0; rps.h 		  */


#ifndef INC_RPS_H_
#define INC_RPS_H_

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "main.h"

#include <string.h>
#include <stdio.h>
//#include <stdlib.h>

#include "fatfs.h"
#include "rtc.h"
#include "sdio.h"
#include "usb_device.h"
#include "gpio.h"
#include "usbd_cdc_if.h"

void allLEDsOff(void);
void testLED(void);
void writeDateToBackupRegister(void);
void tasksWithReceivedDataFromPC(void);
//void sendMessageToSerialPortByUSB(char* message);
void saveDataToSDCard(float, float, float);

uint8_t readMeasurementIntervalFromBackupRegister(void);
void writeMeasurementIntervalToBackupRegister(uint8_t);

#endif /* INC_RPS_H_ */
