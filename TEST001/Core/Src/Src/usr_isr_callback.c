/*
 * usr_isr_callback.c
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



/* Public function prototypes ------------------------------------------------*/




/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
TIMER_TIC timer;


/* Private function prototypes -----------------------------------------------*/


//==============================================================================
//
//==============================================================================

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{

	uint8_t	i;

	if( timer.start > TIMER_AV_NUM){

		timer.dt_buf[timer.av_wcnt] = timer.dt;
		timer.av_wcnt ++;

		if(timer.av_wcnt > TIMER_AV_NUM){
			timer.av_wcnt = 0;
			timer.dt_av = 0;
			for(i=0; i<TIMER_AV_NUM; i++){
				timer.dt_av +=  timer.dt_buf[i];
			}
			timer.dt_av /= TIMER_AV_NUM;
		}

		if( timer.dt_max < timer.dt ){
			timer.dt_max = timer.dt;
		}

		timer.dt = 0;
	}
	else{
		timer.start ++;
		timer.dt = 0;
	}
}

//==============================================================================
//
//==============================================================================
void usr_isr_tim1_up(void)
{
	//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

	timer.dt += 10;

}

