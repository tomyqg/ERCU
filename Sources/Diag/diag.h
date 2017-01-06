/***************************************************************************
2014-10-21, xyl, adapt for ERCU

****************************************************************************/

#ifndef _DIAG_H_
#define _DIAG_H_

#include "diagCfg.h"
#include "Platform_Types.h"
#include "can.h"
#include "os_cfg.h"




/* -- Define SID of Kwp2000 -- */
#define SID_StartDiagnosticSession          0x10 
#define LID_SDS_DefaultDiagMode        		0x81
#define LID_SDS_EndOfLineProgrammingMode    0x83
#define LID_SDS_ProgrammingMode      		0x85
#define LID_SDS_DevelopmentMode        		0x86

#define SID_EcuReset                        0x11    
#define SID_ClearFaultMemory	            0x14 
#define SID_ReadStatusOfDTC              	0x17

#define SID_ReadDTC              			0x18
#define LID_RDTC_ActiveDTC        			0x01
#define LID_RDTC_InactiveDTC        		0x02
#define LID_RDTC_AllDTC        				0x03


#define SID_ReadECUId			            0x1A 
#define SID_ReadDataByLocalId			    0x21  /* 21 A0 */

#define SID_ReadMemoryByAddress             0x23
#define SID_SecurityAccess                  0x27  /* 27 01, 27 02 */
#define LID_SA_RequestSeed     				0x01
#define LID_SA_SendKey        				0x02

#define SID_StartRoutineByLocalId           0x31
#define SID_StopRoutineByLocalId           	0x32
#define SID_ReqRoutineResultByLocalId      	0x33
#define LID_SR_GearCalculation 				0x01
#define LID_SR_ClutchCalculation 			0x02
#define LID_SR_ClutchTest 					0x03
#define LID_SR_GearTest 					0x04

#define SID_TesterPresent                   0x3E

/* -- Define Negtive Response Code -- */
#define NEG_ACK_NONE                              0x00
#define NEG_ACK_GENERALREJECT                     0x10
#define NEG_ACK_SERVICENOTSUPPORTED               0x11
#define NEG_ACK_SUBFUNCTIONNOTSUPPORTED           0x12
#define NEG_ACK_BUSYREPEATREQUEST                 0x21
#define NEG_ACK_CONDITIONSNOTCORRECT              0x22
#define NEG_ACK_REQUESTSEQUENCEERROR              0x24
#define NEG_ACK_ROUTINENOTCOMPLETED               0x23
#define NEG_ACK_REQUESTOUTOFRANGE                 0x31
#define NEG_ACK_ACCESSDENIED                      0x33
#define NEG_ACK_INVALIDKEY                        0x35
#define NEG_ACK_EXCEEDNUMOFATTEMPTS               0x36
#define NEG_ACK_REQUIREDTIMEDELAYNOTEXPIRED       0x37
#define NEG_ACK_UPLOADDOWNLOADNOTACCEPTED         0x40
#define NEG_ACK_RESPONDPENDING                    0x78
#define NEG_ACK_SERVICENOTSUPPORTEDINACTIVEMODE   0x80


//#define MSG_DIAG_RX_INDEX 0
//#define MSG_DIAG_TX_INDEX 0

#define  DIAG_RX_BUFF_SIZE 	128
#define  DIAG_TX_BUFF_SIZE 	128

#define  DIAG_STATE_INACTIVE 	0
#define  DIAG_STATE_ACTIVE 		1

#define  DIAG_P2_TIMEOUT_COUNT 	(5*OS_TICKS_PER_SEC)
#define  DIAG_P3_TIMEOUT_COUNT 	(5*DIAG_P2_TIMEOUT_COUNT)



/* -- Define Frame Types -- */
#define kSingleFrame                0x00
#define kFirstFrame                 0x10
#define kConsecutiveFrame           0x20
#define kFlowControlFrame           0x30
#define kFlowStatus_Overrun         0x32
#define kErrorFrame                 0x40

/* -- Define Flow State -- */
#define kFlowStateCTS                  0x00     /* CTS=Clear to send */
#define kFlowStateWait                 0x01
#define kFlowStateOverrun              0x02

/* -- Define Receive Flow State -- */
#define kRxState_Idle					0x00
#define kRxState_ApplInformed			0x01
#define kRxState_CanFrameReceived		0x02
#define kRxState_WaitCF					0x03
#define kRxState_TxFC					0x04
#define kRxState_Error					0xff

/* -- Define Transmit Flow State -- */
#define kTxState_Idle					0x00
#define kTxState_WaitFC     			0x01
#define kTxState_WaitTxCF				0x02
#define kTxState_ReqTx					0x03
#define kTxState_Error					0xff












typedef struct _TX_tag 
{
  vuint32_t id;         //4  
  uint32_t  *data;      //8   
  vuint8_t  dl;   		//9
  vuint8_t  state;      //11    
  //vuint32_t rate;    	//16
  vuint32_t timer;      //20 
  //FuncType encode_func; //24
  //FuncType tx_err_func; //28
}TX_tag_type ;				//size = 28 bytes


typedef struct _RX_tag
{
  vuint32_t id;         //4  
  uint32_t  *data;      //8   
  vuint8_t  dl;   		//9
  vuint8_t  state;      //11    
  //vuint32_t rate;    	//16
  vuint32_t timer;      //20 
  //FuncType decode_func; //24
  //FuncType rx_timeout_func; //28 
}RX_tag_type;				//size = 28 bytes


















struct RX_TP_tag 
{ 
  uint32 timer;    
  vuint8_t *rx_buffer;
  uint16 data_length;
  vuint16_t index;
  vuint8_t 	sn;
  vuint8_t 	st;
  vuint8_t  target_address;
  vuint8_t  source_address;
};

struct TX_TP_tag 
{ 
  vuint32_t timer;    
  vuint8_t *tx_buffer;
  vuint16_t data_length;
  vuint16_t index;
  vuint8_t 	sn;
  vuint8_t 	st;
  vuint8_t  target_address;
  vuint8_t  source_address;
};

struct DIAG_TX_tag { vuint8_t data[8]; };
struct DIAG_RX_tag { vuint8_t data[8]; };

extern vuint8_t diag_rx_buff[DIAG_RX_BUFF_SIZE];
extern vuint8_t diag_tx_buff[DIAG_TX_BUFF_SIZE];

extern volatile struct RX_TP_tag rx_state;
extern volatile struct TX_TP_tag tx_state;

extern volatile struct DIAG_RX_tag 	gMSG_DIAG_RX;
extern volatile struct DIAG_TX_tag 	gMSG_DIAG_TX;

#define CAN0_RX_MSG_NUM  1
#define CAN0_TX_MSG_NUM  1
extern volatile RX_tag_type RxMsgBuffA[CAN0_RX_MSG_NUM];
extern volatile TX_tag_type TxMsgBuffA[CAN0_TX_MSG_NUM];



extern void MsgDIAG_RX_Decode(void);



extern void Diag_InitTP(void);
extern void Diag_State(void);

















#endif

