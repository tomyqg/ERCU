#include "can_app.h"


/*CAN Messages Buffer Temp Define*/
uint8_t ENC8RespRxBuff[8];
uint8_t ERCUReqTxBuff[8];
uint8_t FC1RxBuff[8];  
uint8_t AQ1RxBuff[8];          
uint8_t SIG1RxBuff[8];                 
uint8_t DateRxBuff[8]; 
uint8_t RCUPresentMsg[8];



void SendSeedRequestToEcu(void)
{  
  ERCUReqTxBuff[0] = 2;
  ERCUReqTxBuff[1] = 0x27;
  ERCUReqTxBuff[2] = 0x01;
  (void)CAN_Tx(ERCU_REQUEST); 
}

void SendKeyToEcu(INT32U keyFromServer)
{  
  ERCUReqTxBuff[0] = 6;
  ERCUReqTxBuff[1] = 0x27;
  ERCUReqTxBuff[2] = 0x02;
  *(INT32U*)(&(ERCUReqTxBuff[3])) = keyFromServer;
  (void)CAN_Tx(ERCU_REQUEST); 
}


extern BOOLEAN g_lockVehicleEnable;
void SendStartroutineToEcu(void)
{  
  ERCUReqTxBuff[0] = 2;
  ERCUReqTxBuff[1] = 0x31;
  if(g_lockVehicleEnable)
  {
    ERCUReqTxBuff[2] = 0xA1;
  }
  else 
  {
    ERCUReqTxBuff[2] = 0xBE;
  }
  (void)CAN_Tx(ERCU_REQUEST); 
}


extern OS_EVENT *SEM_EcuResp;


void ENC8RespMsgDecode(void)
{
  switch(ENC8RespRxBuff[1])
  {
    case 0x67:
    {
      if(0x01 == ENC8RespRxBuff[2])
      {
        INT32U seedFromEcu = *(INT32U*)(&(ENC8RespRxBuff[3]));
        GPRS_FillSeedInResponse(seedFromEcu);
        (void)OSSemPost(SEM_EcuResp);//send seed to Server
      }
      else if(0x02 == ENC8RespRxBuff[2])
      {
        SendStartroutineToEcu();      
      }  
    }
    break;
    
    case 0x71:
    {
      if(0xA1 == ENC8RespRxBuff[2] || 0xBE == ENC8RespRxBuff[2])
      {    
        GPRS_FillPostiveResp();
        (void)OSSemPost(SEM_EcuResp);
      }
    }
    break;
    
    case 0x7F:
    {
      switch(ENC8RespRxBuff[2])
      {
        case 0x27:
        {
          INT8U subFunc = ERCUReqTxBuff[2];
          GPRS_FillNegRespSecurityAccess(subFunc);
          (void)OSSemPost(SEM_EcuResp);//failed    
        }
        break;
        
        case 0x31:
        {       
          GPRS_FillNegRespService31();
          (void)OSSemPost(SEM_EcuResp);            
        }
        break;
      }
      break;      
    }
    break;
    
    default:
    break;
  }
}




/*FC1_Msg variables UnPack function*/
uint32_t g_DSM_m_st_GlobalIndex0,g_DSM_m_st_GlobalIndex1;

void FC1_Msg_unpack(void)
{
    FC1_MsgType *p_FC1_Msg  = (FC1_MsgType *)FC1RxBuff;//(&FC1RxBuff[0]);
    g_DSM_m_st_GlobalIndex0 = p_FC1_Msg->DSM_m_st_GlobalIndex0;
    g_DSM_m_st_GlobalIndex1 = p_FC1_Msg->DSM_m_st_GlobalIndex1;   
}


/*AQ1_Msg variables UnPack function*/
uint32_t g_TR_m_q_DieselTotal,g_TR_m_q_LNGTotal;

void AQ1_Msg_unpack(void)
{
    AQ1_MsgType *p_AQ1_Msg  = (AQ1_MsgType *)AQ1RxBuff;
    g_TR_m_q_DieselTotal    = p_AQ1_Msg->TR_m_q_DieselTotal;
    g_TR_m_q_LNGTotal       = p_AQ1_Msg->TR_m_q_LNGTotal;   
}

/*SIG1_Msg variables UnPack function*/
uint16_t g_SID_m_p_CNGPrs;
uint8_t g_SID_m_t_CNGTemp;
uint8_t g_T15_SWT, g_MODE_SWT, g_CUTV_ST, g_NG_REL, g_EPS_ST;
uint32_t g_TC_m_ti_NGET;

void SIG1_Msg_unpack(void)
{
    SIG1_MsgType *p_SIG1_Msg = (SIG1_MsgType *)SIG1RxBuff;
    
    g_SID_m_p_CNGPrs  = p_SIG1_Msg->SID_m_p_CNGPrs;
    g_SID_m_t_CNGTemp = p_SIG1_Msg->SID_m_t_CNGTemp;
    g_T15_SWT         = p_SIG1_Msg->T15_SWT;
    g_MODE_SWT        = p_SIG1_Msg->MODE_SWT;
    g_CUTV_ST         = p_SIG1_Msg->CUTV_ST;
    g_NG_REL          = p_SIG1_Msg->NG_REL;
    g_EPS_ST          = p_SIG1_Msg->EPS_ST; 
    g_TC_m_ti_NGET    = p_SIG1_Msg->TC_m_ti_NGET;   
}



/*Date_Msg variables UnPack function*/
uint8_t g_TD_Year;
uint8_t g_TD_Day;
uint8_t g_TD_Month;
uint8_t g_TD_Hour;
uint8_t g_TD_Min;
uint8_t g_TD_Sec;

void Date_Msg_unpack(void)
{
    Date_MsgType *p_Date_Msg = (Date_MsgType *)DateRxBuff;
   
    g_TD_Year   = p_Date_Msg->TD_Year;
    g_TD_Day    = p_Date_Msg->TD_Day;
    g_TD_Month  = p_Date_Msg->TD_Month;
    g_TD_Hour   = p_Date_Msg->TD_Hour;
    g_TD_Min    = p_Date_Msg->TD_Min;
    g_TD_Sec    = p_Date_Msg->TD_Sec; 
}





extern volatile uint32_t g_ERCU_m_ct_Running,g_ERCU_m_ct_GPRSLastConnection;

void RCU_P_Msg_pack(void)
{
    RCU_P_MsgType *p_RCU_P_Msg  = (RCU_P_MsgType *)RCUPresentMsg;
    p_RCU_P_Msg->ERCU_m_ct_Running = g_ERCU_m_ct_Running;
    p_RCU_P_Msg->ERCU_m_ct_GPRSLastConnection  = g_ERCU_m_ct_GPRSLastConnection;   
}













