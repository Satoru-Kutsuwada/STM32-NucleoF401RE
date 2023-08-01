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

#define  RCV_BUF_SIZE 128
uint8_t		rcv_dt[2];
uint8_t		rcvbuf[RCV_BUF_SIZE];
uint8_t		rcvnum = 0;
uint8_t		rcv_wpt = 0;
uint8_t		rcv_rpt = 0;


/* Private variables ---------------------------------------------------------*/

#define RS485_MSG_MAX	20
uint8_t		cmd_mesg[RS485_MSG_MAX];
uint8_t		Res_mesg[RS485_MSG_MAX];
uint8_t		cmd_char[RS485_MSG_MAX];
uint8_t		cmd_ptr = 0;
uint8_t		res_ptr = 0;


/* Private variables ---------------------------------------------------------*/

typedef struct{
	RA485_COMMAND		command;
	RA485_ADDRESS		address;
	uint8_t				sub1;
	uint8_t				rcv_byte;
} CMD_MSG;


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

/* Private variables ---------------------------------------------------------*/

typedef struct{
	uint8_t			address_id;
	uint8_t			command_id;
	uint8_t			chksum_id;
} COMMAND_FORM;

const COMMAND_FORM	com_form[] = {
		{ 0, 3, 9 },
		{ 0, 3, 10 },
		{ 0, 3, 6 },
		{ 0, 3, 10 },
};


typedef enum{
	COM_PROTOCOL_SEND,
	COM_PROTOCOL_RECIVE
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
//RETURN_STATUS Rcv_rx485_message(void)；
//RETURN_STATUS Send_rx485_cmd_message( CMD_MSG *com_msg );

RETURN_STATUS Send_rx485_cmd_message( CMD_MSG	 *com_msg );
UART_HandleTypeDef * Get_huart(void);
COM_STEP  Rcv_rx485_message(CMD_MSG	*com_msg);
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
//
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
	RETURN_STATUS	status = RET_TRUE;
	CMD_MSG	com_msg;

	 COM_PROTOCOL_STEP	cp_step = COM_PROTOCOL_SEND;

	while(1){

		SKprintf("rs485_com_task() cmd_ptr=%d \r\n",cmd_ptr);
		if( cp_step == COM_PROTOCOL_SEND ){

			if( com[cmd_ptr].command != RS485_CMD_MAX ){
				//SKprintf("rs485_com_task() 001\r\n");
				com_msg.command = com[cmd_ptr].command;
				com_msg.address = com[cmd_ptr].address;
				com_msg.sub1 = com[cmd_ptr].sub1;
				com_msg.rcv_byte = com[cmd_ptr].rcv_byte;
			}
			else{
				//SKprintf("rs485_com_task() 002\r\n");
				cmd_ptr = 0;
				com_msg.command = com[cmd_ptr].command;
				com_msg.address = com[cmd_ptr].address;
				com_msg.sub1 = com[cmd_ptr].sub1;
				com_msg.rcv_byte = com[cmd_ptr].rcv_byte;
			}

			status = Send_rx485_cmd_message( &com_msg );


			if( status == RET_TRUE ){
				cp_step = COM_PROTOCOL_RECIVE;
                com_step_flg = COM_RCV_INIT;
                //cmd_ptr = 0;
                res_ptr = 0;
			}

		}


		if( status == RET_TRUE ){
			while( rcvnum  > 0 ){
				if( Rcv_rx485_message( &com_msg ) == COM_RCV_COMPLITE){
					switch( Res_mesg[RS485_CMD_ID+1] ){
					case RS485_CMD_STATUS:
						printf("RS485_CMD_STATUS\r\n");


						break;
					case RS485_CMD_VERSION:
						printf("RS485_CMD_VERSION\r\n");


						break;
					case RS485_CMD_MESUR:
						printf("RS485_CMD_MESUR\r\n");

						break;
					case RS485_CMD_MESUR_DATA:
						printf("RS485_CMD_MESUR_DATA\r\n");

						break;
					default:
						break;
				   }
				}
			}
		}

		if( status == RET_TRUE ){
			cp_step = COM_PROTOCOL_SEND;
			cmd_ptr ++;
		}
	}

}


