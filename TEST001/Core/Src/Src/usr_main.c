/*
 * usr_main.c
 *
 *  Created on: Jul 20, 2023
 *      Author: nosak
 */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "croutine.h"

//#include "stm32f4xx_hal_uart.h"
#include "usr_system.h"
#include <stdarg.h>
#include "FreeRTOS.h"
#include "task.h"
/* Public includes -----------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

typedef struct tskTaskControlBlock 			/* The old naming convention is used to prevent breaking kernel aware debuggers. */
{
	volatile StackType_t	*pxTopOfStack;	/*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE TCB STRUCT. */

	#if ( portUSING_MPU_WRAPPERS == 1 )
		xMPU_SETTINGS	xMPUSettings;		/*< The MPU settings are defined as part of the port layer.  THIS MUST BE THE SECOND MEMBER OF THE TCB STRUCT. */
	#endif

	ListItem_t			xStateListItem;	/*< The list that the state list item of a task is reference from denotes the state of that task (Ready, Blocked, Suspended ). */
	ListItem_t			xEventListItem;		/*< Used to reference a task from an event list. */
	UBaseType_t			uxPriority;			/*< The priority of the task.  0 is the lowest priority. */
	StackType_t			*pxStack;			/*< Points to the start of the stack. */
	char				pcTaskName[ configMAX_TASK_NAME_LEN ];/*< Descriptive name given to the task when created.  Facilitates debugging only. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */

	#if ( ( portSTACK_GROWTH > 0 ) || ( configRECORD_STACK_HIGH_ADDRESS == 1 ) )
		StackType_t		*pxEndOfStack;		/*< Points to the highest valid address for the stack. */
	#endif

	#if ( portCRITICAL_NESTING_IN_TCB == 1 )
		UBaseType_t		uxCriticalNesting;	/*< Holds the critical section nesting depth for ports that do not maintain their own count in the port layer. */
	#endif

	#if ( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t		uxTCBNumber;		/*< Stores a number that increments each time a TCB is created.  It allows debuggers to determine when a task has been deleted and then recreated. */
		UBaseType_t		uxTaskNumber;		/*< Stores a number specifically for use by third party trace code. */
	#endif

	#if ( configUSE_MUTEXES == 1 )
		UBaseType_t		uxBasePriority;		/*< The priority last assigned to the task - used by the priority inheritance mechanism. */
		UBaseType_t		uxMutexesHeld;
	#endif

	#if ( configUSE_APPLICATION_TASK_TAG == 1 )
		TaskHookFunction_t pxTaskTag;
	#endif

	#if( configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0 )
		void			*pvThreadLocalStoragePointers[ configNUM_THREAD_LOCAL_STORAGE_POINTERS ];
	#endif

	#if( configGENERATE_RUN_TIME_STATS == 1 )
		uint32_t		ulRunTimeCounter;	/*< Stores the amount of time the task has spent in the Running state. */
	#endif

	#if ( configUSE_NEWLIB_REENTRANT == 1 )
		/* Allocate a Newlib reent structure that is specific to this task.
		Note Newlib support has been included by popular demand, but is not
		used by the FreeRTOS maintainers themselves.  FreeRTOS is not
		responsible for resulting newlib operation.  User must be familiar with
		newlib and must provide system-wide implementations of the necessary
		stubs. Be warned that (at the time of writing) the current newlib design
		implements a system-wide malloc() that must be provided with locks.

		See the third party link http://www.nadler.com/embedded/newlibAndFreeRTOS.html
		for additional information. */
		struct	_reent xNewLib_reent;
	#endif

	#if( configUSE_TASK_NOTIFICATIONS == 1 )
		volatile uint32_t ulNotifiedValue;
		volatile uint8_t ucNotifyState;
	#endif

	/* See the comments in FreeRTOS.h with the definition of
	tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE. */
	#if( tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE != 0 ) /*lint !e731 !e9029 Macro has been consolidated for readability reasons. */
		uint8_t	ucStaticallyAllocated; 		/*< Set to pdTRUE if the task is a statically allocated to ensure no attempt is made to free the memory. */
	#endif

	#if( INCLUDE_xTaskAbortDelay == 1 )
		uint8_t ucDelayAborted;
	#endif

	#if( configUSE_POSIX_ERRNO == 1 )
		int iTaskErrno;
	#endif

} SKtskTaskControlBlock;


