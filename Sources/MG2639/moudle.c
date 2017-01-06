#ifndef __MOUDLE_C_
#define __MOUDLE_C_
#include "moudle.h"
#include"hardeware.h"


/*********************************************************
             Const String
**********************************************************/
static const INT8U str_ok[] = "OK";
const INT8U str_error[] = "ERROR";

/*********************************************************
             Const AT Command
**********************************************************/
static const INT8U at_at[] = "AT";
static const INT8U at_ate0[] = "ATE0";//(使能回馈通道),ATE0时不在终端上显示输入命令;ATE1时在终端上显示输入命令;
static const INT8U at_atq1[] = "ATQ1";
static const INT8U at_simins[] = "AT*TSIMINS?";
//"AT+CPIN?",不考虑PIN码输入，一般情况下是没有必要的                            
static const INT8U at_cpas[] = "AT+CPAS";//模块状态查询
//"AT+ZVERSWITCH=3",
static const INT8U at_creg[] = "AT+CREG?"; 
static const INT8U at_cfun[] = "AT+CFUN=1,0"; //设置模块功能   //Can Not Be Used
//"AT+CGMR",获取产品版本号
//"AT+CMEE=1", 错误报告设置
static const INT8U at_csq[] = "AT+CSQ"; //信号查询
static const INT8U at_pwroff[] = "AT+ZPWROFF"; //信号查询



/**********************************************************
             Moudle Variable Define                         
***********************************************************/
volatile struct _SCI1_RX_INFO  g_sci1RxInfo;
volatile struct _SCI2_RX_INFO  g_sci2RxInfo;
MOUDLE_RESULT g_moudleErrno;


/*********************************************************
              Common or Basic Functions                         
**********************************************************/
INT8U CountChecksum(INT8U index, INT8U length, INT8U data[])
{
  INT8U checksum;
  
  checksum = 0;
  while(length>0)
  {
    length --;
    checksum = (INT8U)(checksum + data[length+index]);    
  }
    
  return checksum;    
}


//rx buffer must be cleared
void ClearSci2RxBuffer(void)
{
	while(g_sci2RxInfo.cnt)
	{
	  g_sci2RxInfo.cnt --;  
	  g_sci2RxInfo.buff[g_sci2RxInfo.cnt] = 0;
	}
}


void SendLengthDigit(INT16U length)
{
  if(length>=100)
	{
	   SCI2_Send1Byte(length/100+'0');
	}
	if(length>=10)
	{
	   SCI2_Send1Byte((length%100)/10+'0');
	}
	if(length>=1)
	{
	   SCI2_Send1Byte((length%10)+'0');
	}    
}


BOOLEAN WaitResponseChrStr(const INT8U chrSuccess, 
                     const INT8U *strFailed, 
                     INT16U waitTime)
{
	INT8U count = (INT8U)(waitTime/(1000/OS_TICKS_PER_SEC));
  
  while(count)
  {
    if(NULL != strchr(g_sci2RxInfo.buff,chrSuccess)) 
    { 
      return TRUE;
      break;
    }
    else if(NULL != strstr(g_sci2RxInfo.buff, strFailed)) 
    { 
      return FALSE;
      break;
    }
    else
    {
    	OSTimeDly(1);
    	count--;
    	if(0 == count) 
    	{ 
        return FALSE;
      }
    }
  }
}



BOOLEAN WaitResponseStrStr(const INT8U *strSuccess, 
                     const INT8U *strFailed, 
                     INT16U waitTime)
{
	INT8U count = (INT8U)(waitTime/(1000/OS_TICKS_PER_SEC));

	while(count)
	{
		if(NULL != strstr(g_sci2RxInfo.buff, strSuccess))
		{
		  return TRUE;
		}
		else if(NULL != strstr(g_sci2RxInfo.buff, strFailed))
		{
		  return FALSE;
		}
		else
		{
			(void)OSTimeDly(1);
			count--;
			if(0 == count) 
  		{
  			return FALSE;
  		}
		}
	}
}



BOOLEAN WaitResponseOK(INT16U t)
{
	INT8U count = (INT8U)(t/(1000/OS_TICKS_PER_SEC));

	while(count)
	{
		if(NULL != strstr(g_sci2RxInfo.buff, str_ok))
		{
		  return TRUE;
		}
		else if(NULL != strstr(g_sci2RxInfo.buff, str_error))
		{
		  return FALSE;
		}
		else
		{
			(void)OSTimeDly(1);
			count--;
		}
	}
	
	return FALSE;		
}


void SendATCmd(const INT8U cmd[]) 
{
  SCI2_SendStr(cmd);
  SCI2_Send1Byte('\r');
}



/********************************************************************************************
***                               Moudle Functions                         
********************************************************************************************/
void Moudle_PowerOn(void) 
{
  MG2639_PWRKEY_N_LOW();
  (void)OSTimeDlyHMSM(0,0,5,0);
  MG2639_PWRKEY_N_HIGH();
}

