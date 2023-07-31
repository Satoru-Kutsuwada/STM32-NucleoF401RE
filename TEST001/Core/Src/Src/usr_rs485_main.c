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
typedef struct{
	RA485_COMMAND		command;
	RA485_ADDRESS		address;
	uint8_t				sub1;
	uint8_t				rcv_byte;
} CMD_MSG;


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




#define RS485_MSG_MAX	20
uint8_t		cmd_mesg[RS485_MSG_MAX];
uint8_t		Res_mesg[RS485_MSG_MAX];
uint8_t		cmd_char[RS485_MSG_MAX];
uint8_t		cmd_ptr = 0;



const CMD_MSG	com[] = {
	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE01, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_01_BYTE	 },
	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE02, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_01_BYTE	 },
	{ RS485_CMD_VERSION, 	RS485_AD_SLEVE01, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_02_BYTE	 },
	{ RS485_CMD_VERSION, 	RS485_AD_SLEVE02, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_02_BYTE	 },
	{ RS485_CMD_MESUR, 		RS485_AD_SLEVE01, 	1,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_03_BYTE	 },
	{ RS485_CMD_MESUR, 		RS485_AD_SLEVE02, 	1,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_03_BYTE	 },
	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE01, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_01_BYTE	 },
	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE02, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_01_BYTE	 },
	{ RS485_CMD_MESUR_DATA, RS485_AD_SLEVE01, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_04_BYTE	 },
	{ RS485_CMD_MESUR_DATA, RS485_AD_SLEVE02, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_04_BYTE	 },
	{ RS485_CMD_MAX, 		0, 					0,	0 }
};



/* Private function prototypes -----------------------------------------------*/
//RETURN_STATUS Rcv_rx485_message(void)；
//RETURN_STATUS Send_rx485_cmd_message( CMD_MSG *com_msg );
RETURN_STATUS Rcv_rx485_message(void);

RETURN_STATUS Send_rx485_cmd_message( CMD_MSG	 *com_msg );

//==============================================================================
//
//==============================================================================
void rs485_com_task(void)
{
	RETURN_STATUS	status = RET_TRUE;
	CMD_MSG	com_msg;

	SKprintf("rs485_com_task() \r\n");
	if( com[cmd_ptr].command != RS485_CMD_MAX ){
		SKprintf("rs485_com_task() 001\r\n");
		com_msg.command = com[cmd_ptr].command;
		com_msg.address = com[cmd_ptr].address;
		com_msg.sub1 = com[cmd_ptr].sub1;
		com_msg.rcv_byte = com[cmd_ptr].rcv_byte;
	}
	else{
		SKprintf("rs485_com_task() 002\r\n");
		cmd_ptr = 0;
		com_msg.command = com[cmd_ptr].command;
		com_msg.address = com[cmd_ptr].address;
		com_msg.sub1 = com[cmd_ptr].sub1;
		com_msg.rcv_byte = com[cmd_ptr].rcv_byte;
	}

	status = Send_rx485_cmd_message( &com_msg );

	SKprintf("rs485_com_task() 003 s=%d\r\n", status);

	if( status == RET_TRUE ){

		while( Get_rs485_rcvflg() == 0 ){
			osDelay(1);
		}

		status = Rcv_rx485_message();
		Set_rs485_rcvflg(0);
	}

	if( status == RET_TRUE ){
		cmd_ptr ++;
	}

}