/* Public define -------------------------------------------------------------*/
#define ___UART_POLING

/* Public macro --------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
extern UART_HandleTypeDef huart2;
extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim1;

extern osThreadId_t Task_mainHandle;
extern osThreadId_t Task_sub1Handle;
extern osThreadId_t Task_sub2Handle;

extern osThreadAttr_t Task_main_attributes;
extern osThreadAttr_t Task_sub1_attributes;
extern osThreadAttr_t Task_sub2_attributes;


/* Public function prototypes ------------------------------------------------*/




/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/


typedef struct {
	char 		name[configMAX_TASK_NAME_LEN];
	uint8_t 	*chk_stack;
	uint16_t	size;
	uint16_t	used;
} TASK_STACK_CHECK;

TASK_STACK_CHECK task_chk_table[SK_TASK_MAX];


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
int gUartReceived = 0;
int Sem_Printf = 0;


/* Private function prototypes -----------------------------------------------*/
void Get_UART_Handle(UART_HandleTypeDef *uart_handle);

//==============================================================================
//
//==============================================================================
void task_chk_init(void)
{
	SKtskTaskControlBlock *hTask;
	osThreadAttr_t		*attr;
	uint32_t 			*dtp;
	int		i;
	int		j;


	SKprintf("task_chk_init()\r\n");

	for( i=0; i < SK_TASK_MAX; i++ ){
		switch(i){
		case SK_TASK_main:
			dtp = &Task_mainHandle;
			attr = &Task_main_attributes;
			break;
		case SK_TASK_sub1:
			dtp = &Task_sub1Handle;
			attr = &Task_sub1_attributes;
			break;
		case SK_TASK_sub2:
			dtp = &Task_sub2Handle;
			attr = &Task_sub2_attributes;
			break;
		default:
			SKprintf("Error taskid\r\n");
			break;
		}
		hTask = (TaskHandle_t *)*dtp;

		for( j=0; j < configMAX_TASK_NAME_LEN; j++ ){
			task_chk_table[i].name[j] = hTask->pcTaskName[j];
		}
		task_chk_table[i].name[j-1] = '\0';

		task_chk_table[i].chk_stack = hTask->pxStack;
		task_chk_table[i].size = attr->stack_size;
		task_chk_table[i].used = 0;

		SKprintf(" %s\r\n", &task_chk_table[i].name[0]);
		SKprintf("  Check Start Address = %p\r\n", task_chk_table[i].chk_stack);
		SKprintf("  Size          = %d\r\n", task_chk_table[i].size);
		SKprintf("  Used Size     = %d(%d%%)\r\n\r\n", task_chk_table[i].used, 100*task_chk_table[i].used/task_chk_table[i].size);

	}
}

