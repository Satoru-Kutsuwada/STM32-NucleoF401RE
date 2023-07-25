/*
 * usr_debug_tool.c
 *
 *  Created on: Jul 21, 2023
 *      Author: nosak
 */


/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usr_system.h"

#include <stdio.h>

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
int input_pos;


//-----------------------------------------
//
//-----------------------------------------
typedef enum{
    INPUT_INIT,
    INPUT_SAVING,
    INPUT_DATA_FIX

} INPUT_CHAR_STEP;

INPUT_CHAR_STEP input_char_step = INPUT_INIT;

//-----------------------------------------
//
//-----------------------------------------
#define INPUT_BUF_SIZE 30

typedef struct{
char main[INPUT_BUF_SIZE];
char *sub_ptr[INPUT_BUF_SIZE/2];
uint16_t data[INPUT_BUF_SIZE/2];

uint16_t data1;
uint16_t data2;
}INPUT_STRING;

INPUT_STRING input_string;


//-----------------------------------------
//
//-----------------------------------------
 typedef enum{
    CMD_RTC,
    CMD_LOG,
    CMD_RS485,


    CMD_MAX
 }COMMAND_MENUE;

#define MAX_COM_LENGTH 10
 typedef struct{
     COMMAND_MENUE pt;
     char command[MAX_COM_LENGTH];
 } COMAND_LIST;

 const COMAND_LIST com_list[] = {
     { CMD_RTC,      "rtc"  },
     { CMD_LOG,      "log" },
     { CMD_RS485,    "rs485" },



     { CMD_MAX, "none" }
 };

 //-----------------------------------------
 //
 //-----------------------------------------
 typedef enum{
     DEB_PROMPT_MODE = 0,
     DEB_LOG_MENUE,
     DEB_RS485_MENUE,

     DEB_DISP_MAX
 } DEB_MENUE_TYPE;

 DEB_MENUE_TYPE dev_menue_type = DEB_PROMPT_MODE;


 //-----------------------------------------
 //
 //-----------------------------------------
#define MAX_LENGTH 30
typedef struct{
   char    name[MAX_LENGTH];
} MENUE;

const MENUE Deb_menue00[] = {
    "\r\nLOG MENUE",
    " 1.LOG DISPLAY",
    " 2.LOG CLEAR",
    " 3.STOP MODE:IMMMEDIATE",
    " 4.STOP MODE:MAX_DATA_STOP",
    " 5.STOP MODE:NON_STOP",

    " r.EXIT"
};


const MENUE Deb_menue01[] = {
    "\r\nRS485 MENUE",
    " 1.RX MODE",
    " 2.TX MODE",

    " r.EXIT"
};

typedef struct
{
   MENUE *pt;
   uint8_t clumn;
} MENUE_NUM_PAGE;

const MENUE_NUM_PAGE MenueList[]={
   Deb_menue00, (uint8_t)(sizeof(Deb_menue00 )/sizeof(MENUE)),
   Deb_menue01, (uint8_t)(sizeof(Deb_menue01 )/sizeof(MENUE))
};


 /* Private function prototypes -----------------------------------------------*/
INPUT_CHAR_STEP read_line_streem(void);
COMMAND_MENUE input2menu(void);

//=============================================================================
//
//=============================================================================
void debu_main(void)
{
	//char ch;

	if(read_line_streem() == INPUT_DATA_FIX){

		input_char_step = INPUT_INIT;

		SKprintf("debu_main:001\r\n");

		switch(dev_menue_type){
		case DEB_PROMPT_MODE:
			DBmanue_prompt();
			break;
		case DEB_LOG_MENUE:
			DBmanue_log();
			break;
		case DEB_RS485_MENUE:
			DBmanue_rs485();
			break;
		default:
			break;
		}

		SKprintf("debu_main:002\r\n");
        // メニュを表示する
        DispMenue(dev_menue_type);
        SKprintf("debu_main:003\r\n");

	}
}
//==============================================================================
//
//==============================================================================
void DispMenue(uint8_t type)
{
    uint8_t i;
    if( type==DEB_PROMPT_MODE ){
        SKprintf("ST> ");
    }
    else{
        for( i=0; i<MenueList[type-1].clumn; i++){
            SKprintf("%s\r\n", &MenueList[type-1].pt[i].name[0]);
        }
    }
}
//=============================================================================
//
//=============================================================================
void DBmanue_prompt(void)
{
    switch( input2menu() ){
    case CMD_RTC:
    	rtc_display();
        break;
    case CMD_LOG:
        dev_menue_type = DEB_LOG_MENUE;
        break;
    case CMD_RS485:
        dev_menue_type = DEB_RS485_MENUE;
        break;
    default:
        break;
    }
}

