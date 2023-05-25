/* Rejestrator parametrów środowiskowych; Piotr Knutel; v1.0; rps.c 		  */

#include "rps.h"

#define S_OK		0	//Status OK
#define G_ERR		1	//błąd ogólny

RTC_TimeTypeDef Time;
RTC_DateTypeDef Date;
RTC_DateTypeDef DateComapare;
extern uint8_t measurementInterval;

const TCHAR firstLineOfFile[120] = "data i czas,T [°C],p [hPa],RH [%],"
		"Rejestrator parametrów środowiskowych - Piotr Knutel,\0";
uint8_t ReceivedData[40]; // tablica przechowujaca odebrane dane
uint8_t time[3] = {0,0,0};
uint8_t date[4] = {0,0,0,0};
int cmp = 0;
const char strSetTime[8] = "settime ";
const char strSetDate[8] = "setdate ";
const char strWhatTime[8] = "whattime";
const char strGetFileInfo[3] = "gfi";
const char strSetInterval[8] = "setinte ";
const char strGetInterval[8] = "getinte ";
const char strSetDaysPerFile[8] = "setdpfs ";
const char strGetDaysPerFile[8] = "getdpfs ";

uint32_t fileSize = 0;
uint32_t pointer;
uint32_t eof;
float lastTemperature = 0;
float lastPressure = 0;
float lastHumidity = 0;
uint8_t lastMeasurementInfoString[300];
UINT nrWrittenBytes;
enum period {WEEK, ONE_DAY};
typedef int EPR_STAT;

void clearTableChar(char *tab)
{
	for (int i = 0; i < sizeof(tab); ++i) {
		tab[i] = 0;
	}
}

void writeDateToBackupRegister(void)
{
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, ((Date.Year << 8) | (Date.Month)));
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, ((Date.Date << 8) | (Date.WeekDay)));
}

uint8_t readMeasurementIntervalFromBackupRegister()
{
	return (uint8_t)HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
}

void writeMeasurementIntervalToBackupRegister(uint8_t interval)
{
	uint16_t bkp_dr3 = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
	bkp_dr3 &= 0xFF00;
	bkp_dr3 |= (uint16_t)interval;
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, bkp_dr3);
}

uint8_t readDaysPerFileSettingFromBKUP()
{
	return (uint8_t)(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3) >> 8);
	// 0 - tydzień, 1 - dzień
}

void writeDaysPerFileSettingToBackupRegister(uint8_t daysPerFile)
{
	uint16_t bkp_dr3 = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
	bkp_dr3 &= 0x00FF;
	bkp_dr3 |= ((uint16_t)daysPerFile << 8);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, bkp_dr3);
}

void writeLastMeasTimeToBKUP(RTC_TimeTypeDef *measTime)
{
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4,
			((measTime->Hours << 8) | (measTime->Minutes)));
}

void readLastMeasTimeFromBKUP(RTC_TimeTypeDef *measTime)
{
	uint16_t bkup_dr4 = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);
	measTime->Hours = (uint8_t)(bkup_dr4 >> 8);
	measTime->Minutes = (uint8_t)bkup_dr4;
}

