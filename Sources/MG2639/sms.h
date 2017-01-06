#ifndef __SMS_H_
#define __SMS_H_

#include "includes.h"
#include "SCI.h"
#include "moudle.h"
#include "sys_cfg.h"

/*******************************************************
                    SMS Macro
********************************************************/
#define SMS_RX_BUFFER_LENGTH      SCI2_RX_BUFFER_LENGTH
#define SMS_CONTENT_LEN           64
#define SMS_INDEX_LEN_MAX         2

#define WAIT_TIME_SMS_SEND_TOTAL  2000

#define READ_BOOK_OK(buff,cpbrIdx) \
      buff[cpbrIdx+0]=='+'\
   && buff[cpbrIdx+1]=='C'\
   && buff[cpbrIdx+2]=='P'\
   && buff[cpbrIdx+3]=='B'\
   && buff[cpbrIdx+4]=='R'

#define FLAG86(buff,firstNumIdx) \
      buff[firstNumIdx-3]=='+'\
   && buff[firstNumIdx-2]=='8'\
   && buff[firstNumIdx-1]=='6'\

#define FIRST_DIGIT_IS_1(buff,firstDigitIdx) (buff[firstDigitIdx]=='1')

#define SMS_DELETE_CONDITION(buff) ('7' <= buff[0])

/*******************************************************
                    SMS Type Define
********************************************************/
struct _SMS_RX_INFO
{
  INT8U cnt;
  INT8U buff[SMS_RX_BUFFER_LENGTH];
};

struct _SMS_INDEX
{
  INT8U len;
  INT8U buff[SMS_INDEX_LEN_MAX];
};

struct _SMS_CONTENT
{
  INT8U len;
  INT8U buff[SMS_CONTENT_LEN];
};
 

/*******************************************************
               SMS Functions Declarations
********************************************************/
extern BOOLEAN SMS_Prepare(void);
extern void SMS_HandleSms(void);
































#endif/*__SMS_H_*/