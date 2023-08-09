/*
 * usr_rs485_main.c
 *
 *  Created on: 2023/07/31
 *      Author: nosak
 */


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "usr_system.h"
#include "rs485_com.h"
#include <stddef.h>


#include <stdarg.h>
/* Public includes -----------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

/* Public define -------------------------------------------------------------*/

/* Public macro --------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;

extern LOG_DATA	log;
/* Public function prototypes ------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
#define RS485_TX	GPIO_PIN_SET
#define RS485_RX	GPIO_PIN_RESET

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define RS485_ADD_ID	0x00
#define RS485_CMD_ID	0x03

#define RS485_ADD_BYTE	3
#define RS485_SUM_BYTE	2

#define RS485_COM_01_BYTE	6
#define RS485_COM_02_BYTE	7
#define RS485_COM_03_BYTE	3
#define RS485_COM_04_BYTE	7


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
typedef struct {
	uint8_t		resp_dt;
	uint8_t		status;
	uint8_t		status_err;
	uint8_t		sens_kind;
	uint16_t	slv_ver;
	uint16_t	sens_ver;
	float		sens_dt;
}SLAVE_DATA;

SLAVE_DATA slv_dt[RS485_AD_MAX];

/* Private variables ---------------------------------------------------------*/

uint8_t		work_buf[RCV_BUF_SIZE];
uint8_t		work_buf_num;
UART_BUF	uart[SK_UART_MAX];



/* Private variables ---------------------------------------------------------*/
#define TEXT_LENGTH		6
const char com_start_text[]   = "COM-ST\0";
const char res_start_text[]   = "RES-ST\0";
const char message_end_text[] = "MSGEND\0";
char comp_buf[TEXT_LENGTH+1];

/* Private variables ---------------------------------------------------------*/
typedef enum{
	// スタートテキスト
	COM_START_TXT_00 = 0,
	COM_START_TXT_01,
	COM_START_TXT_02,
	COM_START_TXT_03,
	COM_START_TXT_04,
	COM_START_TXT_05,

	// 通信カウンタ
	COM_COUNTER_L,
	COM_COUNTER_H,

	// アドレス
	COM_ADDRESS_ID,
	COM_ADDRESS_DIST,
	COM_ADDRESS_SORC,
	COM_ADDRESS_RESERVD,

	// コマンド・レスポンス
	COM_COMMAND_ID,
	COM_COMMAND,
	COM_COMMAND_RESP,
	COM_COMMAND_01,
	COM_COMMAND_02,
	COM_COMMAND_03,
	COM_COMMAND_04,
	COM_COMMAND_05,
	COM_COMMAND_06,
	COM_COMMAND_07,
	COM_COMMAND_08,
	COM_COMMAND_09,
	COM_COMMAND_10,
	COM_COMMAND_11,

	//　チェックサム
	COM_CHKSUM_ID,
	COM_CHKSUM,

	// エンドテキスト
	COM_END_TXT_00,
	COM_END_TXT_01,
	COM_END_TXT_02,
	COM_END_TXT_03,
	COM_END_TXT_04,
	COM_END_TXT_05,

	COM_TABLE_MAX

}COM_TABLE;


/* Command 01 -------------------------------*/
/* Command 01 RESPONS -----------------------*/
#define COM_ERR_DITAIL 		COM_COMMAND_01
#define COM_SES_KIND 		COM_COMMAND_02

/* Command 02 ---------*/
/* Command 02 RESPONS -----------------------*/
#define COM_SLV_VERSION_L 	COM_COMMAND_01
#define COM_SLV_VERSION_H 	COM_COMMAND_02
#define COM_SNS_VERSION_L 	COM_COMMAND_03
#define COM_SNS_VERSION_H 	COM_COMMAND_04

/* Command 03 ---------*/
#define COM_SENS_CTRL 		COM_COMMAND_01

/* Command 03 RESPONS -----------------------*/

/* Command 04 ---------*/
#define COM_DATA_KIND 		COM_COMMAND_01

/* Command 04 RESPONS -----------------------*/
#define COM_MESUR_DATA_L 	COM_COMMAND_01
#define COM_MESUR_DATA_ML 	COM_COMMAND_02
#define COM_MESUR_DATA_MH 	COM_COMMAND_03
#define COM_MESUR_DATA_H 	COM_COMMAND_04


uint8_t		Cmd_mesg[COM_TABLE_MAX];
uint8_t		Res_mesg[COM_TABLE_MAX];
uint8_t		cmd_char[COM_TABLE_MAX];
uint8_t		cmd_ptr = 0;
uint8_t		res_ptr = 0;
uint16_t	com_counter = 0;


uint8_t		Sem_rs485_rcv = 0;
ESC_SEQ		esc;


/* Private variables ---------------------------------------------------------*/


const CMD_MSG	com[] = {
	{ RS485_CMD_MESUR_DATA, RS485_AD_SLEVE01, 	0 },
//	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE01, 	0 },
	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE02, 	0 },
	{ RS485_CMD_VERSION, 	RS485_AD_SLEVE01, 	0 },
	{ RS485_CMD_VERSION, 	RS485_AD_SLEVE02, 	0 },
	{ RS485_CMD_MESUR, 		RS485_AD_SLEVE01, 	1 },
	{ RS485_CMD_MESUR, 		RS485_AD_SLEVE02, 	1 },
	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE01, 	0 },
	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE02, 	0 },
	{ RS485_CMD_MESUR_DATA, RS485_AD_SLEVE01, 	0 },
	{ RS485_CMD_MESUR_DATA, RS485_AD_SLEVE02, 	0 },
	{ RS485_CMD_MAX, 		0, 					0 }
};

