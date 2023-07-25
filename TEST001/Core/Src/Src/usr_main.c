/*
 * usr_main.c
 *
 *  Created on: Jul 20, 2023
 *      Author: nosak
 */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include "stm32f4xx_hal_uart.h"
#include "usr_system.h"
#include <stdarg.h>

/* Public includes -----------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

/* Public define -------------------------------------------------------------*/
#define ___UART_POLING

/* Public macro --------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
extern UART_HandleTypeDef huart2;
extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim1;


/* Public function prototypes ------------------------------------------------*/




/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
int gUartReceived = 0;
int Sem_Printf = 0;


/* Private function prototypes -----------------------------------------------*/
void Get_UART_Handle(UART_HandleTypeDef *uart_handle);


//==============================================================================
//
// 総和を求める関数（値は int 型を想定）
// n は、渡す引数の数、それ以降は計算する値です。
//==============================================================================

int	SKprintf (const char *string, ...)
{
	va_list ap;
	int i;

	char buffer[PRiNTF_BUFFMAX];


	while( Sem_Printf != 0 );

	Sem_Printf = 1;

	// 可変個引数の利用準備
	// -- １… va_list 構造体 ap
	// -- 2 … 可変個引数の直前にある引数

	va_start(ap, string);
	vsprintf(buffer, string, ap);
	va_end(ap);

	for(i=0; i<PRiNTF_BUFFMAX; i++){
		if(buffer[i] == '\0'){
			break;
		}
	}
	HAL_UART_Transmit(&huart2, buffer, i, HAL_MAX_DELAY);

	Sem_Printf = 0;

}


/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
  *******************************************************************************/
void Get_UART_Handle(UART_HandleTypeDef *uart_handle)
{
	uart_handle = &huart2;
}


/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
  *******************************************************************************/
void user_init(void)
{
	//LED_Flush(0);
	SKprintf("Initialize all configured peripherals\r\n");
	SKprintf("******************\r\n");
	SKprintf("*** UART START ***\r\n");
	SKprintf("******************\r\n");



	//-----------------------------------------------
	// Log Timer Counter
	//-----------------------------------------------
	timer.usec = 0;
	timer.msec = 0;
	timer.usec_max = 0;
	timer.msec_max = 0;
	timer.dt = 0;
	timer.dt_max = 0;
	timer.dt_av = 0;
	timer.start = 0;
	timer.av_wcnt = 0;

	//-----------------------------------------------
	// Log Info Init
	//-----------------------------------------------
	LogInfo_clear();


}

/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
  *******************************************************************************/
void user_main_loop(void)
{
	debu_main();
}

#ifdef ___NOP
	uint8_t buffer[256];
	HAL_StatusTypeDef s;
	  RTC_TimeTypeDef sTime;
	  RTC_DateTypeDef sDate;

	  //PA5�?1に設�?
	 // GPIOA -> ODR = 0b0000000000100000;
	 // HAL_Delay(1000); //1秒�?つ
	  //PA全てのポ�?�トを0に設�?
	  //GPIOA -> ODR = 0b0000000000000000;
	  //HAL_Delay(1000); //1秒�?つ


#ifdef ___UART_POLING
	    s = HAL_UART_Receive(&huart2, buffer, 1, 3000);
	    if (s == HAL_TIMEOUT)
	    {
	      HAL_UART_Transmit(&huart2, "UART Timeout.\r\n", 15, 10);
	    }
	    else if (s == HAL_OK)
	    {
	      HAL_UART_Transmit(&huart2, buffer, 1, 10);
	    }
#else
		HAL_UART_Receive_IT(&huart2, buffer, 2);
		while (gUartReceived == 0)
		{
			;
		}
		HAL_UART_Transmit_IT(&huart2, buffer, 2);
		gUartReceived = 0;

#endif	// ___UART_POLING



		  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
		  SKprintf("20%02d.%02d.%02d %02d:%02d:%02d\r\n", sDate.Year, sDate.Month, sDate.Date, sTime.Hours, sTime.Minutes, sTime.Seconds);
		  SKprintf("av=%d max=%d\r\n",timer.dt_av,timer.dt_max);
		  SKprintf("dt_buf=%d, %d, %d\r\n", timer.dt_buf[0],timer.dt_buf[1],timer.dt_buf[2]);

		  HAL_Delay(500);


#endif	// ___NOP


#ifndef ___UART_POLING
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	gUartReceived = 1;
}

#endif	// ___UART_POLING


#ifdef ___NOP
int _write(int file, char *ptr, int len)
{
	UART_HandleTypeDef *uart_handle;

	Get_UART_Handle(uart_handle);
  HAL_UART_Transmit(uart_handle,(uint8_t *)ptr,len,10);
  return len;
}
#endif

//==============================================================================
//
//==============================================================================
void rtc_display(void)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	SKprintf("20%02d.%02d.%02d %02d:%02d:%02d\r\n", sDate.Year, sDate.Month, sDate.Date, sTime.Hours, sTime.Minutes, sTime.Seconds);
	SKprintf("av=%d max=%d\r\n",timer.dt_av,timer.dt_max);
	SKprintf("dt_buf=%d, %d, %d\r\n", timer.dt_buf[0],timer.dt_buf[1],timer.dt_buf[2]);
}
/*******************************************************************************
  * @brief
  * @param  None
  * @retval None
  *******************************************************************************/
int getch(void)
{
	uint8_t buffer[256];
	HAL_StatusTypeDef s;
	int rtn = 0;

	s = HAL_UART_Receive(&huart2, buffer, 1, HAL_MAX_DELAY);

	switch(s){
	case HAL_OK:
		rtn = (int) buffer[0];
		break;
	case HAL_ERROR:
	case HAL_BUSY:
	case HAL_TIMEOUT:

		break;
	}

	return rtn;
}

