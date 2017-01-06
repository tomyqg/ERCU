/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                          (c) Copyright 1992-2006, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
* Version    : V2.83
* Description: 测试uCOS-ii的内核
*            : 适用芯片mc9s12g128
* Note(s)    : 最后修改日期 11-12-12
*            : 适用芯片mc9s12g128
*********************************************************************************************************
********* Modified 2014.4.21 xyl*************************************************************************
*/

#include <hidef.h>           /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "common.h"          /* 总头文件*/

/*
*********************************************************************************************************
*                                           用户自己任务的配置
*********************************************************************************************************
*/
#define TASK_START_PRIO                   5
#define TASK_START_STKSIZE                0x40

#define TASK_MOUDLE_PRIO                  6
#define TASK_MOUDLE_STKSIZE               0x60
        
#define TASK_SMS_PRIO                     29        
#define TASK_SMS_STKSIZE                  0x80

#define TASK_GPRS_RECV_PRIO               10        
#define TASK_GPRS_RECV_STKSIZE            0x80 

#define TASK_GPRS_CMD_PRIO                13        
#define TASK_GPRS_CMD_STKSIZE             0x80 

#define TASK_GPRS_HEARTBEAT_PRIO          19        
#define TASK_GPRS_HEARTBEAT_STKSIZE       0x80

#define TASK_GPRS_SEND_PRIO               14        
#define TASK_GPRS_SEND_STKSIZE            0x80 
     
#define TASK_GPS_PRIO                     37
#define TASK_GPS_STKSIZE                  0x80

#define TASK_SD_PRIO                      20
#define TASK_SD_STKSIZE                   0x100

#define TASK_CAN_PRIO                     16
#define TASK_CAN_STKSIZE                  0x80

#define TASK_POWER_MONITOR_PRIO           47
#define TASK_POWER_MONITOR_STKSIZE        0x80


#define TASK_STKCHK_PRIO           60
#define TASK_STKCHK_STKSIZE        0x40

/*
*********************************************************************************************************
*                                            定义自己的任务堆栈
*********************************************************************************************************
*/
static OS_STK Stack_Start[TASK_START_STKSIZE];
static INT8U Stack_Moudle[TASK_MOUDLE_STKSIZE];    
static INT8U Stack_SMS[TASK_SMS_STKSIZE];
static INT8U Stack_GPRSRecv[TASK_GPRS_RECV_STKSIZE];
static INT8U Stack_GPRSCmdHandle[TASK_GPRS_CMD_STKSIZE];
static INT8U Stack_GPRSSend[TASK_GPRS_SEND_STKSIZE];
static INT8U Stack_GPRSHb[TASK_GPRS_HEARTBEAT_STKSIZE];
static INT8U Stack_SD[TASK_SD_STKSIZE];
static INT8U Stack_GPS[TASK_GPS_STKSIZE];      
static INT8U Stack_CAN[TASK_CAN_STKSIZE]; 
static INT8U Stack_PowerMonitor[TASK_POWER_MONITOR_STKSIZE];
static OS_STK Stack_StkChk[TASK_STKCHK_STKSIZE];
 
/*
*********************************************************************************************************
*                                          声明使用到的任务函数
*********************************************************************************************************
*/
void Task_Start(void*pdata);//once
void Task_Moudle(void*pdata);//once
void Task_SMS(void*pdata);//event
void Task_GPRSRecv(void*pdata);//event
void Task_GPRSCmdHandle(void*pdata);//event,
void Task_GPRSSend(void*pdata);//event,
void Task_GPRSHeartbeat(void*pdata);//cyclic,HEART_BEAT_INTERVAL
void Task_SD(void*pdata);//cyclic,30s
void Task_GPS(void*pdata);//cyclic,15s
void Task_CAN(void*pdata);//cyclic,1000ms
void Task_PowerMonitor(void*pdata);//cyclic,100ms
void Task_StkChk(void*pdata);//cyclic,30s


static void PowerOnReadData(void);
static void PowerOffStoreData(void);
static BOOLEAN T15OffMonitor(void);

