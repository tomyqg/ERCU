#ifndef __CAN_H_APP__
#define __CAN_H_APP__


#include "includes.h"
#include "can.h"
#include "gprs.h"



//未使用位采用0x00填充

typedef struct _FC1_tag
{
  uint32_t DSM_m_st_GlobalIndex0;  /* ENC8故障全局指示0 */ //为1的位表示相应的故障
  uint32_t DSM_m_st_GlobalIndex1;  /* ENC8故障全局指示1 */ //为1的位表示相应的故障
}FC1_MsgType;

typedef struct _AQ1_tag
{
  uint32_t TR_m_q_DieselTotal;     /* 燃油消耗累积量 */
  uint32_t TR_m_q_LNGTotal;        /* 天然气消耗累积量 */
}AQ1_MsgType;

typedef struct _SIG1_tag
{
  //High bytes
  uint16_t SID_m_p_CNGPrs;        /* 天然气压力 */
  uint8_t SID_m_t_CNGTemp;        /* 天然气温度 */
  
  //Lowest bit
  byte T15_SWT         :1;        /* 点火开关 */          //0：T15没有上电
                                                          //1：T15上电
                                                      
  byte MODE_SWT        :1;        /* LNG模式开关 */       //0：请求纯油模式
                                                          //1：请求双燃料模式
                                                         
  byte CUTV_ST         :1;        /* 切断阀驱动状态 */    //0: 切断阀OFF
                                                          //1: 切断阀ON
                                                        
  byte NG_REL          :1;        /* 系统当前工作模式 */  //0：系统在纯油下工作
                                                          //1：系统在双燃料模式下工作
 
  //High 4 bits                                                         
  byte EPS_ST          :4;        /* 发动机同步状态 */    //0：无信号
                                                          //1：已同步
                                                          //2：不同步
                                                          //3：正在同步中   
  uint32_t TC_m_ti_NGET;          /* 天然气喷射时长 */
}SIG1_MsgType;  



typedef struct _Date_tag
{
  uint8_t TD_LocalHourOffset;    
  uint8_t TD_LocalMinOffset; 
  uint8_t TD_Year;
  uint8_t TD_Day;
  uint8_t TD_Month;
  uint8_t TD_Hour;
  uint8_t TD_Min;
  uint8_t TD_Sec;  
}Date_MsgType;


//需要下电保存，上电读取
typedef struct _RCU_P_tag
{
  uint32_t ERCU_m_ct_Running;  //ERCU累计运行时长，单位为秒
  uint32_t ERCU_m_ct_GPRSLastConnection;  //自上次收到主机通讯服务请求以来，ERCU累计运行时长，单位为秒
}RCU_P_MsgType;




/*CAN Messages Buffer Temp Declare*/
extern uint8_t ENC8RespRxBuff[8];
extern uint8_t ERCUReqTxBuff[8];
extern uint8_t FC1RxBuff[8] ;
extern uint8_t AQ1RxBuff[8];
extern uint8_t SIG1RxBuff[8];
extern uint8_t DateRxBuff[8];
extern uint8_t RCUPresentMsg[8];



extern void SendSeedRequestToEcu(void);
extern void SendKeyToEcu(INT32U keyFromServer);
extern void ENC8RespMsgDecode(void);
extern void FC1_Msg_unpack(void);
extern void AQ1_Msg_unpack(void);
extern void SIG1_Msg_unpack(void);
extern void Date_Msg_unpack(void);
extern void RCU_P_Msg_pack(void);




































#endif /*End of __HACG_CAN_H__ */

