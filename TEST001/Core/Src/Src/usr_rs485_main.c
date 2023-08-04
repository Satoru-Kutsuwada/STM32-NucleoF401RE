/*
 * usr_rs485_main.c
 *
 *  Created on: 2023/07/31
 *      Author: nosak
 */


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usr_system.h"
#include "rs485_com.h"

/* Public includes -----------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

/* Public define -------------------------------------------------------------*/

/* Public macro --------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/

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
#define  	RCV_BUF_SIZE 	128
uint8_t		rcv_dt[2];
uint8_t		rcvbuf[RCV_BUF_SIZE];
uint8_t		work_buf[RCV_BUF_SIZE];
uint8_t		rcvnum = 0;
uint8_t		rcv_wpt = 0;
uint8_t		rcv_rpt = 0;



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



/* Private variables ---------------------------------------------------------*/
typedef struct{
	RA485_COMMAND		command;
	RA485_ADDRESS		address;
	uint8_t				sub1;
} CMD_MSG;


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

/* Private function prototypes -----------------------------------------------*/
//RETURN_STATUS Send_rx485_cmd_message( CMD_MSG *com_msg );

RETURN_STATUS Send_rx485_cmd_message( CMD_MSG	 *com_msg );
UART_HandleTypeDef * Get_huart(void);

uint16_t  Get_end_test_pt(uint16_t num,uint8_t *buf );
RETURN_STATUS  Set_Res_Message(uint16_t num, uint8_t *src, uint8_t *dist);
uint8_t Get_rcv_data(void);
uint8_t Get_command_chksum(uint8_t start, uint8_t end,uint8_t *dt );


//==============================================================================
//
//==============================================================================

uint8_t *my_putint(int num, uint8_t *buf) {
	SKprintf("num=%d,num/10=%d,num%%10=%d\r\n",num,num/10,num%10);

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

void my_putfloat(float num, int precision, uint8_t *buf) {
	int dt;
	float fracPart,dtf;
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

}

//==============================================================================
//
//==============================================================================
void  tasuk3_init(void)
{
	uint8_t	mybuf[30];
    float   dtf;
    uint8_t dt8[10];
    uint8_t *pt;
    float   *ptf;

    SKprintf("tasuk3_init(void)\r\n");
	 dtf = 3.14;
	 my_putfloat(dtf, 3, mybuf);
	 SKprintf("dtf= %s\r\n",mybuf);

	 dtf = -543.14;
	 my_putfloat(dtf, 2, mybuf);
	 SKprintf("dtf= %s\r\n",mybuf);


	 pt = (uint8_t *)&dtf;
	    dt8[0]=*pt;
	    pt++;
	    dt8[1]=*pt;
	    pt++;
	    dt8[2]=*pt;
	    pt++;
	    dt8[3]=*pt;

	    SKprintf("dt8[]= %02x %02x %02x %02x \r\n",dt8[0],dt8[1],dt8[2],dt8[3]);

	    ptf=(float *)dt8;
		 my_putfloat(*ptf, 3, mybuf);
		 SKprintf("*ptf=%s",mybuf);

		 SKprintf("");

		 SKprintf("char=%d\r\n",sizeof(char));
		 SKprintf("short=%d\r\n",sizeof(short));
		 SKprintf("int=%d\r\n",sizeof(int));
		 SKprintf("long=%d\r\n",sizeof(long));
		 SKprintf("float=%d\r\n",sizeof(float));
		 SKprintf("double=%d\r\n",sizeof(double));



}


//==============================================================================
//
//==============================================================================
void Recive_rs485_prepaer(void)
{
	HAL_StatusTypeDef s;

		// 受信準備
		s= HAL_UART_Receive_IT(Get_huart(), rcv_dt, 1);

		switch(s){
		case HAL_OK:
			break;
		case HAL_ERROR:
		case HAL_BUSY:
		case HAL_TIMEOUT:
			// SKprintf("ERROR %s RS485 RECIVE = %d\r\n",UartList[SK_UART1_RS485].name, s);
			break;
		}
}

//==============================================================================
//
//==============================================================================
void Set_rcv_data(void)
{
    rcvbuf[rcv_wpt] = rcv_dt[0];
    rcvnum ++;
    rcv_wpt ++ ;
    if( rcv_wpt > RCV_BUF_SIZE ){
        rcv_wpt = 0;
    }
}

//==============================================================================
//　受信データ割込みコールバック
//==============================================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	Set_rcv_data();
	Recive_rs485_prepaer();
}


//==============================================================================
//
//==============================================================================
uint8_t Get_rcv_data(void)
{
    uint8_t dt;
    rcvnum --;
    dt =  rcvbuf[rcv_rpt];
    rcv_rpt ++ ;
    if( rcv_rpt > RCV_BUF_SIZE ){
        rcv_rpt = 0;
    }
    return dt;

}
//==============================================================================
//
//==============================================================================
void rs485_com_task(void)
{
		RETURN_STATUS		status = RET_TRUE;
		CMD_MSG				com_msg;
		COM_PROTOCOL_STEP	cp_step = COM_PROTOCOL_SEND;
		uint8_t 			num = 0;
//		uint8_t 	dt;
		float		dtf;
//		float		dtf2;
		uint8_t		*pt;
//		uint8_t		*pt2;
		uint16_t	dt16;
		uint8_t 	mybuf[10];


	while(1){

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

				my_putfloat(dtf, 3, mybuf);
				SKprintf("MEASUR DATA =  %s\r\n",mybuf);






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





