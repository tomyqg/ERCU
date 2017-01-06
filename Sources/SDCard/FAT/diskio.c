#include "diskio.h"
#include "ff.h"			/* FatFs configurations and declarations */
#include "SD.h"



volatile DSTATUS Stat = STA_NOINIT;	 /* 磁盘状态*/
volatile UINT Timer1;		             /* 1000Hz递减时钟 */
#define SD_SECTOR_SIZE        512         

UINT32 TotalSectors = 0; /*SD卡扇区总数*/


void disk_get_total_sectors(BYTE *buffer)
{
  if(ERR_SD_NONE==SD_Read_Block(0, buffer))
  {
    TotalSectors = LD_WORD(buffer+BPB_TotSec16);			
    if (!TotalSectors) 
    {
      TotalSectors = LD_DWORD(buffer+BPB_TotSec32);
    }
  }
}


/*********************************************************************************************************
** Function name:       disk_initialize
** Descriptions:        初始化磁盘驱动器
** input parameters:    drv：物理驱动号    此处实现只用了一个驱动器
** output parameters:   无
** Returned value:      初始化操作状态
*********************************************************************************************************/
DSTATUS disk_initialize (BYTE drv)
{  
  UINT8 retry;
  SD_RESULT sdInitResult;

  if(drv) 
  {
    return STA_NOINIT;
  }

  if (Stat & STA_NOINIT) 
  {
    retry = 3;
    do
    {
      sdInitResult = SD_Init();
      retry--;
    }while(sdInitResult && retry);
    
    if(ERR_SD_NONE != sdInitResult)
      return STA_NOINIT;                                                   
  }
  
  Stat &= ~ STA_NOINIT;	                                            
	return Stat;
}



/*DSTATUS disk_xinitialize (uint8_t cardnum)
{
    Stat = STA_NOINIT;	
    if (Stat & STA_NOINIT) {

        if(sdxInit(cardnum))
        {
            TotalSectors = disk_get_total_sectors(); 
            Stat &= ~STA_NOINIT;
        }

    }

	return Stat;
}*/

/*********************************************************************************************************
** Function name:       disk_status
** Descriptions:        获取磁盘状态
** input parameters:    drv：物理驱动号
** output parameters:   无
** Returned value:      磁盘当前状态
*********************************************************************************************************/
DSTATUS disk_status (BYTE drv)
{
	if (drv) return STA_NOINIT;		                                        /* 只有一个驱动器               */
	return Stat;
}

/*********************************************************************************************************
** Function name:       disk_read
** Descriptions:        读取磁盘扇区
** input parameters:    drv：物理驱动号
                        sector：读开始扇区（LBA）
                        count：读取扇区数量
** output parameters:   buff：读数据缓冲指针
** Returned value:      读操作状态
*********************************************************************************************************/
DRESULT disk_read (
	BYTE drv,
	BYTE *buffer,
	DWORD sector,
	BYTE count	
)
{
	if (drv || !count) return RES_PARERR;                               /* 驱动号和读扇区数入口参数检查 */
	if (Stat & STA_NOINIT) return RES_NOTRDY;                           /* 卡状态检查                   */
    
    //remove 20151113 xyl
    if(sector + count > TotalSectors) return RES_PARERR;             /* 卡空间检查                   */

    SD_Read_Block(sector, buffer);                                    /* SD卡从sector扇区读count个扇区*/

	return RES_OK;
}
  
/*
  SD_Read_Block(0, g_SDFatFs.win);
  TotalSectors = LD_WORD(g_SDFatFs.win+BPB_TotSec16);			
  if (!TotalSectors) 
  {
    TotalSectors = LD_DWORD(g_SDFatFs.win+BPB_TotSec32);
  }
  
*/
/*********************************************************************************************************
** Function name:       disk_write
** Descriptions:        写磁盘扇区
** input parameters:    drv：物理驱动号
                        buff：写数据缓冲指针
                        sector：写开始扇区（LBA）
                        count：写扇区数量
** output parameters:   无
** Returned value:      写操作状态
*********************************************************************************************************/
DRESULT disk_write (
	BYTE drv,
	const BYTE *buffer,	
	DWORD sector,
	BYTE count
)
{
	if (drv || !count) return RES_PARERR;                               /* 驱动号和读扇区数入口参数检查 */
	if (Stat & STA_NOINIT) return RES_NOTRDY;                           /* 卡状态检查                   */
	if (Stat & STA_PROTECT) return RES_WRPRT;                           /* 保护位检查                   */

    if(sector + count > TotalSectors) return RES_PARERR;             /* 卡空间检查                   */
         
  SD_Write_Block(sector,buffer);
  
	return RES_OK;
}

/*********************************************************************************************************
** Function name:       disk_ioctl
** Descriptions:        磁盘I/O控制
** input parameters:    drv：物理驱动号
                        ctrl：控制代号
** output parameters:   buff：返回相应控制信息的指针
** Returned value:      控制操作状态
*********************************************************************************************************/
DRESULT disk_ioctl (
	BYTE drv,
	BYTE ctrl,
	void *buff
)
{
	DRESULT res;

	if (drv) 
	  return RES_PARERR;                                         /* 驱动号入口参数检查           */
	if (Stat & STA_NOINIT) 
	  return RES_NOTRDY;                           /* 卡状态检查                   */

	res = RES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC :	                                                /* 同步已挂起的缓冲区           */
      res = RES_OK;
			break;

		case CTRL_INVALIDATE :	                                          /* 当未挂载文件系统时标示       */
	    Stat = STA_NOINIT;	                                          /* 设置磁盘为未出世哈状态       */
	    res = RES_OK;
	    break;

		case GET_SECTOR_COUNT :	                                          /* 获得磁盘扇区数               */
      *(DWORD*)buff = TotalSectors;
      res = RES_OK;
			break;

		case GET_SECTOR_SIZE :	                                          /* 获得磁盘扇区大小             */
			*(WORD*)buff = SD_SECTOR_SIZE;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	                                            /* 获得擦除块大小               */
      *(WORD*)buff = 1;
      res = RES_OK;
			break;

		default:
			res = RES_OK;
	}
	return res;
}





