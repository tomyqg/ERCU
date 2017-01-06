#ifndef __GPRS_H_
#define __GPRS_H_

#include "moudle.h"
#include <string.h>
#include "includes.h"
#include "gps.h"
#include "sys_cfg.h"

/********************************************************
                        GPRS Macro                                                
*********************************************************/
typedef enum
{
 TCP = 1, 
 UDP,   
}GPRS_TP_TYPE;


#define N_LINK_CHAN  4

typedef enum
{
 LINK_STS_TO_LINK = 0,
 LINK_STS_LINKING,
 LINK_STS_OK,     
 LINK_STS_FAILED,  
}LINK_STS_TYPE;


#define FIXED_LENGTH          9
#define GPRS_TX_DATA_LEN      100
#define HEART_BEAT_DATA_LEN   2

#define GPRS_CONTENT_MAX_LENGTH  100
#define MAX_NUM_OF_LEN_IDX  4


#define HEART_BEAT_ERR_MAX    5
#define HEART_BEAT_INTERVAL   15//s 3min不同运营商、不同地方，不一样，TCP UDP也不同
//典型的，也有说最大 UDP 4min   TCP 9min
//时间不是最主要的问题 

#define WAIT_TIME_GPRS_OPEN_TOTAL         1500
#define WAIT_TIME_GPRS_OPEN_EACH          100
#define WAIT_TIME_GET_PPP_STATUS_TOTAL    1000//ms
#define WAIT_TIME_GET_PPP_STATUS_EACH     100//ms

#define WAIT_TIME_UDP_LINK_TOTAL          10000
#define WAIT_TIME_TCP_LINK_TOTAL          10000
#define WAIT_TIME_LINK_EACH               100

#define WAIT_TIME_GET_UDP_STATUS_TOTAL    2000
#define WAIT_TIME_GET_TCP_STATUS_TOTAL    2000
#define WAIT_TIME_GET_STATUS_EACH         100

#define WAIT_TIME_UDP_SEND_TOTAL          1000
#define WAIT_TIME_TCP_SEND_TOTAL          2000
//#define WAIT_TIME_SEND_EACH               100

#define COMMA_NUM_BEFORE_LENGTH 1

#define GPRS_RX_BUFFER_LENGTH 128

/*********************************************************
        ERCU-SERVER Communication Protocol Macro                                               
**********************************************************/
//长度定义
#define BYTES_ADDRESS_SERVER 1
#define BYTES_ADDRESS_ERCU   1
#define BYTES_ADDRESS_CTRLPC 1
#define BYTES_DEVICE_ID      4
#define BYTES_SERVICE        1
#define BYTES_PARM           2
#define BYTES_GPSINFO        100
#define BYTES_CHECKSUM       1

//心跳包总长度
#define DATA_START_BYTE_NO            (BYTES_ADDRESS_SERVER+BYTES_ADDRESS_ERCU+BYTES_DEVICE_ID+BYTES_SERVICE+BYTES_PARM)  
#define HEARTBEAT_PARM_LEN            (8+8+8+BYTES_GPSINFO)
#define HEARTBEAT_TOTAL_LEN           (DATA_START_BYTE_NO+HEARTBEAT_PARM_LEN+BYTES_CHECKSUM)

//节点地址
#define ADDRESS_SERVER  0x00
#define ADDRESS_ERCU    0x01
#define ADDRESS_CTRLPC  0x02

//设备ID定义
#define DEVICE_ID0      0xD2
#define DEVICE_ID1      0x04
#define DEVICE_ID2      0x00
#define DEVICE_ID3      0x00


//服务标识定义
#define SER_ERCU_SERVER_REGISTER    0x11
#define SER_CTRLPC_SERVER_READ      0x12
#define SER_CTRLPC_ERCU_ENABLE      0x13
#define SER_CTRLPC_ERCU_VERIFY      0x14


#define ADDR_MATCH(buff) \
   (ADDRESS_ERCU == buff[0])\
&& (ADDRESS_CTRLPC == buff[1])\

#define DEV_MATCH(buff) \
   (DEVICE_ID0 == buff[2])\
&& (DEVICE_ID1 == buff[3])\
&& (DEVICE_ID2 == buff[4])\
&& (DEVICE_ID3 == buff[5])\

#define IS_SERVICE(service,buff) (service==buff[6])

#define RESP_TO_SERVER_PARM_LEN     4
#define RESP_TO_SERVER_TOTAL_LEN    (DATA_START_BYTE_NO+RESP_TO_SERVER_PARM_LEN+BYTES_CHECKSUM)


/*******************************************************
                    GPRS Type Define
********************************************************/
struct _GPRS_RX_INFO
{
  INT8U cnt;
  INT8U buff[GPRS_RX_BUFFER_LENGTH];
};

struct _GPRS_CONTENT
{
  INT8U len;
  INT8U buff[GPRS_CONTENT_MAX_LENGTH];
};

struct _GPRS_LINK_INFO
{
  LINK_STS_TYPE status;	   
  GPRS_TP_TYPE tp;
  INT8U channel;
  INT8U hbErrcnt;
};



/********************************************************
         GPRS Variable Declarations                                                 ***
*********************************************************/
extern volatile struct _GPRS_CONTENT  g_gprsContent;
extern volatile INT8U g_gprsCmd[RESP_TO_SERVER_TOTAL_LEN];
extern volatile INT8U g_heartbeatPacket[HEARTBEAT_TOTAL_LEN];
extern volatile INT8U g_serviceCtrlpcToErcu;

/*****************************************************
               GPRS Functions Declarations                                     ***
******************************************************/
extern void GPRS_TryLink(void);

extern void GPRS_SendResp(void);

extern void GPRS_Reset_hbErrcnt(void);
extern void GPRS_Increase_hbErrcnt(void);
extern void GPRS_Reset_ConnectTime(void);
extern void GPRS_Increase_ConnectTime(void);

extern void GPRS_SendHeartbeat(void);
extern void GPRS_CheckLinkSts(void);
extern void GPRS_CheckHeartbeat(void);

extern void GPRS_GetContent(void);
extern void GPRS_HandleCmd(void);

extern void GPRS_FillSeedInResponse(INT32U seedFromEcu);
extern void GPRS_FillPostiveResp(void);
extern void GPRS_FillNegRespSecurityAccess(INT8U subFunc);
extern void GPRS_FillNegRespService31(void);
extern void GPRS_FillTimeoutResp(void);












































#endif  /*__GPRS_H_*/