/*
************************************************
          STA 各任务状态
************************************************
*/
typedef enum 
{
  TASK_INDEX_IDLE = 0,
  TASK_INDEX_SMS,
  TASK_INDEX_GPRSRecv,  
  TASK_INDEX_GPRSCmdHandle,  
  TASK_INDEX_GPRSSend,  
  TASK_INDEX_GPRSHeartbeat,  
  TASK_INDEX_SD,  
  TASK_INDEX_GPS,  
  TASK_INDEX_CAN,  
  TASK_INDEX_PowerMonitor,  
  TASK_INDEX_N_NUM,  
}_TASK_INDEX;
static _TASK_INDEX g_runningTaskIndex = TASK_INDEX_IDLE;

typedef enum 
{
  TASK_STATE_INIT = 0,
  TASK_STATE_RUNNING,
  TASK_STATE_DELAY_WAITING,
  TASK_STATE_SEM_PEND,
  TASK_STATE_SEM_POST,
  TASK_STATE_SEM_RECVIVED,
  TASK_STATE_Q_PEND,
  TASK_STATE_Q_POST,
  TASK_STATE_Q_RECVIVED,
  TASK_STATE_SEND_HB,
  TASK_STATE_CHECK_HB,
  TASK_STATE_SD_WRITE,
  TASK_STATE_SD_SYNC,
  TASK_STATE_T15OFF,
  TASK_STATE_AFTERRUN,
  TASK_STATE_BATOFF,
}_TASK_STATE;

static _TASK_STATE g_stateSMS = TASK_STATE_INIT;
static _TASK_STATE g_stateGPRSRecv = TASK_STATE_INIT;
static _TASK_STATE g_stateGPRSCmdHandle = TASK_STATE_INIT;
static _TASK_STATE g_stateGPRSSend = TASK_STATE_INIT;
static _TASK_STATE g_stateGPRSHeartbeat = TASK_STATE_INIT;
static _TASK_STATE g_stateSD = TASK_STATE_INIT;
static _TASK_STATE g_stateGPS = TASK_STATE_INIT;
static _TASK_STATE g_stateCAN = TASK_STATE_INIT;
static _TASK_STATE g_statePowerMonitor = TASK_STATE_INIT;

/*
************************************************
          信号量SEM
************************************************
*/
OS_EVENT *SEM_SMS;
OS_EVENT *SEM_EcuResp;
OS_EVENT *SEM_GPRSRecv;
OS_EVENT *SEM_GPRSSend;
OS_EVENT *SEM_GPS;
OS_EVENT *Q_GPRSCmd;//OS_Q DATA_Msg;

#define N_MESSAGES    4 
static void *g_gprsQueueMsgPtrGrp[N_MESSAGES];    //定义消息指针数组
static INT8U g_gprsMsgArray[N_MESSAGES][RESP_TO_SERVER_TOTAL_LEN];
static INT8U g_gprsMsgArray_Index; 


static void App_CreateObj(void)
{
  SEM_SMS = OSSemCreate(0);
  SEM_GPRSRecv = OSSemCreate(0);
  SEM_EcuResp = OSSemCreate(0);
  SEM_GPRSSend = OSSemCreate(0);
  SEM_GPS = OSSemCreate(0);
  Q_GPRSCmd = OSQCreate(&g_gprsQueueMsgPtrGrp[0],N_MESSAGES);                //创建消息队列
}

/*
************************************************
     main
************************************************
*/
void main(void) 
{
  DisableInterrupts; 
    
  DEV_Init();

  OSInit();
  App_CreateObj();  
  RTI_Init();
  EnableInterrupts;      
  //(void)OSTaskCreate(Task_Start,(void *)0,&Stack_Start[TASK_START_STKSIZE-1],TASK_START_PRIO); 
  (void)OSTaskCreateExt(Task_Start, 
                        (void *)0,
                        &Stack_Start[TASK_START_STKSIZE-1],
                        TASK_START_PRIO,
                        TASK_START_PRIO,
                        Stack_Start,
                        TASK_START_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
                        );  
 
  OSStart();
}



