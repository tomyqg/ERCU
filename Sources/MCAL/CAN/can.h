#ifndef __CAN_H__
#define __CAN_H__

#include "typedefs.h"
#include "derivative.h"

#define FC1_ID                  0x18FFF121
#define AQ1_ID                  0x18FFF221
#define SIG1_ID                 0x18FFF321
#define DATE_ID                 0x18FEE6EE
#define RCU_P_ID                0x18FFFE02
#define ENC8_DIAG_RESPONSE_ID   0x18DAFA01 
#define ERCU_DIAG_REQUEST_ID    0x18DA01FA

#define DIAG_ENABLE 0
#define DIAG_STATE  0

#if  DIAG_ENABLE > 0
#define MSG_DIAG_RX_ID    0x18DA01FA
#define MSG_DIAG_TX_ID    0x18DAFA01
#endif

/**
@brief    Extended identifiers.
           - (STD_ON)  - if at least one extended identifier is used.
           - (STD_OFF) - if no extended identifiers are used at all
          If no extended identifiers are used then the IDs and MASKs can be stored in uint16 rather than uint32.
*/
#define CAN_EXTENDEDID STD_ON

/**
typedef   Can_IdType
            Type for storing the Identifier Length Type: Normal /Extended.
             - used by "Can_MessageBufferConfigObjectType" structure.
@remarks   The driver does not distinguish between Extended and Mixed transmission modes.
           Extended transmission mode of operation behaves the same as Mixed mode.
*/
#if (CAN_EXTENDEDID == STD_ON)
typedef uint32_t Can_IdType;
#else 
typedef uint16_t Can_IdType;
#endif 














#define CAN_MSG_EMPTY     0x00
#define CAN_MSG_NEW_MASK  0x01
#define CAN_MSG_OVER_MASK 0x02
#define CAN_MSG_NEW_OVER  0x03
#define CAN_MSG_TIMEOUT   0x04
#define CAN_MSG_UNTIMEOUT_MASK 0xFB

#define CAN_MSG_CHECK(msg_index) (can_msgs_buf_array[msg_index].msg_st & CAN_MSG_NEW_OVER)
#define CAN_MSG_CELAR(msg_index) can_msgs_buf_array[msg_index].msg_st = 0
#define CAN_MSG_EMPTY_CHECK(msg_index) (can_msgs_buf_array[msg_index].msg_st & CAN_MSG_UNTIMEOUT_MASK) == CAN_MSG_EMPTY /*REQ IDs: TPMS_SW_BASE_0377*/
#define CAN_MSG_TIMEOUT_SET(msg_index) can_msgs_buf_array[msg_index].msg_st |= CAN_MSG_TIMEOUT
#define CAN_MSG_TIMEOUT_CLEAR(msg_index) can_msgs_buf_array[msg_index].msg_st &= CAN_MSG_UNTIMEOUT_MASK
#define CAN_MSG_TIMEOUT_CHECK(msg_index) (can_msgs_buf_array[msg_index].msg_st & CAN_MSG_TIMEOUT)

/*Hardware interface control*/
#define CAN_REG(_reg) CAN##_reg /*For CAN Module extend*/

/*Disable MSCAN*/
#define CAN_MODULE_STOP CAN_REG(CTL1_CANE) = 0

typedef enum __CAN_MSG_T__
{
    ENC8_RESPONSE,
    ERCU_REQUEST,      
    #if  DIAG_ENABLE > 0
      MSG_DIAG_RX_RECEIVE,
      MSG_DIAG_TX_SEND,
    #endif
    RCU_P_MSG_SEND,
    //CAN_MSG_UDS_RECEIVE, /*UDS接收*/
    FC1_MSG_RECEIVE,
    AQ1_MSG_RECEIVE,
    SIG1_MSG_RECEIVE,
    //DATE_MSG_RECEIVE,
    CAN_MSGS_COUNT,
}can_msg_t;

typedef struct __CAN_MSG_CONFIG_T__
{
    uint32_t id;
    bool     is_ext_id;
    bool     is_tx;
    void (*process_fun)(void);
}can_msg_config_t;

typedef struct __CAN_MSG_BUF_CFG_T__
{
    uint8_t msg_st;
    uint8_t dlc;
    uint8_t *pbuffer;
    uint8_t period; /*某一个报文周期等于其配置的period值乘以CAN_PeriodHandle()被调用的周期*/
                    //period如果为0，则不是周期性报文
    uint8_t tick;   
}can_msg_buf_cfg_t;

extern can_msg_buf_cfg_t can_msgs_buf_array[];
extern io_err_t CAN_Init(void);
extern io_err_t CAN_Tx(can_msg_t msg_index);
extern void CAN_TxCheck(void);
extern void CAN_Rx(void);
extern void CAN_PeriodHandle(BOOL tx_rx);







































#endif /*End of __CAN_H__*/