//when module MG2639 in on state,delay 2s - 5s low to power off it 
void Moudle_PowerOff(void) 
{
  MG2639_PWRKEY_N_LOW();
  //(void)OSTimeDlyHMSM(0,0,5,0);
  //MG2639_PWRKEY_N_HIGH();
}


void Moudle_PowerOffWithCmd(void)
{
  SendATCmd(at_pwroff); 
}


void Moudle_SetBaudrate(void)
{  
  SCI_OpenCommunication(SCI_MAP_GSM,BR);
  SCI2_INT_ENABLE();
}



BOOLEAN Moudle_Init(void)
{
  BOOLEAN ret;
  INT8U i;  
  INT8U *ATCmdArray[]={                        
    at_ate0,//Only for release version
    //at_cfun,
  };
  
  ClearSci2RxBuffer();
  
  for(i=0; i<(sizeof(ATCmdArray)/2); i++)//除以2，16位MCU,地址是2个字节
  {
    SendATCmd(ATCmdArray[i]); 
    ret = WaitResponseOK(500);
    //if(TRUE==ret)//For debug
      ClearSci2RxBuffer();
    if(FALSE==ret)
    {
      g_moudleErrno = ERR_MOUDLE_INIT;
  	 	return FALSE;
    }
  } 
 
  return TRUE;
}


BOOLEAN Moudle_CheckCard(void)
{
  BOOLEAN ret;
  
  SendATCmd(at_simins); 
  ret = WaitResponseOK(500);
  ClearSci2RxBuffer();
  if(FALSE==ret)
  {
    g_moudleErrno = ERR_SIM_CHECK;
	 	return FALSE;
  }

  return TRUE;
} 



/*************************************************
              查询网络是否注册
*************************************************/
INT8U moudleSts;
BOOLEAN Moudle_CheckReg(void)
{
  /*SendATCmd(at_cpas); 
  if(FALSE == WaitResponseOK(500))
  {
    moudleSts = 1;
    ClearSci2RxBuffer();
	 	//return FALSE;
  }  
  if((NULL != strchr(g_sci2RxInfo.buff,'0')))
  {
    moudleSts = 2;
    ClearSci2RxBuffer();
  }
  else
  {
    moudleSts = 3;
    ClearSci2RxBuffer();
	 	//return FALSE;    
  }  */
  
  /*SendATCmd(at_csq);
  WaitResponseOK(2500);
  ClearSci2RxBuffer(); */

  SendATCmd(at_creg); 
  if(FALSE == WaitResponseOK(500))
  {
    ClearSci2RxBuffer();
  }
  
  if((NULL != strchr(g_sci2RxInfo.buff,'1')) || (NULL != strchr(g_sci2RxInfo.buff,'5')))//1=已注册本地网络，5=已注册，处于漫游状态 
  {
    ClearSci2RxBuffer();
    return TRUE;
  }
  else
  {
    ClearSci2RxBuffer();
	 	return FALSE;    
  }	 	
}
  










void Sci1IntHandle(void)//GPS
{
  volatile INT8U ch = SCI1DRL;
  
  if('$' == ch)
  {
    g_sci1RxInfo.buff[0] = ch;
    g_sci1RxInfo.cnt = 1;
  }
  else
  {
    g_sci1RxInfo.buff[g_sci1RxInfo.cnt] = ch;
    g_sci1RxInfo.cnt ++;
    if(10 == ch)
    {
      extern OS_EVENT *SEM_GPS;
      SCI1_INT_DISABLE();
      (void)OSSemPost(SEM_GPS);
    }
  }
}




void Sci2IntHandle(void)//MG2639 SMS and GPRS
{  
  static BOOLEAN cmtFlag = FALSE;
  static BOOLEAN recvFlag = FALSE;
  
  g_sci2RxInfo.buff[g_sci2RxInfo.cnt] = SCI2DRL;
  g_sci2RxInfo.cnt ++;
  g_sci2RxInfo.cnt %= SCI2_RX_BUFFER_LENGTH;
  
  if(!cmtFlag && RECEIVED_SMS(g_sci2RxInfo.buff, g_sci2RxInfo.cnt)) 
	{
		cmtFlag = TRUE;
	}
	
	if(cmtFlag && RECEIVED_CHAR10(g_sci2RxInfo.buff, g_sci2RxInfo.cnt))
	{
    extern OS_EVENT *SEM_SMS;
	  cmtFlag = FALSE;
		(void)OSSemPost(SEM_SMS);
	}
	
	if(!recvFlag && RECEIVED_GPRS(g_sci2RxInfo.buff, g_sci2RxInfo.cnt))
	{
	  recvFlag = TRUE;
	}
	
  if(recvFlag && RECEIVED_CHAR10(g_sci2RxInfo.buff, g_sci2RxInfo.cnt))
	{
    extern OS_EVENT *SEM_GPRSRecv;
	  recvFlag = FALSE;
		(void)OSSemPost(SEM_GPRSRecv);
	}
}

































































































































#endif/*__MG2639_C_*/