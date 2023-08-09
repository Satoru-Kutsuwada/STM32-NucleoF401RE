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

//#include <stdarg.h>

/* Public includes -----------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

/* Public define -------------------------------------------------------------*/


/* Public macro --------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;


/* Public function prototypes ------------------------------------------------*/




/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/



/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
LOG_DATA	log;

/* Private function prototypes -----------------------------------------------*/
void Set_logInfo2(const char *string, ...);
void GetTime_tim1up(TIMER_DATA *time);
UART_HandleTypeDef * Get_huart(SK_UART sel);

//==============================================================================
//
//==============================================================================

uint8_t *my_putint(int num, uint8_t *buf)
{
//	SKprintf("num=%d,num/10=%d,num%%10=%d\r\n",num,num/10,num%10);

	if (num < 0) {
        *buf = '-';
        buf++;
        num = -num;
    }

    if (num / 10 != 0) {
        buf = my_putint(num / 10, buf);
    }

    *buf = '0' + (num % 10);
    buf++;

    return buf;
}
//==============================================================================
//
//==============================================================================

uint8_t *my_putfloat(double num, int precision, uint8_t *buf)
{
	int dt;
	double fracPart,dtf;
	int intPart;
	int digit;

	intPart = (int)num;
    buf = my_putint(intPart, buf);
    *buf = '.';
    buf ++;

    fracPart = num - intPart;

    if (fracPart < 0) {
        fracPart = -fracPart;
    }

    int count = 0;
    while (count < precision) {
        fracPart *= 10;
        int digit = (int)fracPart;

        *buf = '0' + digit;
        buf ++;

        fracPart -= digit;
        count++;
    }

    *buf = '\0';

    return buf;

}
//=============================================================================
//
//
//=============================================================================

uint8_t *my_putchar(char c, uint8_t *buf )
{
	//SKprintf("_putchar c=%x, buf=%p\r\n",c,buf);
	*buf = c;
	buf ++;
	return buf ;
}

//=============================================================================
//
//
//=============================================================================
uint8_t *my_puts(char* str, uint8_t *buf )
{
	uint8_t *buf2;

	buf2=buf;

	//SKprintf("_puts str=%s, buf=%p\r\n",str,buf);
    while ( *str != '\0' ) {
        buf = my_putchar(*str, buf);
        str++;
    }
    *buf='\0';
	//SKprintf("_puts str=%s  %p\r\n",buf2, buf);
	return buf ;
}