//==============================================================================
//
//==============================================================================
RETURN_STATUS Rcv_rx485_message(void)
{
	RETURN_STATUS	status = RET_TRUE;
	uint8_t		i   = 0;
	uint8_t		flg = 0;
	uint8_t		sum = 0;
	uint8_t		address;


	SKprintf("Respons  = ");

	// チェックサム確認
	while( cmd_mesg[i] != '$' ){
		sum += Res_mesg[i];

		// デバックメッセージ
		SKprintf("%02x ", Res_mesg[i]);
		cmd_char[i] =  (uint8_t)((Res_mesg[i]<0x20||Res_mesg[i]>=0x7f)? '.': Res_mesg[i]);

		i ++;
	}
	SKprintf("%02x ", Res_mesg[i]);
	i ++;
	SKprintf("%02x \r\n", Res_mesg[i]);


	if( Res_mesg[i] != sum ){
		status = RET_FALSE;
	}

	if( status == RET_TRUE ){

		// 送信元アドレス
		address = Res_mesg[RS485_ADD_ID + 2];


		if( Res_mesg[RS485_CMD_ID] == '*' && Res_mesg[RS485_CMD_ID+1] == RS485_CMD_STATUS){
			slv_dt[address].resp_dt = Res_mesg[RS485_CMD_ID+2];
			slv_dt[address].status = Res_mesg[RS485_CMD_ID+3];
			slv_dt[address].status_err = Res_mesg[RS485_CMD_ID+4];
			slv_dt[address].sens_kind = Res_mesg[RS485_CMD_ID+5];
		}

		else if( Res_mesg[RS485_CMD_ID] == '*' && Res_mesg[RS485_CMD_ID+1] == RS485_CMD_VERSION){
			slv_dt[address].resp_dt = Res_mesg[RS485_CMD_ID+2];
			slv_dt[address].slv_ver = (Res_mesg[RS485_CMD_ID+3] << 8) | Res_mesg[RS485_CMD_ID+4] ;
			slv_dt[address].slv_ver = (Res_mesg[RS485_CMD_ID+3] << 8) | Res_mesg[RS485_CMD_ID+4] ;
			slv_dt[address].sens_ver = (Res_mesg[RS485_CMD_ID+5] << 8) | Res_mesg[RS485_CMD_ID+6] ;
		}
		else if( Res_mesg[RS485_CMD_ID] == '*' && Res_mesg[RS485_CMD_ID+1] == RS485_CMD_MESUR){
			slv_dt[address].resp_dt = Res_mesg[RS485_CMD_ID+2];
		}
		else if( Res_mesg[RS485_CMD_ID] == '*' && Res_mesg[RS485_CMD_ID+1] == RS485_CMD_MESUR_DATA){
			slv_dt[address].resp_dt = Res_mesg[RS485_CMD_ID+2];
			slv_dt[address].sens_dt = (float)(Res_mesg[RS485_CMD_ID+3] | (Res_mesg[RS485_CMD_ID+4] << 8)| (Res_mesg[RS485_CMD_ID+5] << 16)| (Res_mesg[RS485_CMD_ID+6] << 24));
		}
		else{
			status = RET_FALSE;
		}

	}

	return status;

}


//==============================================================================
//
//==============================================================================

RETURN_STATUS Send_rx485_cmd_message( CMD_MSG	 *com_msg )
{
	RETURN_STATUS	status = RET_TRUE;
	uint8_t		i   = 0;
	uint8_t		sum = 0;


	cmd_mesg[RS485_ADD_ID] = '#';
	cmd_mesg[RS485_ADD_ID + 1] = com_msg->address;
	cmd_mesg[RS485_ADD_ID + 2] = RS485_AD_MASTER;

	switch(com_msg->command){
	case RS485_CMD_STATUS:
		cmd_mesg[RS485_CMD_ID] = '*';
		cmd_mesg[RS485_CMD_ID + 1] = com_msg->command;
		cmd_mesg[RS485_CMD_ID + 2] = '$';
		break;
	case RS485_CMD_VERSION:
		cmd_mesg[RS485_CMD_ID] = '*';
		cmd_mesg[RS485_CMD_ID + 1] = com_msg->command;
		cmd_mesg[RS485_CMD_ID + 2] = '$';
		break;
	case RS485_CMD_MESUR:
		cmd_mesg[RS485_CMD_ID] = '*';
		cmd_mesg[RS485_CMD_ID + 1] = com_msg->command;
		cmd_mesg[RS485_CMD_ID + 2] = com_msg->sub1;
		cmd_mesg[RS485_CMD_ID + 3] = '$';
		break;
	case RS485_CMD_MESUR_DATA:
		cmd_mesg[RS485_CMD_ID] = '*';
		cmd_mesg[RS485_CMD_ID + 1] = com_msg->command;
		cmd_mesg[RS485_CMD_ID + 2] = com_msg->sub1;
		cmd_mesg[RS485_CMD_ID + 3] = '$';
		break;
	default:
		SKprintf("ERROE RS485 COMMAND ERR");
		status = RET_FALSE;
		break;
	}

	// チェックサム計算
	if( status == RET_TRUE ){

		i = 0;
		sum = 0;

		SKprintf("Comand Send = ");
		while( cmd_mesg[i] != '$' ){
			sum += cmd_mesg[i];

			// デバックメッセージ
			SKprintf("%02x ", cmd_mesg[i]);
			cmd_char[i] =  (uint8_t)((cmd_mesg[i]<0x20||cmd_mesg[i]>=0x7f)? '.': cmd_mesg[i]);
			i ++;
		}
		// '$'
		SKprintf("%02x ", cmd_mesg[i]);
		cmd_char[i] =  (uint8_t)((cmd_mesg[i]<0x20||cmd_mesg[i]>=0x7f)? '.': cmd_mesg[i]);

		// CHKSUM
		i ++;
		cmd_mesg[i] = sum;
		cmd_char[i] =  (uint8_t)((cmd_mesg[i]<0x20||cmd_mesg[i]>=0x7f)? '.': cmd_mesg[i]);
		cmd_char[i+1] = '\0';

		// デバックメッセージ
		SKprintf("%02x  %s\r\n", cmd_mesg[i], cmd_char);


		// 受信準備
		status = Recive_rs485(Res_mesg, com_msg->rcv_byte);
	}


	if( status == RET_TRUE ){
		// コマンド送信
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RS485_TX);
		status = Send_rs485(cmd_mesg, i+1 );
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RS485_RX);
	}

	return status;

}