//=============================================================================
//
//=============================================================================
void DBmanue_log(void)
{
	switch( input_string.main[0] ){
	case '1':
		LogInfo_display();
		break;
	case '2':
		LogInfo_clear();
		break;
	case '3':
		Set_logflg(LF_NON_STOP);
		break;
	case '4':
		Set_logflg(LF_IMMMEDIATE_STOP);
		break;
	case '5':
		Set_logflg(LF_MAX_DATA_STOP);
		break;
	case 'r':
	case 'R':
		dev_menue_type = DEB_PROMPT_MODE;
	default:
		break;
	}
}

//=============================================================================
//
//=============================================================================
void DBmanue_rs485(void)
{
	switch( input_string.main[0] ){
	case '1':
		break;
	case '2':
		break;
	case '3':
		break;
	case '4':
		break;
	case '5':
		break;
	case 'r':
	case 'R':
		dev_menue_type = DEB_PROMPT_MODE;
	default:
		break;
	}
}


//==============================================================================
//
//==============================================================================

COMMAND_MENUE input2menu(void)
{
     uint16_t i = 0;
    COMMAND_MENUE rtn = CMD_MAX;

        while( com_list[i].pt != CMD_MAX ){
            if (strcmp(&input_string.main[0], &com_list[i].command[0]) == 0){
                rtn = com_list[i].pt;
                break;
            }
            i++;
        }

    return rtn;
}
//==============================================================================
//
//==============================================================================
void command_split(void)
{
	int		i;
	int		j;
	char	moji = 0;
	char 	*ptr;

	for( j = 0; j<INPUT_BUF_SIZE/2; j++ ){
		input_string.sub_ptr[j] = 0;
	}

	for( i= 0; i<INPUT_BUF_SIZE; i++ ){
		switch(input_string.main[i]){
		case ' ':		// スペース
		case ',':		//　カンマ
		case '	':		//　タブ
			input_string.main[i] = '\0';
			moji = 0;
			break;
		default:
			if( moji == 0 ){
				if( j < INPUT_BUF_SIZE/2){
					input_string.sub_ptr[j] = &input_string.main[i];
				}
				moji = 1;
			}
			break;
		}
	}

	for( j = 1; j<INPUT_BUF_SIZE/2; j++ ){
		ptr = input_string.sub_ptr[j];
		moji = 1;
		if( ptr != 0 ){
			if ( ptr[0] == '0' && ptr[1] == 'x'){
				ptr ++;
				ptr ++;
				while( *ptr != '\0' ){
					if( ('0'<=*ptr && *ptr<='9') || ('A'<=*ptr && *ptr<='F') || ('a'<=*ptr && *ptr<='f')){

					}
					else{
						moji = 0;
					}
					ptr ++;
				}

				if( moji == 1 ){
					sscanf(input_string.sub_ptr[j], "%x", &input_string.data[j]);
				}
			}
			else{
				while( *ptr != '\0' ){
					if( '0'<=*ptr && *ptr<='9'){
					}
					else{
						moji = 0;
					}
					ptr ++;
				}

				if( moji == 1 ){
					sscanf(input_string.sub_ptr[j], "%d", &input_string.data[j]);
				}
			}

		}
		else{
			break;
		}
	}

}

//==============================================================================
//
//==============================================================================
 INPUT_CHAR_STEP read_line_streem(void)
{
    char c;

#ifdef ___NOP
    uint16_t i;
    uint16_t j;
    uint16_t keta;
    uint8_t num;
    uint8_t sub_cnt;
#endif	// ___NOP
    char	string[2];

    string[0] = '\0';
    string[1] = '\0';


    c = (char)getch();

    if( c != 0 ){

        switch(input_char_step){
            case INPUT_INIT:
                input_pos = 0;
                input_char_step = INPUT_SAVING;


            case INPUT_SAVING:
                if( isprint(c) && (input_pos <= INPUT_BUF_SIZE - 2)){
                    input_string.main[input_pos] = c;
                    input_pos ++;
                    string[0] = c;
                    SKprintf("%s",string);
                }
                else if (c == 0x08 && input_pos > 0) {      // Back Space
                    input_pos --;
                    SKprintf("\x08 \x08");
                }
                else if (c == '\r') {
                    input_string.main[input_pos] = '\0';
                    SKprintf("\r\n");
                    input_char_step = INPUT_DATA_FIX;

                    command_split();
                }
            default:
                break;
        }
    }
    return(input_char_step);
 }