/*
************************************************
     Start Task
************************************************
*/
void Task_Start(void*pdata)
{
  (void)pdata;
  
  //OSStatInit();

  //(void)OSTaskCreate(Task_StkChk, (void *)0,&Stack_StkChk[TASK_STKCHK_STKSIZE-1],TASK_STKCHK_PRIO);  
  (void)OSTaskCreateExt(Task_StkChk, 
                        (void *)0,
                        &Stack_StkChk[TASK_STKCHK_STKSIZE-1],
                        TASK_STKCHK_PRIO,
                        TASK_STKCHK_PRIO,
                        Stack_StkChk,
                        TASK_STKCHK_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
                        );  

  //(void)OSTaskCreate(Task_Moudle, (void *)0,&Stack_Moudle[TASK_MOUDLE_STKSIZE-1],TASK_MOUDLE_PRIO);  
  (void)OSTaskCreateExt(Task_Moudle, 
                        (void *)0,
                        &Stack_Moudle[TASK_MOUDLE_STKSIZE-1],
                        TASK_MOUDLE_PRIO,
                        TASK_MOUDLE_PRIO,
                        Stack_Moudle,
                        TASK_MOUDLE_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
                        );  
  (void)OSTaskCreateExt(Task_SD, 
                        (void *)0,
                        &Stack_SD[TASK_SD_STKSIZE-1],
                        TASK_SD_PRIO,
                        TASK_SD_PRIO,
                        Stack_SD,
                        TASK_SD_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
                        );  
  (void)OSTaskCreateExt(Task_CAN, 
                        (void *)0,
                        &Stack_CAN[TASK_CAN_STKSIZE-1],
                        TASK_CAN_PRIO,
                        TASK_CAN_PRIO,
                        Stack_CAN,
                        TASK_CAN_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
                        );
  (void)OSTaskCreateExt(Task_PowerMonitor, 
                        (void *)0,
                        &Stack_PowerMonitor[TASK_POWER_MONITOR_STKSIZE-1],
                        TASK_POWER_MONITOR_PRIO,
                        TASK_POWER_MONITOR_PRIO,
                        Stack_PowerMonitor,
                        TASK_POWER_MONITOR_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
                        );
  
  //RTI_Init();
  (void)OSTaskDel(OS_PRIO_SELF); 
}

/*
************************************************
     Moudle Task
************************************************
*/
void Task_Moudle(void*pdata)
{ 
  (void)pdata;
      
  Moudle_PowerOn();
  (void)OSTimeDlyHMSM(0,0,10,0);//better more than 10s  
  Moudle_SetBaudrate();
  (void)Moudle_Init();
  if(!Moudle_CheckCard())
  {
    (void)Moudle_PowerOffWithCmd();
    (void)OSTaskDel(OS_PRIO_SELF); 
  }
  (void)SMS_Prepare();  
  GPRS_TryLink();  
  
  (void)OSTaskCreateExt(Task_GPRSHeartbeat, 
                        (void *)0,
                        &Stack_GPRSHb[TASK_GPRS_HEARTBEAT_STKSIZE-1],
                        TASK_GPRS_HEARTBEAT_PRIO, 
                        TASK_GPRS_HEARTBEAT_PRIO, 
                        Stack_GPRSHb,
                        TASK_GPRS_HEARTBEAT_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
                        );
  (void)OSTaskCreateExt(Task_GPRSRecv, 
                        (void *)0,
                        &Stack_GPRSRecv[TASK_GPRS_RECV_STKSIZE-1],
                        TASK_GPRS_RECV_PRIO,
                        TASK_GPRS_RECV_PRIO,
                        Stack_GPRSRecv,
                        TASK_GPRS_RECV_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
                        );
  (void)OSTaskCreateExt(Task_GPRSCmdHandle, 
                        (void *)0,
                        &Stack_GPRSCmdHandle[TASK_GPRS_CMD_STKSIZE-1],
                        TASK_GPRS_CMD_PRIO,
                        TASK_GPRS_CMD_PRIO,
                        Stack_GPRSCmdHandle,
                        TASK_GPRS_CMD_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
                        );
  (void)OSTaskCreateExt(Task_GPRSSend, 
                        (void *)0,
                        &Stack_GPRSSend[TASK_GPRS_SEND_STKSIZE-1],
                        TASK_GPRS_SEND_PRIO,
                        TASK_GPRS_SEND_PRIO,
                        Stack_GPRSSend,
                        TASK_GPRS_SEND_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
                        );    
  (void)OSTaskCreateExt(Task_SMS, 
                        (void *)0,
                        &Stack_SMS[TASK_SMS_STKSIZE-1],
                        TASK_SMS_PRIO,
                        TASK_SMS_PRIO,
                        Stack_SMS,
                        TASK_SMS_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
                        );  
  (void)OSTaskCreateExt(Task_GPS, 
                        (void *)0,
                        &Stack_GPS[TASK_GPS_STKSIZE-1],
                        TASK_GPS_PRIO,
                        TASK_GPS_PRIO,
                        Stack_GPS,
                        TASK_GPS_STKSIZE,
                        (void *)0,
                        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR                     
                        );  

  (void)OSTaskDel(OS_PRIO_SELF);  
}