/* Private variables ---------------------------------------------------------*/

typedef struct{
	uint8_t			address_id;
	uint8_t			command_id;
	uint8_t			chksum_id;
} COMMAND_FORM;

const COMMAND_FORM	com_form[] = {

		{ 0, 0, 0 },		// コマンドが１スタートのためダミーデータを入れる

		{ 0, 3, 9 },
		{ 0, 3, 10 },
		{ 0, 3, 6 },
		{ 0, 3, 10 },
};


typedef enum{
	COM_PROTOCOL_SEND,
	COM_PROTOCOL_RECIVE,
	COM_PROTOCOL_RESPONS

} COM_PROTOCOL_STEP;

typedef enum {
    COM_RCV_INIT = 0,
    COM_RCV_ADD_ID,
    COM_RCV_ADD_ID_DIST,
    COM_RCV_ADD_ID_SOURCE,
    COM_RCV_CMD_ID,
    COM_RCV_COMMAND,
    COM_RCV_CSUM_ID,
    COM_RCV_CSUM,
    COM_RCV_COMPLITE

}COM_STEP;

COM_STEP    com_step_flg;




CMD_MSG				RTtaskISR;
/* Private function prototypes -----------------------------------------------*/





//RETURN_STATUS Send_rx485_cmd_message( CMD_MSG *com_msg );

RETURN_STATUS Send_rx485_cmd_message( CMD_MSG	 *com_msg );
UART_HandleTypeDef * Get_huart(void);

uint16_t  Get_end_test_pt(uint16_t num,uint8_t *buf );
RETURN_STATUS  Set_Res_Message(uint16_t num, uint8_t *src, uint8_t *dist);
uint8_t Get_rcv_data(SK_UART sel);
uint8_t Get_command_chksum(uint8_t start, uint8_t end,uint8_t *dt );
osMessageQueueId_t GetMessageQue(SK_TASK task);

void uart_Rcv_init(SK_UART sel);

void RS_nop(CMD_MSG	*rt_task );
void RScomand_send(CMD_MSG	*rt_task );
void RSrespons_recive(CMD_MSG	*rt_task );
void RSrespons_proc(CMD_MSG	*rt_task );
void RSTimeout( CMD_MSG	*rt_task );
void RSstop_req( CMD_MSG	*rt_task );
void SendMsgQueISR(RS485_TASK_EVENT event, uint8_t task);
void SendMsgQue(CMD_MSG	*rt_task );
void Set_logInfo2(const char *string, ...);

uint8_t *log_txt_conv(uint8_t *buffer, uint8_t *st1, uint8_t *st2, uint8_t dt1, uint8_t dt2 );

void LogdisplayISR(void);

//==============================================================================
//
//==============================================================================
const void (*rs485_func_table[RT_EVENT_MAX][RT_STATE_MAX])( CMD_MSG	*rt_task )={
	// 	_INIT 		_READY			_RESPONS_RECIVE		_RESPONS

		{ RS_nop,	RScomand_send,	RS_nop,				RS_nop			},	// RT_EVENT_COMMAND_REQ
		{ RS_nop,	RS_nop,			RSrespons_recive,	RS_nop			},	// RT_EVENT_UART_RX
		{ RS_nop,	RS_nop,			RS_nop,				RSrespons_proc	},	// RT_EVENT_RESPONS
		{ RS_nop,	RSstop_req,		RSstop_req,			RSstop_req		},	// RT_EVENT_STOP_REQ
		{ RS_nop,	RS_nop,			RSTimeout,			RS_nop			}	// RT_EVENT_TIMEOUT
};





//==============================================================================
//
//==============================================================================
void  tasuk3_init(void)
{
	SKprintf("tasuk3_init(void)\r\n");
}




//==============================================================================
//
//==============================================================================
void Set_rcv_data(SK_UART sel)
{
	uart[sel].Set_cnt++;

	while(uart[sel].Sem_rs485_rcv==1);

	uart[sel].Sem_rs485_rcv= 1;
	uart[sel].rcvnum ++;
	uart[sel].Sem_rs485_rcv= 0;

	uart[sel].rcvbuf[uart[sel].rcv_wpt] = uart[sel].rcv_dt[0];
//	uart[sel].rcvnum ++;
	uart[sel].totalnum ++;

	uart[sel].rcv_wpt ++ ;
    if( uart[sel].rcv_wpt > RCV_BUF_SIZE ){
    	uart[sel].rcv_wpt = 0;
    }
}
//==============================================================================
//
//==============================================================================
uint8_t Get_rcv_data(SK_UART sel)
{
    uint8_t dt;
	uart[sel].Get_cnt++;

  	while(uart[sel].Sem_rs485_rcv==1);

	uart[sel].Sem_rs485_rcv = 1;
	uart[sel].rcvnum --;
	uart[sel].Sem_rs485_rcv = 0;

	dt =  uart[sel].rcvbuf[uart[sel].rcv_rpt];
	uart[sel].rcv_rpt ++ ;
	if( uart[sel].rcv_rpt > RCV_BUF_SIZE ){
		uart[sel].rcv_rpt = 0;
	}

    return dt;

}