const char ConvC[]= { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
//=============================================================================
//
//
//=============================================================================

uint8_t *my_putshex(uint16_t dt, uint8_t flg, uint8_t *buf )
{
	uint8_t	sw;

	switch(flg){
	case 4:
		*buf = ConvC[( dt >> 12 ) & 0x0f ];
		buf ++;
		*buf = ConvC[( dt >> 8 ) & 0x0f ];
		buf ++;
		*buf = ConvC[( dt >> 4 ) & 0x0f ];
		buf ++;
		*buf = ConvC[ dt  & 0x0f ];
		buf ++;
		break;

	case 2:
		*buf = ConvC[( dt >> 4 ) & 0x0f ];
		buf ++;
		*buf = ConvC[ dt  & 0x0f ];
		buf ++;
		break;
	case 0:
		sw = 0;
		if(( sw == 1) || (( dt >> 12 ) & 0x0f ) != 0 ){
			*buf = ConvC[( dt >> 12 ) & 0x0f ];
			buf ++;
			sw = 1;
		}
		if(( sw == 1) || (( dt >> 8 ) & 0x0f ) != 0 ){
			*buf = ConvC[( dt >> 8 ) & 0x0f ];
			buf ++;
			sw = 1;
		}
		if(( sw == 1) || (( dt >> 4 ) & 0x0f ) != 0 ){
			*buf = ConvC[( dt >> 4 ) & 0x0f ];
			buf ++;
			sw = 1;
		}

		*buf = ConvC[ dt & 0x0f ];
		buf ++;


		break;
	}
	return buf ;
}


//=============================================================================
//
//
//=============================================================================
void Set_logflg(LOG_FLAG flg)
{

	if( flg < LF_MAX){
		log.flg = flg;
		SKprintf("log.flg=%d \r\n",log.flg);
	}
}


//=============================================================================
//
//
//=============================================================================
void Set_logInfo(char *string)
{
//	RTC_TimeTypeDef sTime;
//	RTC_DateTypeDef sDate;
	TIMER_DATA time;

	int i;
	uint8_t	flg = 0;
	uint32_t dt;



	switch(log.flg){
	case LF_NON_STOP:
		break;
	case LF_IMMMEDIATE_STOP:
		flg = 1;
		break;
	case LF_MAX_DATA_STOP:
		if( log.num >= LOG_RECODE_MAX)
			flg = 1;
		break;
	default:
		break;
	}


	if( flg == 0 ){
		for(i=0; i<PRiNTF_BUFFMAX; i++){
			log.rec[log.wptr].string[i] = string[i];
			if(string[i] == '\0'){
				break;
			}
		}

//		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
//		HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
		GetTime_tim1up(&time);

		log.rec[log.wptr].Hours = time.hour;
		log.rec[log.wptr].Minutes = time.min;
		log.rec[log.wptr].Seconds = time.sec;
		log.rec[log.wptr].msec = time.msec;
		log.rec[log.wptr].usec = time.usec;

//		log.rec[log.wptr].dt = timer.dt;
//		log.rec[log.wptr].dt_av = timer.dt_av;


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
#ifdef ___NOP
#endif

char	loginfo_buf[128];
//=============================================================================
//
//
//=============================================================================
void Set_logInfo2(const char *string, ...)
{
//	RTC_TimeTypeDef sTime;
	TIMER_DATA time;

	va_list ap;
	int i;
	char *buffer, *temp;
	uint8_t	flg = 0;


    int 	intvalue;
    char	*charvalue;
    uint16_t uintvalue;
    uint32_t uint32value;
    double  floatvalue;



	temp = buffer = (char *)loginfo_buf;
//	temp = buffer = (char *)pvPortMalloc(128);
	//SKprintf("_logInfo2() 0001 temp= %p\r\n",temp);

	switch(log.flg){
	case LF_NON_STOP:
		break;
	case LF_IMMMEDIATE_STOP:
		flg = 1;
		break;
	case LF_MAX_DATA_STOP:
		//if( log.num >= LOG_RECODE_MAX)
		//	flg = 1;
		break;
	default:
		break;
	}

	if( flg == 0 ){
		va_start(ap, string);
//		vsprintf(buffer, string, ap);

		i = 0;
	    while (*string != '\0') {
	    	//*buffer = *string;
	    	//buffer ++;

	        if (*string == '%') {
	            string++; // Move past '%'
	            if (*string == 'd') {
	            	intvalue = va_arg(ap, int);

	                buffer = my_putint(intvalue, buffer);
	            }
#ifdef ___NOP
	            else if (*string == 'f') {
	            	//SKprintf("_logInfo2() 0002 val =%d\r\n",intvalue);
	            	uint32value = va_arg(ap, uint32_t);
	            	floatvalue = (float) uint32value;
	            	//SKprintf("_logInfo2() 0002 val =%g\r\n",floatvalue);
	            	//SKprintf("_logInfo2() 0010 val =%f\r\n",floatvalue);
	                buffer = my_putfloat(floatvalue, 2, buffer );
	               // SKprintf("val =%s\r\n",temp);

	            }
#endif
	            else if (*string == 'f') {
	            	//SKprintf("_logInfo2() 0002 val =%d\r\n",intvalue);
	            	floatvalue = va_arg(ap, double);
	            	//SKprintf("_logInfo2() 0002 val =%g\r\n",floatvalue);
	            	//SKprintf("_logInfo2() 0010 val =%f\r\n",floatvalue);
	                buffer = my_putfloat(floatvalue, 2, buffer );
	            }

	            //else if (*string == 'c') {
	            //    int value = va_arg(ap, int);
	            //    buffer = my_putchar(value, buffer );
	            //}
	            else if (*string == 's') {
	            	charvalue = va_arg(ap, char*);
	            	//SKprintf("_logInfo2() 0003 val =%s\r\n",charvalue);
	                buffer = my_puts(charvalue, buffer );
	            } else if (*string == 'x') {
	            	uintvalue = (uint16_t)va_arg(ap, int);
	            	//SKprintf("_logInfo2() 0004 val =%x\r\n",uintvalue);
	                buffer = my_putshex(uintvalue, 0, buffer );
	            } else {
	                buffer = my_puts('%', buffer );
	                buffer = my_puts(*string, buffer );
	            }
	        }
	        else {
	            buffer = my_putchar(*string, buffer );

//	        	buffer = my_puts(*string, buffer );
	        }
	        string++;
	    	//buffer++;
	    }

		*buffer = '\0';

	    //SKprintf("%s\r\n",temp);



    	//SKprintf("_logInfo2() 0005\r\n");


		va_end(ap);

		for(i=0; i<PRiNTF_BUFFMAX; i++){
			log.rec[log.wptr].string[i] = temp[i];
			if(temp[i] == '\0'){
				break;
			}
		}

		//SKprintf("CC %s\r\n", &log.rec[log.wptr].string[0]);

//		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		GetTime_tim1up(&time);

		log.rec[log.wptr].Hours = time.hour;
		log.rec[log.wptr].Minutes = time.min;
		log.rec[log.wptr].Seconds = time.sec;
		log.rec[log.wptr].msec = time.msec;
		log.rec[log.wptr].usec = time.usec;


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
	//SKprintf("_logInfo2() 0006 temp= %p\r\n",temp);
//	vPortFree(temp);
}

//=============================================================================
//
//
//=============================================================================
void LogInfo_display(void)
{
	uint16_t	i;
	uint16_t	rptr = log.rptr;

	SKprintf("LogInfo_display()\r\n");
	if( log.num != 0 ){
		for(i=0; i<LOG_RECODE_MAX; i++){
			SKprintf("%02d:%02d:%02d.", log.rec[rptr].Hours, log.rec[rptr].Minutes, log.rec[rptr].Seconds);

			SKprintf("%03d %02d0 ", log.rec[rptr].msec, log.rec[rptr].usec);

			SKprintf("%s\r\n", &log.rec[rptr].string[0]);

			rptr ++;
			if( rptr > LOG_RECODE_MAX ){
				rptr = 0;
			}
			if( log.wptr == rptr ){
				break;
			}
		}
	}
	else{
		SKprintf("LOG NONE\r\n");
	}
}

//=============================================================================
//
//
//=============================================================================
void LogdisplayISR(void)
{
	uint16_t	i,j;
	uint16_t	rptr = log.rptr;



	if( log.num != 0 ){
		HAL_UART_Transmit(Get_huart(SK_UART2_DEBUG), "\r\n", 3, HAL_MAX_DELAY);

		for(i=0; i<LOG_RECODE_MAX; i++){

			for(j=0; j<PRiNTF_BUFFMAX; j++ ){
				if( log.rec[rptr].string[j] == '\0'){
					break;
				}
			}
			//SKprintf(" %s\r\n", buf);
			//SKprintf("%s\r\n", &log.rec[rptr].string[0]);

			HAL_UART_Transmit(Get_huart(SK_UART2_DEBUG), &log.rec[rptr].string[0], j, HAL_MAX_DELAY);
			HAL_UART_Transmit(Get_huart(SK_UART2_DEBUG), "\r\n", 3, HAL_MAX_DELAY);

			rptr ++;
			if( rptr > LOG_RECODE_MAX ){
				rptr = 0;
			}

			if( log.wptr == rptr ){
				break;
			}
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

	SKprintf("LOG CLEAR \r\n");
}
