/* Rejestrator parametrów środowiskowych; Piotr Knutel; v1.0; sensor_bme280.c */

#include "sensor_bme280.h"

extern float lastTemperature, lastPressure, lastHumidity;
extern uint8_t lastMeasurementInfoString[300];

uint8_t BME280id;
uint8_t BME280config;
uint8_t status;
uint8_t conf1, meas1, ctrl_hum1;
int32_t tFine;
unsigned short digT1, digP1;
short digT2, digT3, digP2, digP3, digP4, digP5, digP6, digP7, digP8, digP9,
	  digH2, digH4, digH5;
unsigned char digH1, digH3;
char digH6;
uint8_t dig1[26];
uint8_t dig2[7];

void clearTable(uint8_t *tab)
{
	for (int i = 0; i < sizeof(tab); ++i) {
		tab[i] = 0;
	}
}

void BME280Read(uint8_t address, uint8_t numberOfBytes, uint8_t* answer)
{
	uint8_t *tx, *rx;
	tx = malloc((numberOfBytes + 1) * sizeof *tx);
	rx = malloc((numberOfBytes + 1) * sizeof *rx);
	tx[0] = address | (1<<7); //dla odczytu pamięci, 7 bit adresu jest zawsze 1
	clearTable(answer);
	HAL_GPIO_WritePin(BME280_CS_GPIO_Port, BME280_CS_Pin, RESET);
	HAL_SPI_TransmitReceive(SPI_BME280, tx, rx, numberOfBytes + 1, 5);
	HAL_GPIO_WritePin(BME280_CS_GPIO_Port, BME280_CS_Pin, SET);
	for (int i = 0; i < numberOfBytes; ++i) {
		answer[i] = rx[i + 1];
	}
	free(rx);
	free(tx);
}

uint8_t BME280Write(uint8_t address, uint8_t data)
{
	uint8_t tx[2];
	uint8_t verify = 0;
	uint8_t oldData = 0;
	uint8_t mask = 0;
	if ((address != BME280_ADDRESS_CONFIG)
			&& (address != BME280_ADDRESS_CTRL_MEAS)
			&& (address != BME280_ADDRESS_CTRL_HUM)
			&& (address != BME280_ADDRESS_RESET))
		return STATUS_WRONG_ARGUMENT;
	switch (address) {
		case BME280_ADDRESS_CONFIG:
			mask = 0b00000010;
			break;
		case BME280_ADDRESS_CTRL_HUM:
			mask = 0b11111000;
			break;
	}
	BME280Read(address, 1, &oldData);
	data &= ~mask;																//wyzeruje bity argumentu data, które nie powinno się zmieniać (dokumentacja BME280 str. 27)
	data |= (oldData & mask);													//wyzeruje stare bity, które można zmieniać i w ich miejsce wstawi nowe
	tx[0] = address & 0b01111111;												//dla zapisu do pamięci 7 bit adresu jest zawsze 0
	tx[1] = data;
	HAL_GPIO_WritePin(BME280_CS_GPIO_Port, BME280_CS_Pin, RESET);
 	HAL_SPI_Transmit(SPI_BME280, tx, 2, 5);
	HAL_GPIO_WritePin(BME280_CS_GPIO_Port, BME280_CS_Pin, SET);
	if (address == BME280_ADDRESS_RESET) {
		return STATUS_OK;
	}
 	BME280Read(address, 1, &verify);
 	if (verify == data)
 		return STATUS_OK;
 	else
 		return STATUS_ERROR;
}
int32_t BME280compensateT(int32_t adcT)
{  //dokumentacja BME280 str. 25)
	int32_t var1, var2, T;
	var1 = ((((adcT >> 3) - ((int32_t)digT1 << 1))) * ((int32_t)digT2)) >> 11;
	var2 = (((((adcT >> 4) - ((int32_t)digT1)) * ((adcT >> 4)
			- ((int32_t)digT1))) >> 12) * ((int32_t)digT3)) >> 14;
	tFine = var1 + var2;
	T = (tFine * 5 + 128) >> 8;
	return T;
}

uint32_t BME280compensateP(int32_t adcP)
{
	int64_t var1, var2, p;
	var1 = ((int64_t)tFine) - 128000;
	var2 = var1 * var1 * (int64_t)digP6;
	var2 = var2 + ((var1 * (int64_t)digP5) << 17);
	var2 = var2 +(((int64_t)digP4) << 35);
	var1 = ((var1*var1* (int64_t)digP3) >> 8) + ((var1 * (int64_t)digP2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)digP1) >> 33;
	if (var1 == 0) {
		return 0;
	}
	p = 1048576 - adcP;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)digP9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)digP8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)digP7) << 4);
	return (uint32_t)p;
}

uint32_t BME280compensateH(int32_t adcH)
{
	int32_t v;
	v = (tFine - ((int32_t)76800));
	v = (((((adcH <<14) - (((int32_t)digH4) << 20) - (((int32_t)digH5) * v))
			+ ((int32_t)16384)) >> 15) * (((((((v * ((int32_t)digH6)) >> 10)
					* (((v*((int32_t)digH3)) >> 11) + ((int32_t)32768))) >> 10)
					+ ((int32_t)2097152)) * ((int32_t)digH2) + 8192) >> 14));
	v = (v - (((((v >> 15) * (v >> 15)) >> 7) * ((int32_t)digH1)) >> 4));
	v = (v < 0 ? 0 : v);
	v = (v > 419430400 ? 419430400 : v);
	return (uint32_t)(v >> 12);
}