// realizacja zadań po otrzymaniu danych z komputera, poprzez USB
void tasksWithReceivedDataFromPC()
{
	char Message[300];
	int MessageLen;

	// wykrywanie i obsługa poleceń
	// polecenie 1: odczyt daty i czasu "whattime"
	cmp = strncmp((char*)ReceivedData, strWhatTime, 8);
	if (cmp == 0)
	{
		MessageLen = sprintf(Message, "Data: %02d.%02d.20%02d Czas: %02d:%02d:"
				"%02d\n\r", Date.Date, Date.Month, Date.Year, Time.Hours,
				Time.Minutes, Time.Seconds);
		CDC_Transmit_FS((uint8_t*)Message, MessageLen);
	}

	// polecenie 2: ustawienie czasu "settime "
	cmp = strncmp((char*)ReceivedData, strSetTime, 8);
	if (cmp == 0) {
		for (int i = 0; i<3; ++i) {
			time[i] = ReceivedData[i+8];
		}
		setTime(time);

		//wysłanie do PC aktualnej daty i godziny
		HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);
		MessageLen = sprintf(Message, "Data: %02d.%02d.20%02d Czas: %02d:%02d:"
				"%02d\n\r", Date.Date, Date.Month, Date.Year, Time.Hours,
				Time.Minutes, Time.Seconds);
		CDC_Transmit_FS((uint8_t*)Message, MessageLen);
	}

	// polecenie 3: ustawienie daty "setdate "
	cmp = strncmp((char*)ReceivedData, strSetDate, 8);
	if (cmp == 0) {
		for (int i = 0; i<4; ++i) {
			date[i] = ReceivedData[i+8];
		}
		setDate(date);
		HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);
		writeDateToBackupRegister();

		//wysłanie do PC aktualnej daty i godziny
		HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN); 							//daty już nie trzeba pobierać
		MessageLen = sprintf(Message, "Data: %02d.%02d.20%02d Czas: %02d:%02d:"
				"%02d\n\r", Date.Date, Date.Month, Date.Year, Time.Hours,
				Time.Minutes, Time.Seconds);
		CDC_Transmit_FS((uint8_t*)Message, MessageLen);
	}

	// polecenie 4: odczyt informacji o pliku
	cmp = strncmp((char*)ReceivedData, strGetFileInfo, 3);
	if (cmp == 0) {
		RTC_TimeTypeDef lt;
		readLastMeasTimeFromBKUP(&lt);
		MessageLen = sprintf(Message, "Info: rozmiar=%ldB znak=%ld zapisano="
			  "%dB eof=%ld; Ostatni pomiar: %f st. C %f hPa %f%% RH o %02d:%02d"
			  "\n\r",fileSize, pointer, nrWrittenBytes, eof, lastTemperature,
			  lastPressure, lastHumidity, lt.Hours, lt.Minutes);
		CDC_Transmit_FS((uint8_t*)Message, MessageLen);
	}

	// polecenie 5: ustawienie odstępu pomiędzy pomiarami (interwału)
	cmp = strncmp((char*)ReceivedData, strSetInterval, 8);
	if (cmp == 0) {
		uint8_t i = ReceivedData[8];
		if ((i == 1 ) || (i == 5) || (i == 10) || (i == 15) || (i == 20) ||
				(i == 30) || (i == 60)) {
			writeMeasurementIntervalToBackupRegister(i);
			measurementInterval = readMeasurementIntervalFromBackupRegister();
			MessageLen = sprintf(Message, "Poprawna wartosc: %d. Pomiar co %d "
					"minut.\n\r", i, measurementInterval);
			CDC_Transmit_FS((uint8_t*)Message, MessageLen);
		} else {
			MessageLen = sprintf(Message, "Odebrano niepoprawna wartosc: %d! "
				"Pomiar co %d minut.\n\r", i, measurementInterval);
			CDC_Transmit_FS((uint8_t*)Message, MessageLen);
		}
	}

	// polecenie 6: odczyt aktualnego odstępu pomiędzy pomiarami (interwału)
	cmp = strncmp((char*)ReceivedData, strGetInterval, 8);
    if (cmp == 0) {
  		MessageLen = sprintf(Message, "Pomiar co %d minut.\n\r",
  				measurementInterval);
  		CDC_Transmit_FS((uint8_t*)Message, MessageLen);
    }

    /* polecenie 7: ustawienie aktualnego ustawienia określającego przez jaki
       czas ma odbywać się zapis pomiarów do jednego pliku (dzień lub tydzień)*/
    cmp = strncmp((char*)ReceivedData, strSetDaysPerFile, 8);
    if (cmp == 0) {
    	uint8_t dpf = ReceivedData[8];
    	if ((dpf == 0 ) || (dpf == 1)){
    		writeDaysPerFileSettingToBackupRegister(dpf);
    		MessageLen = sprintf(Message, "OK.\n\r");
    		CDC_Transmit_FS((uint8_t*)Message, MessageLen);
    	} else {
    		MessageLen = sprintf(Message, "Niepoprawna wartosc!\n\r");
    		CDC_Transmit_FS((uint8_t*)Message, MessageLen);
    	}
    }

    /* polecenie 8: odczyt aktualnego ustawienia określającego przez jaki czas
       ma odbywać się zapis pomiarów do jednego pliku (dzień lub tydzień) */
    cmp = strncmp((char*)ReceivedData, strGetDaysPerFile, 8);
    if (cmp == 0) {
    	enum period dayPerFileSetting = readDaysPerFileSettingFromBKUP();
    	if (dayPerFileSetting == WEEK) {
        	 MessageLen = sprintf(Message, "Pomiary tygodniowe w pliku.\n\r");
    	} else if(dayPerFileSetting == ONE_DAY) {
        	 MessageLen = sprintf(Message, "Pomiary dobowe w pliku.\n\r");
    	} else {
        	 MessageLen = sprintf(Message,"BLAD! Zla wartosc w rejestrze!\n\r");
    	}
    	CDC_Transmit_FS((uint8_t*)Message, MessageLen);
    }
}

