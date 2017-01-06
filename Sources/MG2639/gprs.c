#include "gprs.h"
#include "can_app.h"

/*********************************************************
             Const String
**********************************************************/
static const INT8U str_connected[] = "CONNECTED";
static const INT8U str_connectfail[] = "CONNECT FAIL";
static const INT8U str_disconnected[] = "DISCONNECTED";
static const INT8U str_established[] = "ESTABLISHED";
static const INT8U server_ip[] =  "121.40.136.104";  
static const INT8U server_port[] = "5555"/*pxl UDP*/;//"4900"/*TCP*/ "5800"/*asq*/  

/*******************************************************
               GPRS AT Command
********************************************************/
static const INT8U gprs_cgdcont[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"";//建立PDP场景
static const INT8U gprs_cgatt[] = "AT+CGATT?"; /*=1*/               //附着GPRS网络
static const INT8U gprs_cgclass[] = "AT+CGCLASS?"; 
static const INT8U gprs_cgact[] = "AT+CGACT?";  /// =1,1//激活移动场景，有一定延迟 

static const INT8U gprs_zpnum[] = "AT+ZPNUM=\"CMNET\",\"\",\"\"";  //,1,0
static const INT8U gprs_zpppopen[] = "AT+ZPPPOPEN";
static const INT8U gprs_zpppclose[] = "AT+ZPPPCLOSE";  
static const INT8U gprs_zpppstatus[] = "AT+ZPPPSTATUS";
static const INT8U gprs_zipgetip[] = "AT+ZIPGETIP";

static const INT8U gprs_zipsetup[] = "AT+ZIPSETUP=";   //TCP
static const INT8U gprs_zipclose[] = "AT+ZIPCLOSE=";  
static const INT8U gprs_zipsend[] = "AT+ZIPSEND=";
static const INT8U gprs_zipstatus[] = "AT+ZIPSTATUS=";

static const INT8U gprs_zipsetupu[] = "AT+ZIPSETUPU="; //UDP
static const INT8U gprs_zipcloseu[] = "AT+ZIPCLOSEU="; 
static const INT8U gprs_zipsendu[] = "AT+ZIPSENDU=";
static const INT8U gprs_zipstatusu[] = "AT+ZIPSTATUSU=";


/*******************************************************
               GPRS variable
********************************************************/
static volatile struct _GPRS_LINK_INFO g_gprsLinkInfo;
static volatile struct _GPRS_RX_INFO  g_gprsRxInfo;
volatile struct _GPRS_CONTENT  g_gprsContent;

volatile INT32U g_ERCU_m_ct_GPRSLastConnection;

static volatile INT8U g_gprsResponse[RESP_TO_SERVER_TOTAL_LEN] = 
{ 
  ADDRESS_CTRLPC,//目标地址
  ADDRESS_ERCU,//源地址
  DEVICE_ID0,DEVICE_ID1,DEVICE_ID2,DEVICE_ID3,//设备地址
  0,                   //服务标识
  0x00,RESP_TO_SERVER_PARM_LEN,//参数长度
};





/*******************************************************
               GPRS Functions Define
********************************************************/
static BOOLEAN GPRS_Prepare(void)
{
  BOOLEAN ret = TRUE;
  INT8U cmdIdx;
  
  const INT8U * Ary_GprsPrepareCmd[] = 
  {
    gprs_cgdcont,
    gprs_cgclass,
    gprs_cgatt,
    gprs_cgact,
  };
   
  for(cmdIdx=0; cmdIdx<sizeof(Ary_GprsPrepareCmd)/2; cmdIdx++)//除以2，因为16位MCU地址是2个字节
  {
    SendATCmd(Ary_GprsPrepareCmd[cmdIdx]);
    ret = WaitResponseOK(2000);
    //if(ret == TRUE)//For debug
      ClearSci2RxBuffer();
    if(ret == FALSE)
  	 	return ret;
  } 

  g_gprsLinkInfo.tp = GPRS_TP;

	return ret;	 	 	 	 	 	
}


static BOOLEAN GPRS_Open(void)
{	 
  BOOLEAN ret;
  
  SendATCmd(gprs_zpnum);			
  ret = WaitResponseOK(WAIT_TIME_GPRS_OPEN_TOTAL);
  ClearSci2RxBuffer();
  if(ret == FALSE)
    return FALSE;

  SendATCmd(gprs_zpppopen);
  ret =   WaitResponseStrStr(str_connected, str_disconnected, WAIT_TIME_GPRS_OPEN_TOTAL)
       || WaitResponseStrStr(str_established, str_disconnected, WAIT_TIME_GPRS_OPEN_TOTAL);
  ClearSci2RxBuffer();
 	return ret;
}           



static BOOLEAN GPRS_Close(void)
{
  BOOLEAN ret;
  
  SendATCmd(gprs_zpppclose);			
  ret = WaitResponseOK(500);
  ClearSci2RxBuffer();
  if(ret == FALSE)
    return FALSE;
  
  return TRUE;
}


  
static BOOLEAN GPRS_GetPPPSts(void) //1000ms timeout
{
  BOOLEAN ret;
  
  SendATCmd(gprs_zpppstatus);			
  ret = WaitResponseStrStr(str_established, str_disconnected, WAIT_TIME_GET_PPP_STATUS_TOTAL);
  ClearSci2RxBuffer();
  g_gprsLinkInfo.status = (ret)?LINK_STS_OK:LINK_STS_FAILED;
 	
 	return ret;
}  




static BOOLEAN GPRS_SetupLink(void)
{
  INT16U setupWaitTime;
  const INT8U *ipsetupCmd;
  BOOLEAN ret;
  
  if(g_gprsLinkInfo.tp==TCP)
  {
      setupWaitTime = WAIT_TIME_TCP_LINK_TOTAL;
      ipsetupCmd = gprs_zipsetup;
  }
  else if(g_gprsLinkInfo.tp==UDP)
  {
      setupWaitTime = WAIT_TIME_UDP_LINK_TOTAL;
      ipsetupCmd = gprs_zipsetupu;
  }
  
  SCI2_SendStr(ipsetupCmd);
  SCI2_Send1Byte('0'+g_gprsLinkInfo.channel);
  SCI2_Send1Byte(',');
  SCI2_SendStr(server_ip);
  SCI2_Send1Byte(',');
  SCI2_SendStr(server_port);
  SCI2_Send1Byte('\r');
  
  ret = WaitResponseStrStr(str_connected, str_connectfail, setupWaitTime);
  ClearSci2RxBuffer();
  g_gprsLinkInfo.status = (ret)?LINK_STS_OK:LINK_STS_FAILED;
 	
 	return ret; 
}



void GPRS_TryLink(void)
{  
  g_moudleErrno = (GPRS_Prepare())?ERR_MOUDLE_NONE:ERR_GPRS_PREPARE;
  g_moudleErrno = (GPRS_Open())?ERR_MOUDLE_NONE:ERR_GPRS_OPEN;
  g_moudleErrno = (GPRS_SetupLink())?ERR_MOUDLE_NONE:ERR_GPRS_LINK;
}

  

static BOOLEAN GPRS_CloseLink(void)
{
  BOOLEAN ret;
  
  if(g_gprsLinkInfo.tp==TCP)
  {
    SCI2_SendStr(gprs_zipclose);
  }
  else if(g_gprsLinkInfo.tp==UDP)
  {
    SCI2_SendStr(gprs_zipcloseu);
  }
  SCI2_Send1Byte('0'+g_gprsLinkInfo.channel);
  SCI2_Send1Byte('\r');			
  ret = WaitResponseOK(500);
  ClearSci2RxBuffer();
  if(ret == FALSE)
    return FALSE;
  g_gprsLinkInfo.status = LINK_STS_TO_LINK;
  
  return TRUE;
}


static BOOLEAN GPRS_GetLinkSts(void)
{
  INT16U getstsWaitTime;
  const INT8U *ipstsCmd;
  BOOLEAN ret;

  if(g_gprsLinkInfo.tp==TCP)
  {
    getstsWaitTime = WAIT_TIME_GET_TCP_STATUS_TOTAL;
    ipstsCmd = gprs_zipsetup;
  }
  else if(g_gprsLinkInfo.tp==UDP)
  {
    getstsWaitTime = WAIT_TIME_GET_UDP_STATUS_TOTAL;
    ipstsCmd = gprs_zipsetupu;
  }

  SCI2_SendStr(ipstsCmd);
  SCI2_Send1Byte('0'+g_gprsLinkInfo.channel);
  SCI2_Send1Byte('\r');			
  ret = WaitResponseStrStr(str_established, str_disconnected, getstsWaitTime);
  ClearSci2RxBuffer();
  g_gprsLinkInfo.status = (ret)?LINK_STS_OK:LINK_STS_FAILED;
 	
 	return ret;
}



static BOOLEAN GPRS_SendMsg(INT8U msg[], INT16U length)
{
	BOOLEAN ret;

  if(g_gprsLinkInfo.tp==TCP)
  {
    SCI2_SendStr(gprs_zipsend);
  }
  else if(g_gprsLinkInfo.tp==UDP)
  {
    SCI2_SendStr(gprs_zipsendu);
  }  
  	
	SCI2_Send1Byte('0'+g_gprsLinkInfo.channel);
  SCI2_Send1Byte(',');
  SendLengthDigit(length);
  SCI2_Send1Byte('\r');
  	
  ret = WaitResponseChrStr('>', str_error, 2000);
  ClearSci2RxBuffer();  
  if(ret == FALSE)
	 	return FALSE;
  SCI2_SendByLength(msg,length);
 
  ret = WaitResponseOK(2000);//may need a long time
  ClearSci2RxBuffer();
  if(ret == FALSE)
	 	return FALSE;
  
  return TRUE; 
}



void GPRS_SendResp(void)
{
  g_gprsResponse[RESP_TO_SERVER_TOTAL_LEN-1] = CountChecksum(0, RESP_TO_SERVER_TOTAL_LEN-1, g_gprsResponse);
  
  if(GPRS_SendMsg(g_gprsResponse, RESP_TO_SERVER_TOTAL_LEN))
  {
    GPRS_Reset_hbErrcnt();
  }
  else
  {
    GPRS_Increase_hbErrcnt();
  }
}



void GPRS_CheckLinkSts(void)
{
  (void)GPRS_GetPPPSts();
  (void)GPRS_GetLinkSts();  
}



volatile INT8U g_heartbeatPacket[HEARTBEAT_TOTAL_LEN] = 
{
  ADDRESS_SERVER,//目标地址const INT8S str[], INT8U len
  ADDRESS_ERCU,//源地址
  DEVICE_ID0,DEVICE_ID1,DEVICE_ID2,DEVICE_ID3,//设备地址
  SER_ERCU_SERVER_REGISTER,                   //服务标识
  0x00,HEARTBEAT_PARM_LEN,//参数长度
};



void GPRS_Reset_hbErrcnt(void)
{
  g_gprsLinkInfo.hbErrcnt = 0;
}

void GPRS_Increase_hbErrcnt(void)
{
  g_gprsLinkInfo.hbErrcnt++;
}


void GPRS_Reset_ConnectTime(void)
{
  g_ERCU_m_ct_GPRSLastConnection = 0;
}


void GPRS_Increase_ConnectTime(void)
{
  g_ERCU_m_ct_GPRSLastConnection++;
}


static void GPRS_FillHbPacket(void)
{
  INT8U canMsgIdx;
  INT8U gpsIdx;
  INT8U hbChecksum;
  BOOLEAN gpsCntValid;
  
  for(canMsgIdx=0; canMsgIdx<8; canMsgIdx++)
  {
    g_heartbeatPacket[DATA_START_BYTE_NO+canMsgIdx]     = FC1RxBuff[canMsgIdx];
    g_heartbeatPacket[DATA_START_BYTE_NO+canMsgIdx+8]   = AQ1RxBuff[canMsgIdx];
    g_heartbeatPacket[DATA_START_BYTE_NO+canMsgIdx+8+8] = SIG1RxBuff[canMsgIdx];
  }

  gpsCntValid = g_gpsInfo.cnt>0 && g_gpsInfo.cnt<BYTES_GPSINFO;
  if(gpsCntValid)
  {	  
  	for(gpsIdx=0; gpsIdx<g_gpsInfo.cnt; gpsIdx++)
  	{
  	  g_heartbeatPacket[DATA_START_BYTE_NO+8+8+8+gpsIdx] = g_gpsInfo.buff[gpsIdx]; 
  	}
  	for(gpsIdx=g_gpsInfo.cnt; gpsIdx<BYTES_GPSINFO; gpsIdx++)
  	{
  	  g_heartbeatPacket[DATA_START_BYTE_NO+8+8+8+gpsIdx] = 0; 
  	}  	
  }
  else
  {
     for(gpsIdx=0; gpsIdx<(BYTES_GPSINFO); gpsIdx++)
     {
       g_heartbeatPacket[DATA_START_BYTE_NO+gpsIdx+8+8+8] = 0;
     }
  }

  hbChecksum = CountChecksum(0, HEARTBEAT_TOTAL_LEN-1, g_heartbeatPacket);
  g_heartbeatPacket[HEARTBEAT_TOTAL_LEN-1] = (INT8U)hbChecksum;
}



void GPRS_SendHeartbeat(void)
{  
	if(g_gprsLinkInfo.status==LINK_STS_OK)
  {
    GPRS_FillHbPacket();
    if(TRUE == GPRS_SendMsg(g_heartbeatPacket,HEARTBEAT_TOTAL_LEN))
    {
      GPRS_Reset_hbErrcnt();
      GPRS_Reset_ConnectTime();
    }
    else
    {
      GPRS_Increase_hbErrcnt();
    }
  }
}



static BOOLEAN GPRS_StsAbnormal(void)
{
    return (   (g_gprsLinkInfo.status==LINK_STS_FAILED)
            || (g_gprsLinkInfo.hbErrcnt>=HEART_BEAT_ERR_MAX));
}



void GPRS_CheckHeartbeat(void)
{  
  static INT8U checkStsCnt;
  
  checkStsCnt++;
  if(HEART_BEAT_ERR_MAX<=checkStsCnt)
  {
    checkStsCnt = 0;
    GPRS_CheckLinkSts();
  } 
    
  if(GPRS_StsAbnormal())
  {
    (void)GPRS_CloseLink();
    (void)GPRS_Close();
    GPRS_Reset_hbErrcnt();
    g_gprsLinkInfo.status = LINK_STS_TO_LINK;
    g_gprsLinkInfo.channel += 1;
    g_gprsLinkInfo.channel %= N_LINK_CHAN;
    GPRS_TryLink();
  }                                  
}




static BOOLEAN GPRS_FindDigitIdx(INT8U rxCnt, INT8U *pFirstIdx, INT8U *pLastIdx)
{
  INT8U idx;
  INT8U cnt = 0;
  BOOLEAN firstFound = FALSE;
  BOOLEAN lastFound = FALSE;
  
  for(idx=0; idx<rxCnt; idx++)
  {
  	if(g_gprsRxInfo.buff[idx]==',')
  	{
  	  cnt++;
  	}
  	if(!firstFound)
  	{
  	  if(COMMA_NUM_BEFORE_LENGTH==cnt)
    	{
    	  firstFound = TRUE;
    	  *pFirstIdx = idx+1;
    	} 
  	}
  	if((COMMA_NUM_BEFORE_LENGTH+1)==cnt)
  	{
  	  lastFound = TRUE;
  	  *pLastIdx = idx-1;
    	break;
  	}
  }
  
  return (firstFound && lastFound);
}



const INT8U MultiplyFactor[MAX_NUM_OF_LEN_IDX] = {1,10,100,1000};
/**************************************************************************
 1.Calculate the num of length character(ASCII code) and validate it;
 2.If valid, calculate gprs data length
****************************************************************************/
static INT8U GPRS_GetContentLen(INT8U first_digit_idx, INT8U last_digit_idx)
{
  INT8U len;
  INT8U digitCnt;
  BOOLEAN digitCntIsValid;
  INT8U numIdx;
  
  len = 0;
  digitCnt = last_digit_idx - first_digit_idx + 1;
  digitCntIsValid = (digitCnt>=1 && digitCnt<=MAX_NUM_OF_LEN_IDX);
  if(digitCntIsValid)
  {
    for(numIdx=0; numIdx<digitCnt; numIdx++)
    {
      len += MultiplyFactor[numIdx]*(g_gprsRxInfo.buff[last_digit_idx-numIdx]-'0');    
    }
  }
  
  return len;  
}


static void GPRS_CpyBuff(void)
{  
  g_gprsRxInfo.cnt = g_sci2RxInfo.cnt;
  (void)memcpy(g_gprsRxInfo.buff,g_sci2RxInfo.buff,g_gprsRxInfo.cnt);
  ClearSci2RxBuffer();
}



void GPRS_GetContent(void)
{
  INT8U first_digit_idx;  
  INT8U last_digit_idx;
  INT8U dataIdx;

  GPRS_CpyBuff();
  if(GPRS_FindDigitIdx(g_gprsRxInfo.cnt, &first_digit_idx, &last_digit_idx))
  {
    g_gprsContent.len = GPRS_GetContentLen(first_digit_idx, last_digit_idx);
    for(dataIdx=0; dataIdx<g_gprsContent.len; dataIdx++)
    {
      g_gprsContent.buff[dataIdx] = g_gprsRxInfo.buff[last_digit_idx+2+dataIdx];
    }
  }
}



void GPRS_FillSeedInResponse(INT32U seedFromEcu)
{
  g_gprsResponse[DATA_START_BYTE_NO-BYTES_PARM-BYTES_SERVICE] = SER_CTRLPC_ERCU_ENABLE+0x40;
  *(INT32U*)(&(g_gprsResponse[DATA_START_BYTE_NO])) = seedFromEcu;
}


void GPRS_FillPostiveResp(void)
{
  g_gprsResponse[DATA_START_BYTE_NO-BYTES_PARM-BYTES_SERVICE] = SER_CTRLPC_ERCU_VERIFY+0x40;
  g_gprsResponse[DATA_START_BYTE_NO  ] = 0;
  g_gprsResponse[DATA_START_BYTE_NO+1] = 0;
  g_gprsResponse[DATA_START_BYTE_NO+2] = 0;
  g_gprsResponse[DATA_START_BYTE_NO+3] = 1;
}
        

void GPRS_FillNegRespSecurityAccess(INT8U subFunc)
{
  if(0x01==subFunc)
    g_gprsResponse[DATA_START_BYTE_NO-BYTES_PARM-BYTES_SERVICE] = SER_CTRLPC_ERCU_ENABLE+0x80;
  else if(0x02==subFunc)
    g_gprsResponse[DATA_START_BYTE_NO-BYTES_PARM-BYTES_SERVICE] = SER_CTRLPC_ERCU_VERIFY+0x80;      
  g_gprsResponse[DATA_START_BYTE_NO  ] = 0;
  g_gprsResponse[DATA_START_BYTE_NO+1] = 0;
  g_gprsResponse[DATA_START_BYTE_NO+2] = 0;
  g_gprsResponse[DATA_START_BYTE_NO+3] = 0;
}


void GPRS_FillNegRespService31(void)
{
  g_gprsResponse[DATA_START_BYTE_NO-BYTES_PARM-BYTES_SERVICE] = SER_CTRLPC_ERCU_VERIFY+0x80;
  g_gprsResponse[DATA_START_BYTE_NO  ] = 0;
  g_gprsResponse[DATA_START_BYTE_NO+1] = 0;
  g_gprsResponse[DATA_START_BYTE_NO+2] = 0;
  g_gprsResponse[DATA_START_BYTE_NO+3] = 0;
}


void GPRS_FillTimeoutResp(void)
{
  g_gprsResponse[DATA_START_BYTE_NO-BYTES_PARM-BYTES_SERVICE] = g_serviceCtrlpcToErcu+0x80;      
  g_gprsResponse[DATA_START_BYTE_NO+0] = 0;
  g_gprsResponse[DATA_START_BYTE_NO+1] = 0;
  g_gprsResponse[DATA_START_BYTE_NO+2] = 0x01;//待定，超时故障码
  g_gprsResponse[DATA_START_BYTE_NO+3] = 0;  
}


volatile INT8U g_gprsCmd[RESP_TO_SERVER_TOTAL_LEN];
volatile BOOLEAN g_lockVehicleEnable;
volatile INT8U g_serviceCtrlpcToErcu;

void GPRS_HandleCmd(void)
{
  if(ADDR_MATCH(g_gprsCmd))
  {
    if(DEV_MATCH(g_gprsCmd))
    {     
       if(IS_SERVICE(SER_CTRLPC_ERCU_ENABLE,g_gprsCmd))
       {
          g_serviceCtrlpcToErcu = SER_CTRLPC_ERCU_ENABLE;
          g_lockVehicleEnable = g_gprsCmd[12];
          SendSeedRequestToEcu();//考虑超时处理（ECU不响应）
       }         
       else if(IS_SERVICE(SER_CTRLPC_ERCU_VERIFY,g_gprsCmd))//key from PC
       {
          INT32U keyFromServer = *(INT32U*)(&(g_gprsContent.buff[9]));
          g_serviceCtrlpcToErcu = SER_CTRLPC_ERCU_VERIFY;
          SendKeyToEcu(keyFromServer);
       }
    }
  }
} 