void BME280measurement(float* outTemperature, float* outPressure,
		float* outHumidity)
{

	/* Obsługa BME280 */
	  uint8_t temp[3] = {0, 0, 0};
	  uint8_t press[3] = {0, 0, 0};
	  uint8_t hum[2] = {0, 0};
	  uint8_t allADC[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	  uint8_t t_sb, filter, spi3w_en, config, osrs_p, osrs_t, mode, ctrl_meas,
	  	  	  ctrl_hum;

	  t_sb = BME280_T_SB_20MS;
	  filter = BME280_FILTER_OFF;
	  spi3w_en = BME280_SPI3W_EN_OFF;
	  config = 0 | (t_sb << 5) | (filter << 2) | spi3w_en;
	  osrs_t = BME280_OSRS_X1;
	  osrs_p = BME280_OSRS_X1;
	  mode = BME280_MODE_FORCED;
	  ctrl_meas = (osrs_t << 5) | (osrs_p << 2) | mode;
	  ctrl_hum = BME280_OSRS_X1;

	  BME280Write(BME280_ADDRESS_RESET, BME280_RESET);
	  HAL_Delay(10);
	  BME280Read(BME280_ADDRESS_ID, 1, &BME280id);
	  status = BME280Write(BME280_ADDRESS_CONFIG, config);
	  status = BME280Write(BME280_ADDRESS_CTRL_MEAS, ctrl_meas);
	  status = BME280Write(BME280_ADDRESS_CTRL_HUM, ctrl_hum);
	  status = BME280Write(BME280_ADDRESS_CTRL_MEAS, ctrl_meas);

	  HAL_Delay(12);
	  BME280Read(BME280_ADDRESS_CONFIG, 1, &conf1);
	  BME280Read(BME280_ADDRESS_CTRL_MEAS, 1, &meas1);
	  BME280Read(BME280_ADDRESS_CTRL_HUM, 1, &ctrl_hum1);
	  BME280Read(BME280_ADDRESS_FIRST_BYTE_OF_ADC_VALUE, 8, allADC);
	  BME280Read(0x88, 26, dig1);
	  BME280Read(0xE1, 7, dig2);
	  digT1 = (unsigned short)((dig1[1] << 8) | dig1[0]);
	  digT2 = (short)((dig1[3] << 8) | dig1[2]);
	  digT3 = (short)((dig1[5] << 8) | dig1[4]);
	  digP1 = (unsigned short)((dig1[7] << 8) | dig1[6]);
	  digP2 = (short)((dig1[9] << 8) | dig1[8]);
	  digP3 = (short)((dig1[11] << 8) | dig1[10]);
	  digP4 = (short)((dig1[13] << 8) | dig1[12]);
	  digP5 = (short)((dig1[15] << 8) | dig1[14]);
	  digP6 = (short)((dig1[17] << 8) | dig1[16]);
	  digP7 = (short)((dig1[19] << 8) | dig1[18]);
	  digP8 = (short)((dig1[21] << 8) | dig1[20]);
	  digP2 = (short)((dig1[23] << 8) | dig1[22]);
	  digH1 = (unsigned char)dig1[25];
	  digH2 = (short)((dig2[1] << 8) | dig2[0]);
	  digH3 = (unsigned char)dig2[2];
	  digH4 = (short)((dig2[3] << 4) | (dig2[4] & 0x0F));
	  digH5 = (short)(((dig2[4] & 0xF0) >> 4) | (dig2[5] << 4));
	  digH6 = (char)dig2[6];
	  press[0] = allADC[0];
	  press[1] = allADC[1];
	  press[2] = allADC[2];
	  temp[0] = allADC[3];
	  temp[1] = allADC[4];
	  temp[2] = allADC[5];
	  hum[0] = allADC[6];
	  hum[1] = allADC[7];
	  int32_t t, p, h;
	  t = p = h = 0;
	  t = (int32_t)((temp[0] << 12) | (temp[1] << 4) | (temp[2] >> 4));
	  p = (int32_t)((press[0] << 12) | (press[1] << 4) | ((press)[2] >> 4));
	  h = (int32_t)((hum[0] << 8) | hum[1]);
	  int ti = BME280compensateT(t);
	  uint32_t pi = BME280compensateP(p);
	  uint32_t hi = BME280compensateH(h);

	  float Temperature = (float)ti / 100;
	  *outTemperature = Temperature;
	  float Pressure = (float)pi / 25600;
	  *outPressure = Pressure;
	  float Humidity = (float)hi / 1024;
	  *outHumidity = Humidity;
	  /*przekazanie wyników pomiaru do zmiannych globalnych z rps.c,
	    potrzebnych do ewentualnego przekazania do PC po komendzie "gfi"("I") */
	  lastTemperature = Temperature;
	  lastPressure = Pressure;
	  lastHumidity = Humidity;
}