/* zwraca numer obecego tygodnia w miesiącu (od 1 do 5) lub 0 - gdy obecny
   tydzień zaczął sie jeszcze w poprzednim miesiacu, lub 9 jeśli błąd */
uint8_t presentWeekInMonth()
{
	uint8_t week; /* numer tygodnia w miesiącu, 1 to pierwszy pełny tydzień
					 w niesiącu, 5 to ostatni (niepełny) */
	uint8_t dayOfWeek = Date.WeekDay;
	if (dayOfWeek == 0) {
		dayOfWeek = 7;
	}
	if (dayOfWeek > Date.Date) {
		return 0; //początek obecnego tygodnia należął do poprzedniego miesiąca
	}
	uint8_t lastMonday = Date.Date - (dayOfWeek - 1);
	if ((lastMonday >= 1) && (lastMonday < 8)) {
		week = 1;
	} else if ((lastMonday >= 8) && (lastMonday < 15)) {
		week = 2;
	} else if ((lastMonday >= 15) && (lastMonday < 22)) {
		week = 3;
	} else if ((lastMonday >= 22) && (lastMonday < 29)) {
		week = 4;
	} else if ((lastMonday >= 29) && (lastMonday < 32)) {
		week = 5;
	} else {
		return 9; //error
	}
	return week;
}

EPR_STAT specifyBeginingOfFileName(char* fileName, uint8_t *len)
{
	int textLen;
	uint8_t week;

	clearTableChar(fileName);
	enum period daysPerFile = readDaysPerFileSettingFromBKUP();
	switch (daysPerFile) {
		case WEEK:
			week = presentWeekInMonth();
			if (week > 5) {
				return G_ERR;
			}
			RTC_DateTypeDef DateInFileName;
			DateInFileName.Year 	= Date.Year;
			DateInFileName.Month 	= Date.Month;
			if (week == 0) { /* obecny tydzień zaczał się w poprzednim miesiącu,
								więc będzie 5 tygodniem poprzedniego miesiąca */
				if (DateInFileName.Month != 1) {
					DateInFileName.Month = DateInFileName.Month - 1;
					week = 5;
				} else {
					DateInFileName.Month = 12;
					DateInFileName.Year = DateInFileName.Year - 1;
					week = 5;
				}
			}
			textLen = snprintf(fileName, 7, "%02d%02dW%01d",
					DateInFileName.Year, DateInFileName.Month, week);
			break;

		case ONE_DAY:
			textLen = snprintf(fileName, 7, "%02d%02d%02d", Date.Year,
					Date.Month, Date.Date);
			break;

		default:
			return G_ERR; //error
	}
	*len = textLen;
	return S_OK;
}