/*
*********************************************************************************************************
              SMS Task
*********************************************************************************************************
*/
void Task_SMS(void*pdata)
{
  INT8U err;
  
  (void)pdata;
  
  while(1)
  {
    g_runningTaskIndex = TASK_INDEX_SMS;
    g_stateSMS = TASK_STATE_SEM_PEND;
    OSSemPend(SEM_SMS,0,&err);
    g_stateSMS = TASK_STATE_SEM_RECVIVED;
    SMS_HandleSms();
  }
}



/*
*********************************************************************************************************
*                                          GPRS Recv Task
*********************************************************************************************************
*/
void Task_GPRSRecv(void*pdata)
{
  INT8U err;

  (void)pdata;
  
  while(1)
  {
    g_runningTaskIndex = TASK_INDEX_GPRSRecv;
    g_stateGPRSRecv = TASK_STATE_SEM_PEND;
    OSSemPend(SEM_GPRSRecv,0,&err);
    g_stateGPRSRecv = TASK_STATE_SEM_RECVIVED;
    GPRS_Reset_ConnectTime();
    GPRS_Reset_hbErrcnt();
    GPRS_GetContent();
    (void)memcpy(g_gprsMsgArray[g_gprsMsgArray_Index], g_gprsContent.buff, RESP_TO_SERVER_TOTAL_LEN);
    g_stateGPRSRecv = TASK_STATE_Q_POST;
    (void)OSQPost(Q_GPRSCmd,*(g_gprsMsgArray + g_gprsMsgArray_Index));
    g_gprsMsgArray_Index++;  
    g_gprsMsgArray_Index %= N_MESSAGES;
  }
}

/*
*********************************************************************************************************
*                                          GPRS CmdHandle
*********************************************************************************************************
*/
void Task_GPRSCmdHandle(void*pdata)
{
  INT8U err;
  INT8U *Q_dataPtr;
  
  (void)pdata;
  
  while(1)
  {
      g_runningTaskIndex = TASK_INDEX_GPRSCmdHandle;
      g_stateGPRSCmdHandle = TASK_STATE_Q_PEND;
      Q_dataPtr = OSQPend(Q_GPRSCmd,0,&err); 
      g_stateGPRSCmdHandle = TASK_STATE_Q_RECVIVED;
      (void)memcpy(g_gprsCmd, Q_dataPtr, RESP_TO_SERVER_TOTAL_LEN);
      GPRS_HandleCmd();
      //OSSemPend(SEM_EcuResp,OS_TICKS_PER_SEC/2,&err);//remove for positive response
      //处理所有错误 理论上只有OS_TIMEOUT
      //如果没有错误 按照ENC8RespMsgDecode()里填充的数据发送
      /*if(OS_NO_ERR != err)
      {
        GPRS_FillTimeoutResp();
      }*/
      
      g_stateGPRSCmdHandle = TASK_STATE_SEM_POST;      
      (void)OSSemPost(SEM_GPRSSend);
  }  
}

/*
*********************************************************************************************************
*                                          GPRS Send Task
*********************************************************************************************************
*/
void Task_GPRSSend(void*pdata)
{
  INT8U err;
  (void)pdata;
  
  while(1)
  {    
    g_runningTaskIndex = TASK_INDEX_GPRSSend;
    g_stateGPRSSend = TASK_STATE_SEM_PEND;
    OSSemPend(SEM_GPRSSend,0,&err);
    g_stateGPRSSend = TASK_STATE_SEM_RECVIVED;
    GPRS_SendResp();
  }
}

