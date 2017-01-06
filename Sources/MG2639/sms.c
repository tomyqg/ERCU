#include "sms.h"
#include <ctype.h>
#include "can_app.h"


static const INT8U at_cmgf[] = "AT+CMGF=1";                   //Text Mode
static const INT8U at_cnmi[] = "AT+CNMI=2,1,0,0,0";           //短信存储方式
static const INT8U at_cpms[] = "AT+CPMS=\"ME\",\"ME\",\"ME\"";//ME表示NV，SM表示SIM
static const INT8U at_cpbs[] = "AT+CPBS=\"SM\"";              //ME表示NV，SM表示SIM卡
static const INT8U at_cmgr[] = "AT+CMGR=";                    //Read SMS
static const INT8U at_cmgs[] = "AT+CMGS=";                    //Send SMS
static const INT8U at_cmgd[] = "AT+CMGD=1,4";                 //Delete SMS
static const INT8U at_cpbr[] = "AT+CPBR=50";
static const INT8U at_cpbw[] = "AT+CPBW=50,\"";                            
static const INT8U after_cpbw[] = "\",129,\"xyl\"";

static const INT8U cmd_LockNum[] = "#LockNum";                               
static const INT8U cmd_ChangeNum[] = "#Change"; //变更绑定受控手机号码                              
static const INT8U cmd_LockVechile[] = "#NATV";
static const INT8U cmd_UnlockVechile[] = "#ATV";
static const INT8U cmd_ReadEcuId[] = "#ECUID";
static const INT8U cmd_ReadErrCode[] = "#ERRCODE";
static const INT8U cmd_ReadState[] = "#STATE";
static const INT8U cmd_ReadFuel[] = "#FUEL";
static const INT8U resp_InvalidContent[] = "Invalid content";
static const INT8U resp_InvalidPhonenum[] = "Invalid number";
static const INT8U resp_SysPhonenum[] = "Now system phone number is";


/*******************************************************
                  SMS Variable
********************************************************/
static struct _SMS_RX_INFO  g_smsRxInfo;
static struct _SMS_INDEX    g_smsIndex;
static struct _SMS_CONTENT  g_smsContent;


const INT8U g_romPhonenum[] = "15623351705";
static volatile INT8U g_sysPhonenum[PHONENUM_LEN];
static volatile INT8U g_comingPhonenum[PHONENUM_LEN];
static volatile INT8U g_chaningPhonenum[PHONENUM_LEN];

static INT8U g_smsResponse[SMS_CONTENT_LEN];




/*******************************************************
                  SMS Functions 
********************************************************/
static void SMS_CpyBuff(void)
{  
  g_smsRxInfo.cnt = g_sci2RxInfo.cnt;
  (void)memcpy(g_smsRxInfo.buff,g_sci2RxInfo.buff,g_smsRxInfo.cnt);
  ClearSci2RxBuffer();
}



static void SMS_SetSysPhonenum(INT8U *settingPhonenum)
{
  (void)memcpy(g_sysPhonenum, settingPhonenum, PHONENUM_LEN);   
}



static void SMS_GetPhonenumInBook(void)
{
  INT8U cpbrIdx;
  INT8U firstNumIdx;
  
  for(cpbrIdx=0; cpbrIdx<g_smsRxInfo.cnt; cpbrIdx++)
  {
    if(READ_BOOK_OK(g_smsRxInfo.buff,cpbrIdx))
    {
      for(firstNumIdx=cpbrIdx+5; firstNumIdx<g_smsRxInfo.cnt; firstNumIdx++)
      {
        if(g_smsRxInfo.buff[firstNumIdx-1] == '"')
        {
          SMS_SetSysPhonenum((INT8U*)(&(g_smsRxInfo.buff[firstNumIdx])));//(void)memcpy(g_sysPhonenum, &(g_smsRxInfo.buff[firstNumIdx]), PHONENUM_LEN);
      	  return;          
        }
      }
    }
  } 
}



static BOOLEAN SMS_ValidateComingPhonenum(void)
{
  INT8U numIdx;
  
  //return strncmp(g_sysPhonenum, g_comingPhonenum, PHONENUM_LEN);
  
  for(numIdx=0; numIdx<PHONENUM_LEN; numIdx++)
  {
    if(g_sysPhonenum[numIdx] != g_comingPhonenum[numIdx])
    {
      return FALSE;
    }
  }
  
  return TRUE;
}



static BOOLEAN ReadPhonebook(void)
{
  BOOLEAN ret;
  
  SendATCmd(at_cpbr);   
  ret = WaitResponseOK(500);

 	return ret;
}



static BOOLEAN WritePhoneBook(void)
{
  BOOLEAN ret;

  SCI2_SendStr(at_cpbw);
  SCI2_SendByLength(g_chaningPhonenum, PHONENUM_LEN);
  SCI2_SendStr(after_cpbw);
  SCI2_Send1Byte('\r');
  ret = WaitResponseOK(500);
  ClearSci2RxBuffer();
  return ret;
}