EPR_STAT specifyFileName(char *outName)
{
	char startOfName[6];
	uint8_t len;
	char additionalFileID;
	FILINFO FileInfo;
	FRESULT FATres;
	DWORD maxFileSize = 51200; //50kB
	DWORD maxLineLen = 50;
	DWORD max = maxFileSize - maxLineLen - 1;
	_Bool findFlag = 0;
	#define NUM_ADDED_CHARACTERS	7	//(len + NUM_ADDED_CHARACTERS) < 14

	if (specifyBeginingOfFileName(startOfName, &len) != S_OK) {
		return G_ERR;
	}
	additionalFileID = 'A';
	while (!findFlag) {
		snprintf(outName, len + NUM_ADDED_CHARACTERS, "M%6s%c.CSV", startOfName,
				additionalFileID);
		FATres = f_stat(outName, &FileInfo);
		if ((FATres == FR_NO_FILE) || ((FATres == S_OK)
				&& (FileInfo.fsize <= max))) {
			findFlag = 1;
		} else if ((FATres == FR_OK) && (FileInfo.fsize > max)) {
			++additionalFileID;
	  		if (additionalFileID > 'Z') {
	  			return G_ERR;
	  		}
		} else {
			return G_ERR;
		}
	}
	return S_OK;
}

void saveDataToSDCard(float temperature, float pressure, float humidity)
{
	FILINFO FileInfo;
	FRESULT FATres;
	char record[41];
	uint8_t recordLen;
	_Bool recordHasBeenMadeFlag = 0;

	if(BSP_SD_Init() == FR_OK) {
		if (f_mount(&SDFatFS, SDPath, 1) == FR_OK) {
			char myPath[13];	//rozmiar tablicy <= 13							//max 13, jeśli więcej to nie działą
			if (specifyFileName(myPath) == S_OK) {
				FATres = f_stat(myPath, &FileInfo);
				if (FATres == FR_OK || FATres == FR_NO_FILE) {
					if (FATres == FR_NO_FILE) {
						f_open(&SDFile, myPath, FA_WRITE|FA_READ|FA_CREATE_NEW);
						f_puts(firstLineOfFile, &SDFile);
					} else {
					   f_open(&SDFile,myPath,FA_WRITE|FA_READ|FA_OPEN_EXISTING);
					}
					f_lseek(&SDFile, f_size(&SDFile));
					fileSize = f_size(&SDFile);
					pointer = f_tell(&SDFile);
					eof = f_eof(&SDFile);

					recordLen = snprintf(record, 41, "\n20%02d-%02d-%02d "
							"%02d:%02d:%02d,%.02f,%.02f,%.0f,", Date.Year,
							Date.Month, Date.Date, Time.Hours, Time.Minutes,
							Time.Seconds, temperature, pressure, humidity);
					if (f_write(&SDFile, record, recordLen, &nrWrittenBytes)
							== FR_OK) {
						recordHasBeenMadeFlag = 1;
					}
				}
			}
			f_close(&SDFile);
			f_mount(&SDFatFS, SDPath, 0);
		}
		HAL_SD_DeInit(&hsd);
	}
	if (recordHasBeenMadeFlag) {
		HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, LED_ON);					//wł. zielonej LED, jesli zapis do pliku się udał
		HAL_Delay(400);
		HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, LED_OFF);
	  } else {
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, LED_OFF);				//wył. czerwoną LED na 0,2s, jeśli zaspis do pliku się nie udał, mrógnięcie diodą
			HAL_Delay(200);
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, LED_ON);
			HAL_Delay(200);														//potrzebne, bo czerwona LED jest wł. podczas całego stanu pracy i gaszona przed samym uśpieniem.
	  }
}