/*
*********************************************************************************************************
*                                          GPRS HeartBeat Task
*********************************************************************************************************
*/

void Task_GPRSHeartbeat(void*pdata)
{
  (void)pdata;
  
  while(1)
  {
    
    g_runningTaskIndex = TASK_INDEX_GPRSHeartbeat;
    g_stateGPRSHeartbeat = TASK_STATE_SEND_HB;
    GPRS_SendHeartbeat();
    g_stateGPRSHeartbeat = TASK_STATE_CHECK_HB;
    GPRS_CheckHeartbeat();
    g_stateGPRSHeartbeat = TASK_STATE_DELAY_WAITING;
    (void)OSTimeDlyHMSM(0,0,HEART_BEAT_INTERVAL,0); 
  }
}



/*
***********************************************************
           SD Task
***********************************************************
*/
//"0:\\Log.txt" 只有一个驱动器 前面可以不加驱动器号
const BYTE FILE_NAME[] = "\\TINY2.txt"; //GPSLog len<= 12 litter or equal to 12
typedef enum
{
 ERR_FF_NONE = 0,
 ERR_FF_DISK_INIT,
 ERR_FF_MOUNT,     
 ERR_FF_OPEN,      
 ERR_FF_STAT,      
 ERR_FF_SEEK,      
 ERR_FF_WRITE,       
 ERR_FF_SYNC,      
}FF_RESULT;

FRESULT g_ffErrno;
FF_RESULT g_ffErrkind;

FATFS g_SDFatFs;				                                            
FIL g_FileGPSLog;



void Task_SD(void*pdata)
{ 
  UINT16 bytesWritten;
  FILINFO FileInfo;
  
  (void)pdata;  
  if(disk_initialize(0))
  {
    g_ffErrkind = ERR_FF_DISK_INIT;
    (void)OSTaskDel(OS_PRIO_SELF); 
  }
  disk_get_total_sectors(g_SDFatFs.win);
  
  g_FileGPSLog.fs = &g_SDFatFs;

  if(g_ffErrno=f_mount((BYTE)0, &g_SDFatFs))
  {
    g_ffErrkind = ERR_FF_MOUNT;
    (void)OSTaskDel(OS_PRIO_SELF);
  }
  
  DisableInterrupts;
  if(g_ffErrno=f_open(&g_FileGPSLog, FILE_NAME, FA_WRITE|FA_READ|FA_OPEN_EXISTING))
  {
    if(g_ffErrno=f_open(&g_FileGPSLog, FILE_NAME, FA_WRITE|FA_READ|FA_CREATE_ALWAYS))
    {
      g_ffErrkind = ERR_FF_OPEN;
      (void)OSTaskDel(OS_PRIO_SELF);
    }
  }
  EnableInterrupts;

  while(1)
  {  
    g_runningTaskIndex = TASK_INDEX_SD;
    DisableInterrupts;
    if(g_ffErrno=f_stat(FILE_NAME, &FileInfo))
      g_ffErrkind = ERR_FF_STAT;
    EnableInterrupts;
    DisableInterrupts;
    if(g_ffErrno=f_lseek(&g_FileGPSLog, FileInfo.fsize))
      g_ffErrkind = ERR_FF_SEEK;
    EnableInterrupts;
    if(ERR_FF_SEEK != g_ffErrkind)
    {
      DisableInterrupts;
      g_stateSD = TASK_STATE_SD_WRITE;
      if(g_ffErrno=f_write(&g_FileGPSLog,g_gpsInfo.buff,g_gpsInfo.cnt,&bytesWritten))
        g_ffErrkind = ERR_FF_WRITE;
      EnableInterrupts;
      DisableInterrupts;
      g_stateSD = TASK_STATE_SD_SYNC;
      if(g_ffErrno=f_sync(&g_FileGPSLog))
        g_ffErrkind = ERR_FF_SYNC;
      EnableInterrupts;
    }    
     
    g_stateSD = TASK_STATE_DELAY_WAITING;
    (void)OSTimeDlyHMSM(0,0,30,0); 
  }
}