//==============================================================================
//
//==============================================================================
void task_stack_chk(void)
{
	int		i;
	int		j;


	//SKprintf("\r\ntask_stack_chk()\r\n");
	for( i=0; i < SK_TASK_MAX; i++ ){

		for( j= 0; j < task_chk_table[i].size; j++){
			if(task_chk_table[i].chk_stack[j] != 0xa5){
				break;
			}
		}
		task_chk_table[i].used = task_chk_table[i].size - j;

		//SKprintf(" %s:Used Size= %d(%d%%)\r\n", &task_chk_table[i].name[0], task_chk_table[i].used, (100*task_chk_table[i].used/task_chk_table[i].size));

		if( j < (task_chk_table[i].size/10) ){
			SKprintf("WARNING:STACK FULL  %s\r\n", &task_chk_table[i].name[0]);
		}

	}
}
//==============================================================================
//
//==============================================================================
void Disp_task_info(SK_TASK taskid)
{
	SKtskTaskControlBlock *hTask;
	osThreadAttr_t		*attr;
	uint32_t 			*dtp;

	switch(taskid){
	case SK_TASK_main:
		dtp = &Task_mainHandle;
		hTask = (TaskHandle_t *)*dtp;
		attr = &Task_main_attributes;
		break;
	case SK_TASK_sub1:
		dtp = &Task_sub1Handle;
		hTask = (TaskHandle_t *)*dtp;
		attr = &Task_sub1_attributes;
		break;
	case SK_TASK_sub2:
		dtp = &Task_sub2Handle;
		hTask = (TaskHandle_t *)*dtp;
		attr = &Task_sub2_attributes;
		break;
	default:
		SKprintf("Error taskid\r\n");
		break;
	}

	SKprintf("<<< %s >>>\r\n" ,attr->name);
	SKprintf(" TCB Address           = %p\r\n" ,hTask);
	SKprintf(" Stack Botom Address   = 0x%lx\r\n", hTask->pxTopOfStack );
	SKprintf(" Stack top Address     = 0x%lx\r\n", hTask->pxStack );
	SKprintf(" stack_size            = %lx\r\n" ,attr->stack_size);
	SKprintf(" uxTCBNumber           = %lx\r\n", hTask->uxTCBNumber );
	SKprintf(" uxTaskNumber          = %lx\r\n", hTask->uxTaskNumber );
	SKprintf(" uxBasePriority        = %lx\r\n", hTask->uxBasePriority );
	SKprintf(" uxMutexesHeld         = %lx\r\n", hTask->uxMutexesHeld );
	SKprintf(" ulNotifiedValue       = %lx\r\n", hTask->ulNotifiedValue );
	SKprintf(" ucNotifyState         = %lx\r\n", hTask->ucNotifyState );
	SKprintf(" ucStaticallyAllocated = %lx\r\n", hTask->ucStaticallyAllocated );

#ifdef ___NOP

	SKprintf(" name = %s\r\n" ,attr->name);
	SKprintf(" attr_bits = %lx\r\n" ,attr->attr_bits);
	//SKprintf(" *cb_mem = %p\r\n" ,*attr->cb_mem);
	SKprintf(" cb_size = %lx\r\n" ,attr->cb_size);
	//SKprintf(" *stack_mem = %p\r\n" ,*attr->stack_mem);
	SKprintf(" priority = %lx\r\n" ,attr->priority);
	SKprintf(" tz_module = %lx\r\n" ,attr->tz_module);
	SKprintf(" reserved = %lx\r\n" ,attr->reserved);

	SKprintf(" ---------------\r\n");




	dtp = (uint32_t *)hTask;
	SKprintf("TCB Address= 0x%lx\r\n" ,*dtp);


	//SKprintf("\r\nTASK CONTROL BLOCK\r\n");

	dt = &hTask->pxStack;
	dtp = &hTask->pxStack;
	SKprintf(" dt= 0x%lx\r\n", dt );
	SKprintf(" dtp= %p\r\n", dt );
	SKprintf(" *dtp= 0x%lx\r\n", *dtp );





	SKprintf(" pxTopOfStack = %p\r\n", hTask->pxTopOfStack );
	SKprintf(" pxTopOfStack* = %x\r\n", *hTask->pxTopOfStack );
	SKprintf(" pxTopOfStack dt = %lx\r\n", dt );


	dt = (uint32_t )*hTask->pxStack;
	SKprintf(" pxStack = %p\r\n", hTask->pxStack );
	SKprintf(" pxStack* = %x\r\n", *hTask->pxStack );
	SKprintf(" pxStack dt = %x\r\n", dt );
	dt = (uint32_t )hTask->pxStack;
	SKprintf(" pxStack dt2 = %x\r\n", dt );





	hTask->pcTaskName[configMAX_TASK_NAME_LEN-1] = '\0';
	SKprintf(" pcTaskName = %s\r\n", &hTask->pcTaskName[0]);

	typedef struct {
	  const char                   *name;   ///< name of the thread
	  uint32_t                 attr_bits;   ///< attribute bits
	  void                      *cb_mem;    ///< memory for control block
	  uint32_t                   cb_size;   ///< size of provided memory for control block
	  void                   *stack_mem;    ///< memory for stack
	  uint32_t                stack_size;   ///< size of stack
	  osPriority_t              priority;   ///< initial thread priority (default: osPriorityNormal)
	  TZ_ModuleId_t            tz_module;   ///< TrustZone module identifier
	  uint32_t                  reserved;   ///< reserved (must be 0)
	} osThreadAttr_t;


	SKprintf("\r\nTASK ATTRIBUTE\r\n");
	SKprintf(" name = %s\r\n" ,attr->name);
	SKprintf(" attr_bits = %lx\r\n" ,attr->attr_bits);
	SKprintf(" *cb_mem = %p\r\n" ,*attr->cb_mem);
	SKprintf(" cb_size = %lx\r\n" ,attr->cb_size);
	SKprintf(" *stack_mem = %p\r\n" ,*attr->stack_mem);
	SKprintf(" stack_size = %lx\r\n" ,attr->stack_size);
	SKprintf(" priority = %lx\r\n" ,attr->priority);
	SKprintf(" tz_module = %lx\r\n" ,attr->tz_module);
	SKprintf(" reserved = %lx\r\n" ,attr->reserved);
#endif	// ___NOP


}