static BOOLEAN SMS_SendMsg(INT8U msg[])
{
	BOOLEAN ret;

 	SCI2_SendStr(at_cmgs);
 	SCI2_Send1Byte('"');
 	SCI2_SendByLength(g_sysPhonenum, PHONENUM_LEN);
 	SCI2_Send1Byte('"');
 	SCI2_Send1Byte('\r'); 	
 	
  ret = WaitResponseChrStr('>', str_error, WAIT_TIME_SMS_SEND_TOTAL);
  ClearSci2RxBuffer(); 
  if(ret == FALSE)
  {
	 	return FALSE;
  }

 	SCI2_SendStr(msg);
 	(void)SCI2_Send1Byte(0x1A);//Ctrl-Z 
 	ret = WaitResponseOK(WAIT_TIME_SMS_SEND_TOTAL);//may need a long time
  ClearSci2RxBuffer();
  if(ret == FALSE)
	 	return FALSE;
  
  return TRUE; 
}
 



static void SMS_GetIndex(void)
{
  INT8U idx;
  
  for(idx=g_smsRxInfo.cnt; idx>0; --idx)                             //先把中断里的自加减一
	{
  	BOOLEAN indexFound =  g_smsRxInfo.buff[idx-0] == ',' 
  	                   && g_smsRxInfo.buff[idx-6] == ':';
  	if(indexFound)
  	{
      if(isdigit(g_smsRxInfo.buff[idx+3]))
      {
        g_smsIndex.len=2;
    	  g_smsIndex.buff[0]=g_smsRxInfo.buff[idx+2];
    	  g_smsIndex.buff[1]=g_smsRxInfo.buff[idx+3];
    	}
      else  
      {
        g_smsIndex.len=1;
    	  g_smsIndex.buff[0]=g_smsRxInfo.buff[idx+2];
    	}
  	}
	}
}



static BOOLEAN SMS_ReadMsg(void) 
{
 	BOOLEAN ret;
 	
 	SCI2_SendStr(at_cmgr);
 	if(g_smsIndex.len>=1)
 	{
 	  SCI2_Send1Byte(g_smsIndex.buff[0]);
 	}
 	if(g_smsIndex.len>=2)
 	{
 	  SCI2_Send1Byte(g_smsIndex.buff[1]);
 	}
 	/*if(g_smsIndex.len>=3)
 	{
 	  SCI2_Send1Byte(g_smsIndex.buff[2]);
 	}*/
 	SCI2_Send1Byte('\r');
 	
 	ret = WaitResponseOK(2000);//may need a long time
  if(ret == FALSE)
	 	return FALSE;
  
  return TRUE; 
}




static void SMS_GetComingPhonenum(void)
{
  INT8U firstDigitIdx;
  
  for(firstDigitIdx=(g_smsRxInfo.cnt-1); firstDigitIdx>0; firstDigitIdx--)
  {
    if(FLAG86(g_smsRxInfo.buff,firstDigitIdx))                    
    {
      (void)memcpy(g_comingPhonenum, &(g_smsRxInfo.buff[firstDigitIdx]), PHONENUM_LEN);
      break;
    }
  } 
  
  return;
}


static void SMS_GetContent(void)
{
  INT8U idx;

  for(idx=g_smsRxInfo.cnt; idx>0; idx--)
  {
  	if(   g_smsRxInfo.buff[idx-3]=='+' 
  	   && g_smsRxInfo.buff[idx-2]=='3' 
  	   && g_smsRxInfo.buff[idx-1]=='2' 
  	   && g_smsRxInfo.buff[idx-0]=='"')
 		break;
	}
 	g_smsContent.len = 0;
 	idx += 3;
  while('\r' != g_smsRxInfo.buff[idx] && idx<SCI2_RX_BUFFER_LENGTH)
 	{
 	 	g_smsContent.buff[g_smsContent.len] = g_smsRxInfo.buff[idx];
 	 	g_smsContent.len ++;
 	 	idx ++;
  }
}



static BOOLEAN SMS_FindCmd(const INT8U *str)
{
  return (NULL != strstr(g_smsContent.buff,str)); 
}
 



static BOOLEAN SMS_FindNewnumInContent(void)
{
  INT8U firstDigitIdx;
  INT8U numIdx;

  if(g_smsContent.len < PHONENUM_LEN)
  {
    return FALSE;
  }
  
  for(firstDigitIdx=0; firstDigitIdx<g_smsContent.len; firstDigitIdx++)
  {    
    if(FIRST_DIGIT_IS_1(g_smsContent.buff,firstDigitIdx))
    {      
      BOOLEAN lenValid = TRUE;
      for(numIdx=1; numIdx<(PHONENUM_LEN); numIdx++)
      {
        if(!isdigit(g_smsContent.buff[firstDigitIdx+numIdx]))
        {
          lenValid = FALSE;
          break;
        }
      }
      
      if(lenValid)//之所以完全判断完成以后才进行新号码提取，是为了避免污染g_chaningPhonenum
      {
        (void)memcpy(g_chaningPhonenum, &(g_smsContent.buff[firstDigitIdx]), PHONENUM_LEN);
        return TRUE;
      }
    }
  }
  
  return FALSE;
}