/*
********************************************************
*        GPS Task
********************************************************
*/
void Task_GPS(void*pdata)
{
  INT8U err;
  (void)pdata;
  GPS_Init();

  while(1)
  {
    g_runningTaskIndex = TASK_INDEX_GPS;
    g_stateGPS = TASK_STATE_SEM_PEND;
    OSSemPend(SEM_GPS,0,&err);
    g_stateGPS = TASK_STATE_SEM_RECVIVED;
    OSSemSet(SEM_GPS,0,&err);
    GPS_CpyBuff();
    g_stateGPS = TASK_STATE_DELAY_WAITING;
    (void)OSTimeDlyHMSM(0,0,HEART_BEAT_INTERVAL-1,0); 
    GPS_INT_ENABLE();
  }
}


 
/*
*********************************************************************************************************
*                                          CAN Task
*********************************************************************************************************
*/ 
INT32U g_ERCU_m_ct_Running;
void Task_CAN(void*pdata)
{
  (void)pdata;
  (void)CAN_Init();
  #if  DIAG_ENABLE > 0
  Diag_InitTP();
  #endif

  while(1)
  {
     g_runningTaskIndex = TASK_INDEX_CAN;
     g_stateCAN = TASK_STATE_RUNNING;
     LED1_D2_TOOGLE();
     g_ERCU_m_ct_Running++;
     GPRS_Increase_ConnectTime();
     CAN_PeriodHandle(TRUE);//TX 
     g_stateCAN = TASK_STATE_DELAY_WAITING;
     (void)OSTimeDlyHMSM(0,0,1,0);//发送在线报文，周期1s
  }
}

 
/************************************************************
  When Power On FindFreeSpace and Read data stored 
*************************************************************/ 
const UINT32 g_eeActiveFlag = 0xFACF0000;
UINT8 g_eeActiveSectorNum;
INT16U g_eeFreeSpaceStart;

static void PowerOnReadData(void)
{
  (void)FlashInit(PSSDConfig);
  g_eeActiveSectorNum = FindActiveUserSector();
  if(NO_ACTIVE_USER_SECTOR==g_eeActiveSectorNum)
  {
    //first time, let it pass                          
  }
  else
  {
    g_eeFreeSpaceStart = FindFreeSpace(g_eeActiveSectorNum);
    g_ERCU_m_ct_Running = READ32(g_eeFreeSpaceStart-sizeof(g_ERCU_m_ct_Running));
  }  
}





/*
*********************************************************************************************************
           Store data when power down
*********************************************************************************************************
*/ 
static void PowerOffStoreData(void)
{
  DisableInterrupts;
   
  if(NO_ACTIVE_USER_SECTOR==g_eeActiveSectorNum)//after program been flashed into MCU,the first time to  store data
  {      
    (void)EEProgram(PSSDConfig, \
                             EE_START_ADDR+sizeof(g_ERCU_m_ct_Running), \
                             sizeof(g_ERCU_m_ct_Running), \
                             (UINT32)&g_ERCU_m_ct_Running, \
                             FlashCommandSequence);
    (void)EEProgram(PSSDConfig, \
                             EE_START_ADDR, \
                             sizeof(g_eeActiveFlag), \
                             (UINT32)&g_eeActiveFlag, \
                             FlashCommandSequence);                                    
  }
  else
  {
    if(g_eeFreeSpaceStart>=(EE_START_ADDR+USER_DEF_SECTOR_SIZE+g_eeActiveSectorNum*USER_DEF_SECTOR_SIZE))//This sector have been full filled
    {
      g_eeFreeSpaceStart += sizeof(g_ERCU_m_ct_Running);
      if((USER_DEF_SECTOR_NUM-1)==g_eeActiveSectorNum)
      {
        g_eeFreeSpaceStart = EE_START_ADDR+sizeof(g_ERCU_m_ct_Running);
      }
      
      (void)EEProgram(PSSDConfig, \
                             g_eeFreeSpaceStart, \
                             sizeof(g_ERCU_m_ct_Running), \
                             (UINT32)&g_ERCU_m_ct_Running, \
                             FlashCommandSequence);
      (void)EEProgram(PSSDConfig, \
                             g_eeFreeSpaceStart-sizeof(g_ERCU_m_ct_Running), \
                             sizeof(g_eeActiveFlag), \
                             (UINT32)&g_eeActiveFlag, \
                             FlashCommandSequence);
      //After finishing the action(afterrun store data), Erase other sector(s)                       
      (void)EEErase(PSSDConfig, EE_START_ADDR+g_eeActiveSectorNum*USER_DEF_SECTOR_SIZE, USER_DEF_SECTOR_SIZE, FlashCommandSequence);
      (void)EEEraseVerify(PSSDConfig, g_eeFreeSpaceStart+g_eeActiveSectorNum*USER_DEF_SECTOR_SIZE, USER_DEF_SECTOR_SIZE, FlashCommandSequence);
    }
    else
    {
      (void)EEProgram(PSSDConfig, \
                               g_eeFreeSpaceStart, \
                               sizeof(g_ERCU_m_ct_Running), \
                               (UINT32)&g_ERCU_m_ct_Running, \
                               FlashCommandSequence);
     }
   }
  EnableInterrupts;
}


