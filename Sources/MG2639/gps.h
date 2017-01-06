#ifndef __GPS_H_
#define __GPS_H_

#include "includes.h"
#include "moudle.h"
#include "SCI.h"
#include "sys_cfg.h"


/****************************************************
        GPS Macro                                                
****************************************************/
#define GPS_BUFFER_LEN      80
#define GPS_INT_ENABLE_TIME 1//unit(s)

#define GPS_INT_DISABLE() SCI1_INT_DISABLE()
#define GPS_INT_ENABLE()  SCI1_INT_ENABLE()

/*******************************************************
                    Type Define
********************************************************/
struct _GPS_INFO
{
  INT8U cnt;
  INT8U buff[GPS_BUFFER_LEN];
};


/****************************************************
        GPS Variable Declarations                                                
****************************************************/
extern volatile struct _GPS_INFO  g_gpsInfo;


/****************************************************
        GPS Functions Declarations                                                
****************************************************/
extern void GPS_Init(void);
extern void GPS_CpyBuff(void);












































#endif/*__GPS_H_*/