//==============================================================================
//
//==============================================================================
COM_STEP  Rcv_rx485_message(CMD_MSG	*com_msg)
{
	uint8_t		i;
	uint8_t		sum;
	uint8_t		dt;

		dt = Get_rcv_data();

		switch( com_step_flg ){
		case  COM_RCV_INIT:
			if( dt == '#'  &&  res_ptr== com_form[com_msg->command].address_id ){
				Res_mesg[res_ptr] = dt;
				com_step_flg = COM_RCV_ADD_ID;
				res_ptr++;
			}
			break;
		case COM_RCV_ADD_ID:
			Res_mesg[res_ptr] = dt;
			com_step_flg = COM_RCV_ADD_ID_DIST;
			res_ptr++;
			break;

		case COM_RCV_ADD_ID_DIST:
			Res_mesg[res_ptr] = dt;
			com_step_flg = COM_RCV_ADD_ID_SOURCE;
			res_ptr++;
			break;

		case COM_RCV_ADD_ID_SOURCE:
			if( dt == '*'   &&  res_ptr== com_form[com_msg->command].command_id ){
				Res_mesg[res_ptr] = dt;
				com_step_flg = COM_RCV_CMD_ID;
				res_ptr++;
			}
			else{
				com_step_flg = COM_RCV_INIT;
				res_ptr = 0;
			}
			break;

		case COM_RCV_CMD_ID:
			Res_mesg[res_ptr] = dt;
			com_step_flg = COM_RCV_COMMAND;
			res_ptr ++;
			break;

		case COM_RCV_COMMAND:
			if( dt == '$'   &&  res_ptr == com_form[com_msg->command].chksum_id ){
				Res_mesg[res_ptr] = dt;
				com_step_flg = COM_RCV_CSUM_ID;
				res_ptr ++;
			}
			else{
				Res_mesg[res_ptr] = dt;
				com_step_flg = COM_RCV_COMMAND;
				res_ptr ++;
			}
			break;
		case COM_RCV_CSUM_ID:


			sum = 0;
			for( i = 0; i< (res_ptr -1); i++){
				sum += Res_mesg[i];
			}

			if( sum == dt ){
				Res_mesg[res_ptr] = dt;
				com_step_flg = COM_RCV_COMPLITE;
				res_ptr ++;


				printf("Res_mesg= ");
				for( i=0; i<res_ptr; i++ ){
					printf("%02x ",Res_mesg[i]);
					cmd_char[i] = ((Res_mesg[i]<0x20||Res_mesg[i]>=0x7f)? '.': cmd_mesg[i]);
				}
				cmd_char[i+1] = '\0';
				printf(" :: %s\r\n", cmd_char);


			}
			else{
				com_step_flg = COM_RCV_INIT;
				res_ptr = 0;
			}

		case COM_RCV_COMPLITE:
		default:
			break;
	}

        printf("dt=0x%02x, com_step_flg=%d,rcvnum=%d\r\n", dt, com_step_flg,rcvnum);

        return  com_step_flg;
}


//==============================================================================
//
//==============================================================================

RETURN_STATUS Send_rx485_cmd_message( CMD_MSG	 *com_msg )
{
	RETURN_STATUS	status = RET_TRUE;
	uint8_t		i   = 0;
	uint8_t		sum = 0;
	uint8_t		num = 0;



	switch(com_msg->command){
	case RS485_CMD_STATUS:
		cmd_mesg[num++] = '#';
		cmd_mesg[num++] = com_msg->address;
		cmd_mesg[num++] = RS485_AD_MASTER;

		cmd_mesg[num++] = '*';
		cmd_mesg[num++] = com_msg->command;
		break;
	case RS485_CMD_VERSION:
		cmd_mesg[num++] = '#';
		cmd_mesg[num++] = com_msg->address;
		cmd_mesg[num++] = RS485_AD_MASTER;

		cmd_mesg[num++] = '*';
		cmd_mesg[num++] = com_msg->command;
		cmd_mesg[num++] = '$';
		break;
	case RS485_CMD_MESUR:
		cmd_mesg[num++] = '#';
		cmd_mesg[num++] = com_msg->address;
		cmd_mesg[num++] = RS485_AD_MASTER;

		cmd_mesg[num++] = '*';
		cmd_mesg[num++] = com_msg->command;
		cmd_mesg[num++] = com_msg->sub1;
		cmd_mesg[num++] = '$';
		break;
	case RS485_CMD_MESUR_DATA:
		cmd_mesg[num++] = '#';
		cmd_mesg[num++] = com_msg->address;
		cmd_mesg[num++] = RS485_AD_MASTER;

		cmd_mesg[num++] = '*';
		cmd_mesg[num++] = com_msg->command;
		cmd_mesg[num++] = com_msg->sub1;
		cmd_mesg[num++] = '$';
		break;
	default:
		SKprintf("ERROE RS485 COMMAND ERR\r\n");
		status = RET_FALSE;
		break;
	}

	// チェックサム計算
	if( status == RET_TRUE ){

		i = 0;
		sum = 0;

		for( i=0;  i < num; i++ ){
			sum += cmd_mesg[i];
		}
		cmd_mesg[num++] = '$';
		cmd_mesg[num++] = sum;


		SKprintf("Comand Send = ");
		for( i=0;  i < num; i++ ){
			SKprintf("%02x ", cmd_mesg[i]);
			cmd_char[i] =  (uint8_t)((cmd_mesg[i]<0x20||cmd_mesg[i]>=0x7f)? '.': cmd_mesg[i]);
		}
		SKprintf(" :: %s\r\n", cmd_char);


		// 受信準備
		//status = Recive_rs485(Res_mesg, com_msg->rcv_byte);
	}


	if( status == RET_TRUE ){
		// コマンド送信
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RS485_TX);
		status = Send_rs485(cmd_mesg, num );
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RS485_RX);
	}

	return status;

}