static BOOLEAN T15OffMonitor(void)
{
  INT16U T15VoltValue;
  static INT8U offDebounceCnt;
  
  if(   IO_ERR_OK==ADC_ReadChannel(AN_CHANNEL_0, &T15VoltValue)
     && T15VoltValue<T15_THREHOLD_ADC_VALUE)
  {     
    offDebounceCnt++;
  }
  else
  {
    offDebounceCnt = 0;
  }

  if(offDebounceCnt>=3)
  {
    offDebounceCnt = 0;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}


/*
*******************************************************
     PowerMonitor Task
*******************************************************
*/
void Task_PowerMonitor(void*pdata)
{
  (void)pdata;
  PowerOnReadData();
  ADC_Init();
  
  while(1)
  {
     g_runningTaskIndex = TASK_INDEX_PowerMonitor;
     g_statePowerMonitor = TASK_STATE_RUNNING;
     if(T15OffMonitor())
     {
        g_statePowerMonitor = TASK_STATE_T15OFF;
        DisableInterrupts;
        (void)Moudle_PowerOffWithCmd();
        g_statePowerMonitor = TASK_STATE_AFTERRUN;
        PowerOffStoreData();
        g_statePowerMonitor = TASK_STATE_BATOFF;
        BAT_OFF();
     }
     g_statePowerMonitor = TASK_STATE_DELAY_WAITING;
     (void)OSTimeDly(2);
  }
}

/*
*******************************************************
     Check The stack of All Task
*******************************************************
*/
OS_STK_DATA g_stk_CAN;
OS_STK_DATA g_stk_GPRSCmdHandle;
OS_STK_DATA g_stk_GPRSHeartbeat;
OS_STK_DATA g_stk_GPRSRecv;
OS_STK_DATA g_stk_GPRSSend;
OS_STK_DATA g_stk_GPS;
OS_STK_DATA g_stk_PowerMonitor;
OS_STK_DATA g_stk_SD;
OS_STK_DATA g_stk_SMS;

void Task_StkChk(void*pdata)
{
  (void)pdata;
  
  while(1)
  {
    OSTaskStkChk(TASK_CAN_PRIO, &g_stk_CAN);
    OSTaskStkChk(TASK_GPRS_CMD_PRIO, &g_stk_GPRSCmdHandle);
    OSTaskStkChk(TASK_GPRS_HEARTBEAT_PRIO, &g_stk_GPRSHeartbeat);
    OSTaskStkChk(TASK_GPRS_RECV_PRIO, &g_stk_GPRSRecv);
    OSTaskStkChk(TASK_GPRS_SEND_PRIO, &g_stk_GPRSSend);
    OSTaskStkChk(TASK_GPS_PRIO, &g_stk_GPS);
    OSTaskStkChk(TASK_POWER_MONITOR_PRIO, &g_stk_PowerMonitor);
    OSTaskStkChk(TASK_SD_PRIO, &g_stk_SD);
    OSTaskStkChk(TASK_SMS_PRIO, &g_stk_SMS);
    (void)OSTimeDlyHMSM(0,0,30,0);
  }
}