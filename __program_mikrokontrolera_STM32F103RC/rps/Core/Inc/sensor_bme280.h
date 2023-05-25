/* Rejestrator parametrów środowiskowych; Piotr Knutel; v1.0; sensor_bme280.h */


#ifndef INC_SENSOR_BME280_H_
#define INC_SENSOR_BME280_H_

#include "main.h"
#include "spi.h"
#include "rps.h"
#include <string.h>
#include <stdio.h>

#define SPI_BME280					&hspi1
#define STATUS_OK					0
#define STATUS_ERROR				1
#define STATUS_WRONG_ARGUMENT		2
#define BME280_ADDRESS_ID			0xD0
#define BME280_ADDRESS_CTRL_HUM		0xF2
#define BME280_ADDRESS_CTRL_MEAS	0xF4
#define BME280_ADDRESS_CONFIG		0xF5
#define BME280_ADDRESS_FIRST_BYTE_OF_ADC_VALUE	0xF7
#define BME280_ADDRESS_PRESS_MSB	0xF7
#define BME280_ADDRESS_TEMP_MSB		0xFA
#define BME280_ADDRESS_HUM_MSB		0xFD
#define BME280_ADDRESS_RESET		0xE0
#define BME280_RESET				0xB6
#define BME280_T_SB_20MS			0b00000111
#define BME280_FILTER_OFF			0
#define BME280_FILTER_2				0b00000001
#define BME280_FILTER_4				0b00000010
#define BME280_FILTER_8				0b00000011
#define BME280_FILTER_16			0b00000100
#define BME280_SPI3W_EN_OFF			0
#define BME280_SPI3W_EN_ON			1
#define BME280_OSRS_SKIPPED			0
#define BME280_OSRS_X1				1
#define BME280_OSRS_X2				2
#define BME280_OSRS_X4				3
#define BME280_OSRS_X8				4
#define BME280_OSRS_X16				5
#define BME280_MODE_FORCED			1
#define BME280_MODE_SLEEP			0


void BME280measurement(float*, float*, float*);



#endif /* INC_SENSOR_BME280_H_ */