//==============================================================================
//　受信データ割込みコールバック
//==============================================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	if( Get_uart_port(huart) == SK_UART1_RS485 ){
		Set_rcv_data(SK_UART1_RS485);
		uart_Rcv_init(SK_UART1_RS485);
		if(uart[SK_UART1_RS485].totalnum >= COM_TABLE_MAX){
			Set_logInfo("HAL_UART_RxCpltCallback(RS485)");
			SendMsgQueISR(RT_EVENT_UART_RX, 0xff);
		}
	}
	else if( Get_uart_port(huart) == SK_UART2_DEBUG ){
		//Set_logInfo("HAL_UART_RxCpltCallback(DEBUG)");

		if(uart[SK_UART2_DEBUG].rcv_dt[0] == '!'){
			esc.buf[esc.ptr] = uart[SK_UART2_DEBUG].rcv_dt[0];
			esc.ptr++;
		}
		else if(esc.ptr == 1){
			esc.buf[esc.ptr] = uart[SK_UART2_DEBUG].rcv_dt[0];
			esc.ptr = 0;

			// Esc + L
			if(esc.buf[0]=='!' && esc.buf[1]=='l'){
				LogdisplayISR();

			}
		}

		Set_rcv_data(SK_UART2_DEBUG);
		uart_Rcv_init(SK_UART2_DEBUG);
	}
	else{
	}
}



//==============================================================================
//　割込み用
//==============================================================================
void SendMsgQueISR(RS485_TASK_EVENT event, uint8_t task)
{
	osStatus_t 			osStatus;
	MESSAGE_QUE_DATA	*msg;


	//msg = (MESSAGE_QUE_DATA *)pvPortMalloc(sizeof(MESSAGE_QUE_DATA));

	msg = &RTtaskISR;
	msg->u.cmd_msg.event = event;
	msg->send_task = task;
	msg->maroc_ptr = (void *)msg;

	//SKprintf("event=0x%x, task=0x%x,msgpt=%p\r\n", msg->event,msg->send_task, msg->maroc_ptr);
	//   	lsize2 = xPortGetFreeHeapSize();
	// 		SKprintf("lsize1=0x%x,lsize2=%x\r\n",lsize1,lsize2);
	//    	SKprintf("MESSAGE_QUE_DATA=%p\r\n",msg);
	osStatus = osMessageQueuePut (GetMessageQue(SK_TASK_sub2), (void *)msg->maroc_ptr, 0,0);
	if( osStatus == osOK ){
		//Set_logInfo("Rs485 RxRecived. Send MsgQue OK");
	}
	else{
		Set_logInfo("Rs485 RxRecived. Send MsgQue ERROR");
	}
}

//==============================================================================
//
//==============================================================================
void SendMsgQue( CMD_MSG	*rt_task )
{
	osStatus_t 			osStatus;
	MESSAGE_QUE_DATA	*msg;



	msg = (MESSAGE_QUE_DATA *)pvPortMalloc(sizeof(MESSAGE_QUE_DATA));

	#ifdef	 __HEAP_DBUG
	Set_logInfo2("pvPortMalloc=%x",msg);
#endif	//	 __HEAP_DBUG

	msg->u.cmd_msg.event = rt_task->event;
	//msg->send_task = task;
	msg->maroc_ptr = (void *)msg;

	msg->u.cmd_msg.address = rt_task->address;
	msg->u.cmd_msg.command = rt_task->command;
	msg->u.cmd_msg.command_sub = rt_task->command_sub;
	msg->u.cmd_msg.sub1 = rt_task->sub1;


	//SKprintf("event=0x%x, task=0x%x,msgpt=%p\r\n", msg->event,msg->send_task, msg->maroc_ptr);
	//   	lsize2 = xPortGetFreeHeapSize();
	// 		SKprintf("lsize1=0x%x,lsize2=%x\r\n",lsize1,lsize2);
	//    	SKprintf("MESSAGE_QUE_DATA=%p\r\n",msg);
	osStatus = osMessageQueuePut (GetMessageQue(SK_TASK_sub2), (void *)msg->maroc_ptr, 0,0);
	if( osStatus == osOK ){
		//Set_logInfo2("Rs485 RxRecived. Send MsgQue OK");
	}
	else{
		Set_logInfo2("Send MsgQue ERROR=%d",osStatus);
	}
}


//==============================================================================
//
//==============================================================================
//extern osMessageQueueId_t myQueue01Handle;