//==============================================================================
//
//==============================================================================
void Get_task_stackptr(SK_TASK taskid, STACK_INFO *ptr)
{
	SKtskTaskControlBlock *hTask;
	uint32_t 			*dtp;

	switch(taskid){
	case SK_TASK_main:
		dtp = &Task_mainHandle;
		hTask = (TaskHandle_t *)*dtp;
		break;
	case SK_TASK_sub1:
		hTask = (TaskHandle_t)&Task_sub1Handle;
		dtp = &Task_sub1Handle;
		hTask = (TaskHandle_t *)*dtp;
		break;
	case SK_TASK_sub2:
		dtp = &Task_sub2Handle;
		hTask = (TaskHandle_t *)*dtp;
		break;
	default:
		SKprintf("Error taskid\r\n");
		break;
	}

	SKprintf(" Stack top Address     = 0x%lx\r\n", hTask->pxStack );



	ptr->pxStack = (char *)hTask->pxStack;
	ptr->pxTopOfStack = (char *)hTask->pxTopOfStack;
	ptr->size = (uint16_t)(128*4);

	SKprintf("pxStack=%p,pxTopOfStack=%p,size=%d\r\n", ptr->pxStack,ptr->pxTopOfStack,ptr->size);
}

//==============================================================================
//
// 総和を求める関数（値は int 型を想定）
// n は、渡す引数の数、それ以降は計算する値です。
//==============================================================================

int	SKprintf (const char *string, ...)
{
	va_list ap;
	int i;
	char *buffer;

#define CHARA_MAX 100

	while( Sem_Printf != 0 );

	buffer = (char *)pvPortMalloc(CHARA_MAX);

	if( buffer != NULL ){

		Sem_Printf = 1;

		// 可変個引数の利用準備
		// -- １… va_list 構造体 ap
		// -- 2 … 可変個引数の直前にある引数

		va_start(ap, string);
		vsprintf(buffer, string, ap);
		va_end(ap);

		for(i=0; i<CHARA_MAX; i++){
			if(buffer[i] == '\0'){
				break;
			}
		}
		HAL_UART_Transmit(&huart2, buffer, i, HAL_MAX_DELAY);

	}

	vPortFree(buffer);

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

