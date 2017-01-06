#ifndef __CAN_H_APP__
#define __CAN_H_APP__


#include "includes.h"
#include "can.h"
#include "gprs.h"



//δʹ��λ����0x00���

typedef struct _FC1_tag
{
  uint32_t DSM_m_st_GlobalIndex0;  /* ENC8����ȫ��ָʾ0 */ //Ϊ1��λ��ʾ��Ӧ�Ĺ���
  uint32_t DSM_m_st_GlobalIndex1;  /* ENC8����ȫ��ָʾ1 */ //Ϊ1��λ��ʾ��Ӧ�Ĺ���
}FC1_MsgType;

typedef struct _AQ1_tag
{
  uint32_t TR_m_q_DieselTotal;     /* ȼ�������ۻ��� */
  uint32_t TR_m_q_LNGTotal;        /* ��Ȼ�������ۻ��� */
}AQ1_MsgType;

typedef struct _SIG1_tag
{
  //High bytes
  uint16_t SID_m_p_CNGPrs;        /* ��Ȼ��ѹ�� */
  uint8_t SID_m_t_CNGTemp;        /* ��Ȼ���¶� */
  
  //Lowest bit
  byte T15_SWT         :1;        /* ��𿪹� */          //0��T15û���ϵ�
                                                          //1��T15�ϵ�
                                                      
  byte MODE_SWT        :1;        /* LNGģʽ���� */       //0��������ģʽ
                                                          //1������˫ȼ��ģʽ
                                                         
  byte CUTV_ST         :1;        /* �жϷ�����״̬ */    //0: �жϷ�OFF
                                                          //1: �жϷ�ON
                                                        
  byte NG_REL          :1;        /* ϵͳ��ǰ����ģʽ */  //0��ϵͳ�ڴ����¹���
                                                          //1��ϵͳ��˫ȼ��ģʽ�¹���
 
  //High 4 bits                                                         
  byte EPS_ST          :4;        /* ������ͬ��״̬ */    //0�����ź�
                                                          //1����ͬ��
                                                          //2����ͬ��
                                                          //3������ͬ����   
  uint32_t TC_m_ti_NGET;          /* ��Ȼ������ʱ�� */
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


//��Ҫ�µ籣�棬�ϵ��ȡ
typedef struct _RCU_P_tag
{
  uint32_t ERCU_m_ct_Running;  //ERCU�ۼ�����ʱ������λΪ��
  uint32_t ERCU_m_ct_GPRSLastConnection;  //���ϴ��յ�����ͨѶ��������������ERCU�ۼ�����ʱ������λΪ��
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