static BOOLEAN SMS_DeleteMsg(void)
{
 	BOOLEAN ret;
 	
  SendATCmd(at_cmgd);     
  ret = WaitResponseOK(5000);
  ClearSci2RxBuffer();
  if(ret == FALSE)
	 	return FALSE;
  
  return TRUE;
}



static void SMS_ProcessFull(void)
{
  if(SMS_DELETE_CONDITION(g_smsIndex.buff))
    SMS_DeleteMsg();
}



static void SMS_ConvertCanMsg(INT8U canMsg[])
{
  INT8U dataIdx;
  for(dataIdx=0; dataIdx<8; dataIdx++)
  {
    g_smsResponse[dataIdx*2+0] = '0'+((canMsg[dataIdx]>>4)&0x0F);
    g_smsResponse[dataIdx*2+1] = '0'+((canMsg[dataIdx]>>0)&0x0F);
  }
  g_smsResponse[8*2] = '\0';
}



static void SMS_HandleCmd(void)
{
  if(SMS_FindCmd(cmd_ChangeNum))
  {
    if(SMS_FindNewnumInContent())
    {
      SMS_SetSysPhonenum(g_chaningPhonenum);//(void)memcpy(g_sysPhonenum,g_chaningPhonenum,PHONENUM_LEN);
      if(WritePhoneBook())
      {
        (void)memcpy(g_smsResponse,resp_SysPhonenum,sizeof(resp_SysPhonenum)-1);
        (void)memcpy(g_smsResponse+sizeof(resp_SysPhonenum)-1, g_sysPhonenum, PHONENUM_LEN);
      }
    }
    else
    {
      (void)memcpy(g_smsResponse,resp_InvalidPhonenum,sizeof(resp_InvalidPhonenum)-1);
    }
  }
  else if(SMS_FindCmd(cmd_LockNum))
  {
    (void)memcpy(g_smsResponse,cmd_LockNum,sizeof(cmd_LockNum)-1);
    (void)memcpy(g_smsResponse+sizeof(cmd_LockNum)-1, g_sysPhonenum, PHONENUM_LEN);      
    g_smsResponse[sizeof(cmd_LockNum)-1+PHONENUM_LEN+0+0] = 'O';
    g_smsResponse[sizeof(cmd_LockNum)-1+PHONENUM_LEN+1+0] = 'K';
    g_smsResponse[sizeof(cmd_LockNum)-1+PHONENUM_LEN+1+1] = '\0';
    g_smsResponse[0] = '>';
  }
  else if(SMS_FindCmd(cmd_ReadEcuId))
  {
  }
  else if(SMS_FindCmd(cmd_ReadErrCode))
  {
      SMS_ConvertCanMsg(FC1RxBuff);
  }
  else if(SMS_FindCmd(cmd_ReadState))
  {
      SMS_ConvertCanMsg(SIG1RxBuff);
  }
  else if(SMS_FindCmd(cmd_ReadFuel))
  {
      SMS_ConvertCanMsg(AQ1RxBuff);
  }
  else
  {
    (void)memcpy(g_smsResponse,resp_InvalidContent,sizeof(resp_InvalidContent)-1);
  }
}


/*******************************************************
               SMS extern Functions
********************************************************/

BOOLEAN SMS_Prepare(void)
{
  BOOLEAN ret;
  INT8U cmdIdx;  
  const INT8U * cmdArray_smsPrepare[] = {                        
                        at_cmgf,                
                        at_cnmi,
                        at_cpms,     
                        at_cpbs,  
                      };
                                                   //16 bit MCU
  for(cmdIdx=0; cmdIdx<sizeof(cmdArray_smsPrepare)/sizeof(INT16U); cmdIdx++) 
  {
    SendATCmd(cmdArray_smsPrepare[cmdIdx]);
    ret = WaitResponseOK(500);
    //if(ret == TRUE)//For debug
      ClearSci2RxBuffer();
    if(ret == FALSE)
    {
      g_moudleErrno = ERR_SMS_PREPARE;
      return FALSE;
    }
  } 
 
  SMS_SetSysPhonenum(g_romPhonenum);//(void)memcpy(g_sysPhonenum, g_romPhonenum, PHONENUM_LEN);
  
  #if (READ_PHONEBOOK > 0)
  ret = ReadPhonebook();  
  if(ret)
  {
    SMS_CpyBuff();
    SMS_GetPhonenumInBook();
  }
  #endif
  
  return ret;
}



void SMS_HandleSms(void)
{  
  SMS_CpyBuff();      
  SMS_GetIndex();
  if(SMS_ReadMsg())
  {
    SMS_CpyBuff();
    SMS_GetComingPhonenum();
    if(SMS_ValidateComingPhonenum())
    {
      SMS_GetContent();
      SMS_HandleCmd();   
      (void)SMS_SendMsg(g_smsResponse);
      memset(g_smsContent.buff, 0, g_smsContent.len);
    }
  }
  if(SMS_DELETE_CONDITION(g_smsIndex.buff))
    SMS_DeleteMsg();
  //SMS_ProcessFull();             
}