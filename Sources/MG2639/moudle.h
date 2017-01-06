#ifndef __MOUDLE_H_
#define __MOUDLE_H_

#include "includes.h"
#include "SCI.h"
#include "gprs.h"
#include "sys_cfg.h"

/******************************************************
            Moudle Macro and Common Macro
*******************************************************/
#define SCI1_RX_BUFFER_LENGTH 80
#define SCI2_RX_BUFFER_LENGTH 128

#define PHONENUM_LEN 11

typedef enum {
  ERR_MOUDLE_NONE = 0,                
  ERR_MOUDLE_INIT,       
  ERR_SIM_CHECK,         
  ERR_SMS_PREPARE,       
  ERR_GPRS_PREPARE,      
  ERR_GPRS_OPEN,         
  ERR_GPRS_LINK,         
}MOUDLE_RESULT;

/*******************************************************
                    Type Define
********************************************************/
struct _SCI1_RX_INFO
{
  INT8U cnt;
  INT8U buff[SCI1_RX_BUFFER_LENGTH];
};

struct _SCI2_RX_INFO
{
  INT8U cnt;
  INT8U buff[SCI2_RX_BUFFER_LENGTH];
};


/*********************************************************
             Const String Declarations
**********************************************************/
extern const INT8U str_error[];


/*******************************************************
           Moudle Variable Declarations
********************************************************/
extern volatile struct _SCI1_RX_INFO  g_sci1RxInfo;
extern volatile struct _SCI2_RX_INFO  g_sci2RxInfo;
extern MOUDLE_RESULT g_moudleErrno;


//Ã»ÓÐ¿¼ÂÇÐ¡Ð´
#define RECEIVED_SMS(buff,cnt) \
   (buff[cnt-4]=='C')\
&& (buff[cnt-3]=='M')\
&& (buff[cnt-2]=='T')\
//&& (buff[cnt-1]=='I')

/*#define RECEIVED_GPRS() \
   g_sci2RxInfo.buff[g_sci2RxInfo.cnt-4]=='R'\
&& g_sci2RxInfo.buff[g_sci2RxInfo.cnt-3]=='E'\
&& g_sci2RxInfo.buff[g_sci2RxInfo.cnt-2]=='C'\
&& g_sci2RxInfo.buff[g_sci2RxInfo.cnt-1]=='V'\*/
#define RECEIVED_GPRS(buff,cnt) \
   (buff[cnt-4]=='R')\
&& (buff[cnt-3]=='E')\
&& (buff[cnt-2]=='C')\
&& (buff[cnt-1]=='V')\

#define RECEIVED_CHAR10(buff,cnt) \
   (buff[cnt-1]==10)

/*******************************************************
           Moudle Functions Declarations
********************************************************/
extern void Moudle_PowerOn(void);
extern void Moudle_PowerOff(void);
extern void Moudle_PowerOffWithCmd(void);
extern BOOLEAN Moudle_Init(void);
extern void Moudle_SetBaudrate(void);
extern BOOLEAN Moudle_CheckCard(void);
extern BOOLEAN Moudle_CheckReg(void);

extern void SendATCmd(const INT8U cmd[]);
extern void ClearSci2RxBuffer(void);
extern BOOLEAN WaitResponseOK(INT16U t);
extern BOOLEAN WaitResponseChrStr(const INT8U chrSuccess, 
                     const INT8U *strFailed, 
                     INT16U waitTime);
extern BOOLEAN WaitResponseStrStr(const INT8U *strSuccess, 
                     const INT8U *strFailed, 
                     INT16U waitTime);
extern INT8U CountChecksum(INT8U index, INT8U length, INT8U data[]);
extern void SendLengthDigit(INT16U length);














































































#endif /*__MG2639_H_*/