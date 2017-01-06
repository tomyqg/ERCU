#include "gps.h"


/***************************************************
              GPS variable                                            ***
****************************************************/
volatile struct _GPS_INFO  g_gpsInfo;



/***************************************************
              GPS Functions                                            ***
****************************************************/
void GPS_Init(void)
{
  SCI_OpenCommunication(SCI_MAP_GPS,BR); //GPS 115200
  GPS_INT_ENABLE();
}


void GPS_CpyBuff(void)
{  
  g_gpsInfo.cnt = g_sci1RxInfo.cnt;
  (void)memcpy(g_gpsInfo.buff,g_sci1RxInfo.buff,g_gpsInfo.cnt);
}




