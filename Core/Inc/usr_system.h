/*
 * usr_system.h
 *
 *  Created on: Jul 21, 2023
 *      Author: nosak
 */

#ifndef INC_USR_SYSTEM_H_
#define INC_USR_SYSTEM_H_


/* Public includes -----------------------------------------------------------*/

#define TIMER_AV_NUM	3

/* Public typedef ------------------------------------------------------------*/
typedef struct{
	uint32_t	dt;
	uint32_t	dt_buf[TIMER_AV_NUM];
	uint32_t	dt_max;
	uint32_t	dt_av;
	uint8_t		av_wcnt;
	uint8_t		start;

	uint16_t	usec;
	uint16_t	msec;
	uint16_t	usec_max;
	uint16_t	msec_max;
} TIMER_TIC;



/* Public define -------------------------------------------------------------*/
#define PRiNTF_BUFFMAX 50

/* Public macro --------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
extern TIMER_TIC timer;


/* Public function prototypes ------------------------------------------------*/

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc);
void usr_isr_tim1_up(void);
void LogInfo_clear(void);
void Set_logInfo(const char *string, ...);
void LogInfo_display(void);
void debu_main(void);

int getch(void);

#endif /* INC_USR_SYSTEM_H_ */
