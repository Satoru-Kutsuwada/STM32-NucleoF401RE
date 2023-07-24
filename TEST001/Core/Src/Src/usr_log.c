/*
 * usr_log.c
 *
 *  Created on: Jul 21, 2023
 *      Author: nosak
 */


/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usr_system.h"

#include <stdarg.h>

/* Public includes -----------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

/* Public define -------------------------------------------------------------*/


/* Public macro --------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;


/* Public function prototypes ------------------------------------------------*/




/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/



typedef struct{

	// RTC_TimeTypeDef -----------------------------------------------------
	uint8_t Hours;
	uint8_t Minutes;
	uint8_t Seconds;

	// TIMER_TIC ------------------------------------------------------------
	uint32_t	dt;
	uint32_t	dt_av;

	// LOG ------------------------------------------------------------------
	char	string[PRiNTF_BUFFMAX] ;

} LOG_RECODE;

#define LOG_RECODE_MAX	100

typedef enum{
	NON_STOP = 0,
	IMMMEDIATE_STOP,
	MAX_DATA_STOP
} LOG_FLAG;

typedef struct{
	uint16_t 	wptr;
	uint16_t 	rptr;
	uint16_t 	num;
	LOG_FLAG		flg;

	LOG_RECODE	rec[LOG_RECODE_MAX];
} LOG_DATA;


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
LOG_DATA	log;

/* Private function prototypes -----------------------------------------------*/

//=============================================================================
//
//
//=============================================================================
void Set_logInfo(const char *string, ...)
{
	RTC_TimeTypeDef sTime;
	va_list ap;
	int i;
	char buffer[PRiNTF_BUFFMAX];
	uint8_t	flg = 0;



	switch(log.flg){
	case NON_STOP:
		break;
	case IMMMEDIATE_STOP:
		flg = 1;
		break;
	case MAX_DATA_STOP:
		if( log.num >= LOG_RECODE_MAX)
			flg = 1;
		break;
	default:
		break;
	}

	if( flg == 0 ){
		va_start(ap, string);
		vsprintf(buffer, string, ap);
		va_end(ap);

		for(i=0; i<PRiNTF_BUFFMAX; i++){
			log.rec[log.wptr].string[i] = buffer[i];
			if(buffer[i] == '\0'){
				break;
			}
		}

		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

		log.rec[log.wptr].Hours = sTime.Hours;
		log.rec[log.wptr].Minutes = sTime.Minutes;
		log.rec[log.wptr].Seconds = sTime.Seconds;
		log.rec[log.wptr].dt = timer.dt;
		log.rec[log.wptr].dt_av = timer.dt_av;

		log.num ++;
		if( log.num > LOG_RECODE_MAX ){
			log.num = LOG_RECODE_MAX;
		}

		log.wptr ++;
		if( log.wptr > LOG_RECODE_MAX ){
			log.wptr = 0;
		}

		if( log.wptr == log.rptr ){
			log.rptr ++;
			if( log.rptr > LOG_RECODE_MAX ){
				log.rptr = 0;
			}
		}
	}
}

//=============================================================================
//
//
//=============================================================================
void LogInfo_display(void)
{
	uint16_t	i;
	uint16_t	msec;
	uint16_t	usec;
	uint32_t	dt;
	uint16_t	rptr = log.rptr;


	for(i=0; i<LOG_RECODE_MAX; i++){
		SKprintf("%02d:%02d:%02d.", log.rec[rptr].Hours, log.rec[rptr].Minutes, log.rec[rptr].Seconds);

		dt = 1000000 * log.rec[rptr].dt / log.rec[rptr].dt_av;
		msec = (uint16_t)( dt / 1000 );
		usec = (uint16_t)( dt % 1000 );
		SKprintf("%03d %03d ", msec,usec);

		SKprintf("%s\r\n", &log.rec[log.wptr].string[0]);

		rptr ++;
		if( rptr > LOG_RECODE_MAX ){
			rptr = 0;
		}
		if( log.wptr == rptr ){
			break;
		}
	}
}

//=============================================================================
//
//
//=============================================================================
void LogInfo_clear(void)
{

	log.rptr = 0;
	log.wptr = 0;
	log.num = 0;

}