void rs485_com_task(void)
{
#ifdef ___NOP
	RETURN_STATUS		status = RET_TRUE;
		COM_PROTOCOL_STEP	cp_step = COM_PROTOCOL_SEND;
		uint8_t 			num = 0;
//		uint8_t 	dt;
		float		dtf;
//		float		dtf2;
		uint8_t		*pt;
//		uint8_t		*pt2;
		uint16_t	dt16;
		uint32_t	dt32;
		uint8_t 	*msgQueBuf;
		osStatus_t	*rtn;
		void 		*void_ptr;

#endif	// ___NOP

	CMD_MSG				RTtask;
	osStatus_t			os_status;
	MESSAGE_QUE_DATA	*msg;
	uint8_t				msgQueBuf[sizeof(void *)];
	uint8_t				i,j;
	uint8_t				event;
	uint8_t				state;
	uint32_t			timer;
	uint8_t				*temp;
	uint8_t 			*buffer;

	RTtask.state = RT_STATE_INIT;
	timer = osWaitForever;
	RTtask.state = RT_STATE_READY;




	while(1){

		//-------------------------------------------------------------------------
		//	メッセージ待ち
		//-------------------------------------------------------------------------
		os_status = osMessageQueueGet (GetMessageQue(SK_TASK_sub2), &msgQueBuf, 0, osWaitForever);
		msg = (MESSAGE_QUE_DATA *)msgQueBuf;
		msg = (MESSAGE_QUE_DATA	*)msg->maroc_ptr;


		event = 0;
		state = 0;
		switch( os_status ){
		case osOK:
			RTtask.event = msg->u.cmd_msg.event;

			switch(RTtask.event){
			case RT_EVENT_START_REQ:
				RTtask.command = msg->u.cmd_msg.command;
				RTtask.command_sub = msg->u.cmd_msg.command_sub;
				RTtask.address = msg->u.cmd_msg.address;
				RTtask.sub1 = msg->u.cmd_msg.sub1;
				break;
			case RT_EVENT_STOP_REQ:
				RTtask.command_sub = 1;
				break;
			default:
				break;
			}

			event = RTtask.event;
			state = RTtask.state;
			break;

		case osErrorTimeout:
		default:
			SKprintf("rs485_com_task():os_status=Error(%d)\r\n",os_status);
			break;
		}

		if( msg->maroc_ptr != &RTtaskISR ){
			vPortFree(msg->maroc_ptr);

#ifdef	 __HEAP_DBUG
			Set_logInfo2("vPortFree=%x",msg);
#endif	//	 __HEAP_DBUG

		}

#ifdef	 __HEAP_DBUG
		Set_logInfo2("HeapSize 002 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG


		//-------------------------------------------------------------------------
		//	処理関数
		//-------------------------------------------------------------------------
		//Set_logInfo2("RS485 MAIN: event=%d, state=%d\r\n",event,state);
		(*rs485_func_table[event][state ])( &RTtask );



	}
}

//==============================================================================
//
//==============================================================================
void RS_nop( CMD_MSG	*rt_task )
{
	Set_logInfo2("★RS_nop(S=%d,E=%d)",rt_task->state, rt_task->event);
//	SKprintf("RS_nop(%d,%d)\r\n", rt_task->state, rt_task->event);
}
//==============================================================================
//
//==============================================================================
void RScomand_send( CMD_MSG	*rt_task )
{
	RETURN_STATUS		status = RET_TRUE;
	TIMER_EVENT_FORM	*te_form;
	MESSAGE_QUE_DATA	*msg;
	uint8_t				timer_id;



//	Set_logInfo2("RScomand_send()");
	Set_logInfo2("★RScomand_send(S=%d,E=%d)",rt_task->state, rt_task->event);

//	SKprintf("RScomand_send(%d,%d)\r\n", rt_task->state, rt_task->event);

	status = Send_rx485_cmd_message( rt_task );

	if( status == RET_TRUE ){
		rt_task->state = RT_STATE_RESPONS_RECIVE;
		uart[SK_UART1_RS485].totalnum = 0;
		work_buf_num = 0;
		SKprintf("RScomand_send(%d,%d)\r\n", rt_task->state, rt_task->event);


		// タイムアウト　イベントをセット
#ifdef	 __HEAP_DBUG
		Set_logInfo2("HeapSize 008 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG

		msg = (MESSAGE_QUE_DATA *)pvPortMalloc(sizeof(MESSAGE_QUE_DATA));

#ifdef	 __HEAP_DBUG
		Set_logInfo2("pvPortMalloc=%x",msg);
		Set_logInfo2("HeapSize 008 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG

		msg->maroc_ptr = msg;
		msg->u.cmd_msg.event = RT_EVENT_TIMEOUT;

		te_form = (TIMER_EVENT_FORM *)pvPortMalloc(sizeof(TIMER_EVENT_FORM));

#ifdef	 __HEAP_DBUG
		Set_logInfo2("pvPortMalloc=%x",te_form);
#endif	//	 __HEAP_DBUG

		te_form->maroc_ptr = te_form;
		te_form->mail_form = msg;
		te_form->hmsg = GetMessageQue(SK_TASK_sub2);
		te_form->time = 50;		// 約500msec
		timer_id = GetTimerEventID();
		if( timer_id == 0xff ){
			SKprintf("TIMER EVENT NOT AVAILABLE\r\n");
		}
		te_form->timer_id = rt_task->timer_id = timer_id;


#ifdef	 __HEAP_DBUG
		SKprintf("tm_form=%p, msg=%p\r\n",te_form, msg);
		Set_logInfo2("HeapSize 008 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG

		status = osMessageQueuePut (GetMessageQue(SK_TASK_sub1), (void *)te_form->maroc_ptr, 0,0);
		if( status == osOK ){
			//Set_logInfo2("RScomand_send(). Send MsgQue OK");
		}
		else{
			Set_logInfo2("RScomand_send(). Send MsgQue ERROR");
		}
		SKprintf("RScomand_send() End \r\n");
	}
}
//==============================================================================
//
//==============================================================================
void RSrespons_recive( CMD_MSG	*rt_task )
{
	RETURN_STATUS		status = RET_TRUE;
	CMD_MSG				msg;

	Set_logInfo2("★RSrespons_recive(S=%d,E=%d)",rt_task->state, rt_task->event);

//	Set_logInfo2("RSrespons_recive()");
//	SKprintf("RSrespons_recive(%d,%d)\r\n", rt_task->state, rt_task->event);

	while( (uart[SK_UART1_RS485].Set_cnt - uart[SK_UART1_RS485].Get_cnt)   > 0 ){
		work_buf[work_buf_num ++] = Get_rcv_data(SK_UART1_RS485);

		if( Get_end_test_pt(work_buf_num, work_buf) != 0 ){
			SKprintf("Respons Recive\r\n");
			ReleaceTimerEvent(rt_task->timer_id);
			status = Set_Res_Message(work_buf_num, work_buf,Res_mesg);
			if( status == RET_TRUE ){
				rt_task->state = RT_STATE_RESPONS;
				rt_task->retry_num = 0;

				msg.event = RT_EVENT_RESPONS;

#ifdef	 __HEAP_DBUG
				Set_logInfo2("HeapSize 003 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG

				SendMsgQue(&msg);

#ifdef	 __HEAP_DBUG
				Set_logInfo2("HeapSize 003 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG
			}
			else{
				Set_logInfo2("RETRY:Recive data error");
				rt_task->state  = RT_STATE_READY;

				// 再送する
				msg.event = RT_EVENT_START_REQ;
				msg.address = rt_task->address ;
				msg.command = rt_task->command;
				msg.command_sub = rt_task->command_sub;
				msg.sub1 = rt_task->sub1;

#ifdef	 __HEAP_DBUG
				Set_logInfo2("HeapSize 004 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG

				SendMsgQue(&msg);

#ifdef	 __HEAP_DBUG
Set_logInfo2("HeapSize 004 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG


			}
			break;
		}
	}
}

//==============================================================================
//
//==============================================================================
void RSTimeout( CMD_MSG	*rt_task )
{
	CMD_MSG				msg;
	char	 	c[17];
	uint8_t		i,j,k;

	Set_logInfo2("★RSTimeout(S=%d,E=%d)",rt_task->state, rt_task->event);

	rt_task->state = RT_STATE_READY;
	Set_logInfo2("TIME OUT END");



	SKprintf("Get_cnt  =%d\r\n",uart[SK_UART1_RS485].Get_cnt);
	SKprintf("Set_cnt  =%d\r\n",uart[SK_UART1_RS485].Set_cnt);
	SKprintf("totalnum =%d\r\n",uart[SK_UART1_RS485].totalnum);
	SKprintf("rcvnum   =%d\r\n",uart[SK_UART1_RS485].rcvnum);
	SKprintf("rcv_wpt  =%d\r\n",uart[SK_UART1_RS485].rcv_wpt);
	SKprintf("rcv_rpt  =%d\r\n",uart[SK_UART1_RS485].rcv_rpt);

	c[16] = '\0';
	for(i=0; i< RCV_BUF_SIZE/16; i++ ){

		SKprintf(" [%03d] ", i*16 );

		for(j=0; j< 16; j++ ){
			k = uart[SK_UART1_RS485].rcvbuf[i*16+j];
			SKprintf(" %02x ", k );
			c[j] =  (uint8_t)((k<0x20||k>=0x7f)? '.': k);
		}
		SKprintf("  %s\r\n",c );
	}

	SKprintf("\r\nwork_buf_num =%d\r\n",work_buf_num);

	c[16] = '\0';
	for(i=0; i< RCV_BUF_SIZE/16; i++ ){

		SKprintf(" [%03d] ", i*16 );

		for(j=0; j< 16; j++ ){
			k = work_buf[i*16+j];
			SKprintf(" %02x ", k );
			c[j] =  (uint8_t)((k<0x20||k>=0x7f)? '.': k);
		}
		SKprintf("  %s\r\n",c );
	}

	if(rt_task->retry_num < 2){
		// 同じデバイスに再送する
		msg.event = RT_EVENT_START_REQ;
		msg.address = rt_task->address ;
		msg.command = rt_task->command;
		msg.command_sub = rt_task->command_sub;
		msg.sub1 = rt_task->sub1;

#ifdef	 __HEAP_DBUG
		Set_logInfo2("HeapSize 005 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG

		SendMsgQue(&msg);

#ifdef	 __HEAP_DBUG
		Set_logInfo2("HeapSize 005 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG
	}
	else{
		// デバイスを変更して再送する
		if( rt_task->address == RS485_AD_SLEVE01){
			msg.address = RS485_AD_SLEVE02;
		}
		else{
			msg.address = RS485_AD_SLEVE01;
		}
		msg.event = RT_EVENT_START_REQ;
		msg.command = rt_task->command;
		msg.command_sub = rt_task->command_sub;
		msg.sub1 = rt_task->sub1;

#ifdef	 __HEAP_DBUG
Set_logInfo2("HeapSize 006 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG

		SendMsgQue(&msg);

#ifdef	 __HEAP_DBUG
		Set_logInfo2("HeapSize 006 = 0x%x",xPortGetFreeHeapSize());
#endif	//	 __HEAP_DBUG
	}
}
//==============================================================================
//
//==============================================================================
void RSstop_req( CMD_MSG	*rt_task )
{
	Set_logInfo2("★RSstop_req(S=%d,E=%d)",rt_task->state, rt_task->event);
}


//==============================================================================
//
//==============================================================================
void RSrespons_proc( CMD_MSG	*rt_task )
{
	RETURN_STATUS		status = RET_TRUE;
	CMD_MSG				msg;
	uint16_t	dt16;
	float		dtf;
	uint8_t		*pt;
	uint8_t 	*msgQueBuf;


//	Set_logInfo2("RSrespons_proc(()");
	Set_logInfo2("★RSrespons_proc(S=%d,E=%d)",rt_task->state, rt_task->event);
//	SKprintf("RSrespons_proc(%d,%d)\r\n", );

	status = RET_TRUE;
	switch( Res_mesg[COM_COMMAND] ){
	case RS485_CMD_STATUS:
		SKprintf("RS485_CMD_STATUS\r\n");

		break;
	case RS485_CMD_VERSION:
		SKprintf("RS485_CMD_VERSION\r\n");

		dt16 = Res_mesg[COM_SLV_VERSION_H];
		dt16 <<= 8;
		dt16 |= Res_mesg[COM_SLV_VERSION_L];

		SKprintf("  SLV VER = %04x, ",dt16);

		dt16 = Res_mesg[COM_SNS_VERSION_H];
		dt16 <<= 8;
		dt16 |= Res_mesg[COM_SNS_VERSION_L];
		SKprintf("  SNS VER = %04x \r\n",dt16);

		break;
	case RS485_CMD_MESUR:
		SKprintf("RS485_CMD_MESUR\r\n");

		break;
	case RS485_CMD_MESUR_DATA:
		SKprintf("RS485_CMD_MESUR_DATA\r\n");

		pt = (uint8_t *)&dtf;
		pt[3] = Res_mesg[COM_MESUR_DATA_H];
		pt[2] = Res_mesg[COM_MESUR_DATA_MH];
		pt[1] = Res_mesg[COM_MESUR_DATA_ML];
		pt[0] = Res_mesg[COM_MESUR_DATA_L];


		if( rt_task->address == RS485_AD_SLEVE01){
			Set_logInfo2("SLAVE01 DATA = %fmm",dtf);
		}
		else{
			Set_logInfo2("SLAVE02 DATA = %fmm",dtf);
		}



		break;
	default:
		status = RET_FALSE;
		//SKprintf("ERROR Recive Command None \r\n");
		break;
	}

	if( status == RET_TRUE ){
		rt_task->state = RT_STATE_READY;
		cmd_ptr ++;

		Set_logInfo2("command_sub=%d",rt_task->command_sub);
		rt_task->command_sub --;

		if(rt_task->command_sub > 0 ){
			msg.event = RT_EVENT_START_REQ;

			if( rt_task->address == RS485_AD_SLEVE01){
				msg.address = RS485_AD_SLEVE02;
			}
			else{
				msg.address = RS485_AD_SLEVE01;
			}

			msg.command = rt_task->command;
			msg.command_sub = rt_task->command_sub;
			msg.sub1 = rt_task->sub1;

			SendMsgQue(&msg);

		}
		else{
			Set_logInfo2("END RSrespons_OK");
		}
	}


}



#ifdef ___NOP
		switch( cp_step ){
		case COM_PROTOCOL_SEND:
			//SKprintf("rs485_com_task(001) cmd_ptr=%d \r\n",cmd_ptr);

			if( com[cmd_ptr].command != RS485_CMD_MAX ){
				//SKprintf("rs485_com_task() 001\r\n");
				com_msg.command = com[cmd_ptr].command;
				com_msg.address = com[cmd_ptr].address;
				com_msg.sub1 = com[cmd_ptr].sub1;
			}
			else{
				//SKprintf("rs485_com_task() 002\r\n");
				cmd_ptr = 0;
				com_msg.command = com[cmd_ptr].command;
				com_msg.address = com[cmd_ptr].address;
				com_msg.sub1 = com[cmd_ptr].sub1;
			}

			status = Send_rx485_cmd_message( &com_msg );


			if( status == RET_TRUE ){
				cp_step = COM_PROTOCOL_RECIVE;
                num = 0;
			}
			break;

		case COM_PROTOCOL_RECIVE:

			while( rcvnum  > 0 ){
				work_buf[num ++] = Get_rcv_data();
				//SKprintf("Get_end_test_pt() num=%d, dt=%02x\r\n",num,work_buf[(num-1)]);

				if( Get_end_test_pt(num, work_buf) != 0 ){
					SKprintf("Respons Recive\r\n");
					status = Set_Res_Message(num, work_buf,Res_mesg);
					if( status == RET_TRUE ){
						cp_step = COM_PROTOCOL_RESPONS;
						com_step_flg = COM_RCV_INIT;
						res_ptr = 0;
					}
					else{
						cp_step = COM_PROTOCOL_SEND;
						SKprintf("COMMAND RESEND \r\n");
					}
					break;
				}
			}
			break;

		case COM_PROTOCOL_RESPONS:
			status = RET_TRUE;
			switch( Res_mesg[COM_COMMAND] ){
			case RS485_CMD_STATUS:
				SKprintf("RS485_CMD_STATUS\r\n");

				break;
			case RS485_CMD_VERSION:
				SKprintf("RS485_CMD_VERSION\r\n");

				dt16 = Res_mesg[COM_SLV_VERSION_H];
				dt16 <<= 8;
				dt16 |= Res_mesg[COM_SLV_VERSION_L];

				SKprintf("  SLV VER = %04x, ",dt16);

				dt16 = Res_mesg[COM_SNS_VERSION_H];
				dt16 <<= 8;
				dt16 |= Res_mesg[COM_SNS_VERSION_L];
				SKprintf("  SNS VER = %04x \r\n",dt16);

				break;
			case RS485_CMD_MESUR:
				SKprintf("RS485_CMD_MESUR\r\n");

				break;
			case RS485_CMD_MESUR_DATA:
				SKprintf("RS485_CMD_MESUR_DATA\r\n");

				pt = (uint8_t *)&dtf;
				pt[3] = Res_mesg[COM_MESUR_DATA_H];
				pt[2] = Res_mesg[COM_MESUR_DATA_MH];
				pt[1] = Res_mesg[COM_MESUR_DATA_ML];
				pt[0] = Res_mesg[COM_MESUR_DATA_L];

				my_putfloat(dtf, 3, msgQueBuf);
				SKprintf("MEASUR DATA =  %s\r\n",msgQueBuf);






				break;
			default:
				status = RET_FALSE;
				//SKprintf("ERROR Recive Command None \r\n");
				break;
			}

			if( status == RET_TRUE ){
				cp_step = COM_PROTOCOL_SEND;
				cmd_ptr ++;
				break;
			}

			break;

		default:
			break;

		}
	}
}
#endif

//==============================================================================
//
//==============================================================================
uint16_t  Get_end_test_pt(uint16_t num,uint8_t *buf )
{
	uint16_t	i;
	uint16_t	rtn;

	rtn = 0;

	for( i=0; i<num; i++){
		if( (i+TEXT_LENGTH) > num ){
			rtn = 0;
			//SKprintf("None\r\n");
			break;
		}
		else if( buf[i] == message_end_text[0]
				&& buf[i+1] == message_end_text[1]
				&& buf[i+2] == message_end_text[2]
				&& buf[i+3] == message_end_text[3]
				&& buf[i+4] == message_end_text[4]
				&& buf[i+5] == message_end_text[5] ){

			SKprintf("FIX\r\n");
			rtn = i;
			break;
		}
	}

	return rtn;
}
//==============================================================================
//
//==============================================================================
RETURN_STATUS  Set_Res_Message(uint16_t num, uint8_t *src, uint8_t *dist)
{
	RETURN_STATUS	status = RET_TRUE;
	uint16_t	i;
	uint16_t	j;
	uint16_t	start;
	uint16_t	end;
	uint8_t		c[2];

	//-------------------------------------------------------
	// Command buffer Clesr
	//-------------------------------------------------------
	for( i=0; i < COM_TABLE_MAX; i++ ){
		Res_mesg[i] = 0;
	}


	//-------------------------------------------------------------
	// スタートテキスト検索
	//-------------------------------------------------------------
	start = 0xffff;
	for( i=0; i<num; i++){
		if( src[i] == res_start_text[0]
				&& src[i+1] == res_start_text[1]
				&& src[i+2] == res_start_text[2]
				&& src[i+3] == res_start_text[3]
				&& src[i+4] == res_start_text[4]
				&& src[i+5] == res_start_text[5] ){

			start = i;
			break;
		}
	}

	if( start == 0xffff ){
		status = RET_FALSE;
		SKprintf("Error : Respons Data start txt none \r\n");
	}

	//-------------------------------------------------------------
	// エンドテキスト検索
	//-------------------------------------------------------------
	if( status == RET_TRUE ){
		end = 0xffff;
		for( i=0; i<num; i++){
			if( src[i] == message_end_text[0]
					&& src[i+1] == message_end_text[1]
					&& src[i+2] == message_end_text[2]
					&& src[i+3] == message_end_text[3]
					&& src[i+4] == message_end_text[4]
					&& src[i+5] == message_end_text[5] ){

				end = i+5+1;
				break;
			}
		}

		if( end == 0xffff ){
			status = RET_FALSE;
			SKprintf("Error : Respons Data end txt none \r\n");
		}
	}
	//-------------------------------------------------------------
	// Res[]に転送
	//-------------------------------------------------------------
	if( status == RET_TRUE ){
		j = 0;
		for( i=start; i < end; i++){
			if( j < COM_TABLE_MAX ){
				dist[j] = src[i];
			}
			else{
				status = RET_FALSE;
				SKprintf("Error : Respons data size over\r\n");
			}
			j++;
		}
	}
	//-------------------------------------------------------------
	// チェックサム確認
	//-------------------------------------------------------------
	if( status == RET_TRUE ){
		if( dist[COM_CHKSUM] !=Get_command_chksum(COM_COUNTER_L, COM_CHKSUM_ID, dist)){
			status = RET_FALSE;
			SKprintf("Error : Respons data chksum eoor");
		}
	}
	//-------------------------------------------------------------
	// レスポンスデータをログ表示
	//-------------------------------------------------------------
	for( i=0;  i < COM_TABLE_MAX ; i++ ){
		cmd_char[i] =  (uint8_t)((dist[i]<0x20||dist[i]>=0x7f)? '.': dist[i]);
	}

	SKprintf("\r\nRESPONS MESSAGE = \r\n ");
	for( i=0; i < COM_TABLE_MAX ; i++){
		SKprintf("%02x ", dist[i]);
	}
	SKprintf("\r\n ");
	c[1] = '\0';
	for( i=0; i < COM_TABLE_MAX ; i++){
		c[0] = cmd_char[i];
		SKprintf(" %s ", c);
	}
	SKprintf("\r\n");


	return status;

}
//==============================================================================
//
//=============================================================================
uint8_t Get_command_chksum(uint8_t start, uint8_t end,uint8_t *dt )
{
	uint8_t		sum = 0;
	uint8_t		i;

	for( i=start; i<end; i++){
		sum += dt[i];
		//SKprintf("%02x ", dt[i]);
	}
	//SKprintf(" sum=%02x\r\n",sum);

	return sum;
}

//==============================================================================
//
//==============================================================================

RETURN_STATUS Send_rx485_cmd_message( CMD_MSG	 *com_msg )
{
	RETURN_STATUS	status = RET_TRUE;
	uint8_t		i,j;
	uint8_t		c[2];
	uint8_t		*pt;

	//-------------------------------------------------------
	// Command buffer Clesr
	//-------------------------------------------------------
	for( i=0; i < COM_TABLE_MAX; i++ ){
		Cmd_mesg[i] = 0;
	}

	//-------------------------------------------------------
	// START Text
	//-------------------------------------------------------
	for( i=0; i < TEXT_LENGTH; i++ ){
		j = COM_START_TXT_00 + i ;
		Cmd_mesg[j] = com_start_text[i];
	}

	//-------------------------------------------------------
	// 通信カウンタ
	//-------------------------------------------------------
	com_counter ++;
	Cmd_mesg[COM_COUNTER_L] = (uint8_t)com_counter;
	Cmd_mesg[COM_COUNTER_H] = (uint8_t)(com_counter >> 8);

	//-------------------------------------------------------
	// アドレス
	//-------------------------------------------------------
	Cmd_mesg[COM_ADDRESS_ID] = '#';
	Cmd_mesg[COM_ADDRESS_DIST] = com_msg->address;
	Cmd_mesg[COM_ADDRESS_SORC] =  MY_RS485_ADDRESS;

	//-------------------------------------------------------
	// コマンド
	//-------------------------------------------------------
	Cmd_mesg[COM_COMMAND_ID] = '*';
	Cmd_mesg[COM_COMMAND] = com_msg->command;

	switch(com_msg->command){
	case RS485_CMD_STATUS:
		break;
	case RS485_CMD_VERSION:
		break;
	case RS485_CMD_MESUR:
		Cmd_mesg[COM_SENS_CTRL] = com_msg->sub1;
		break;
	case RS485_CMD_MESUR_DATA:
		Cmd_mesg[COM_DATA_KIND ] = com_msg->sub1;
		break;
	default:
		break;
	}

	//-------------------------------------------------------
	// チェックサム計算
	//-------------------------------------------------------
	Cmd_mesg[COM_CHKSUM_ID] = '$';
	Cmd_mesg[COM_CHKSUM] = Get_command_chksum(COM_COUNTER_L, COM_CHKSUM_ID, Cmd_mesg);

	//-------------------------------------------------------
	// END Text
	//-------------------------------------------------------
	for( i=0; i < TEXT_LENGTH; i++ ){
			j = COM_END_TXT_00 + i ;
			Cmd_mesg[j] = message_end_text[i];
	}

	//-------------------------------------------------------
	// デバック用ログ
	//-------------------------------------------------------

	for( i=0;  i < COM_TABLE_MAX; i++ ){
		cmd_char[i] =  (uint8_t)((Cmd_mesg[i]<0x20||Cmd_mesg[i]>=0x7f)? '.': Cmd_mesg[i]);
	}

	SKprintf("\r\nCOMMAND MESSAGE = \r\n ");
	for( i=0;  i < COM_TABLE_MAX; i++ ){
		SKprintf("%02x ", Cmd_mesg[i]);
	}
	SKprintf("\r\n ");
	c[1] = '\0';
	for( i=0;  i < COM_TABLE_MAX; i++ ){
		c[0] = cmd_char[i];
		SKprintf(" %s ", c);
	}
	SKprintf("\r\n");

	//-------------------------------------------------------
	// コマンド送信
	//-------------------------------------------------------
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RS485_TX);
	status = Send_rs485((uint8_t *)Cmd_mesg, COM_TABLE_MAX );
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RS485_RX);


	return status;

}





