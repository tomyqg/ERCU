#include "diskio.h"
#include "ff.h"			/* FatFs configurations and declarations */
#include "SD.h"



volatile DSTATUS Stat = STA_NOINIT;	 /* ����״̬*/
volatile UINT Timer1;		             /* 1000Hz�ݼ�ʱ�� */
#define SD_SECTOR_SIZE        512         

UINT32 TotalSectors = 0; /*SD����������*/


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
** Descriptions:        ��ʼ������������
** input parameters:    drv������������    �˴�ʵ��ֻ����һ��������
** output parameters:   ��
** Returned value:      ��ʼ������״̬
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
** Descriptions:        ��ȡ����״̬
** input parameters:    drv������������
** output parameters:   ��
** Returned value:      ���̵�ǰ״̬
*********************************************************************************************************/
DSTATUS disk_status (BYTE drv)
{
	if (drv) return STA_NOINIT;		                                        /* ֻ��һ��������               */
	return Stat;
}

/*********************************************************************************************************
** Function name:       disk_read
** Descriptions:        ��ȡ��������
** input parameters:    drv������������
                        sector������ʼ������LBA��
                        count����ȡ��������
** output parameters:   buff�������ݻ���ָ��
** Returned value:      ������״̬
*********************************************************************************************************/
DRESULT disk_read (
	BYTE drv,
	BYTE *buffer,
	DWORD sector,
	BYTE count	
)
{
	if (drv || !count) return RES_PARERR;                               /* �����źͶ���������ڲ������ */
	if (Stat & STA_NOINIT) return RES_NOTRDY;                           /* ��״̬���                   */
    
    //remove 20151113 xyl
    if(sector + count > TotalSectors) return RES_PARERR;             /* ���ռ���                   */

    SD_Read_Block(sector, buffer);                                    /* SD����sector������count������*/

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
** Descriptions:        д��������
** input parameters:    drv������������
                        buff��д���ݻ���ָ��
                        sector��д��ʼ������LBA��
                        count��д��������
** output parameters:   ��
** Returned value:      д����״̬
*********************************************************************************************************/
DRESULT disk_write (
	BYTE drv,
	const BYTE *buffer,	
	DWORD sector,
	BYTE count
)
{
	if (drv || !count) return RES_PARERR;                               /* �����źͶ���������ڲ������ */
	if (Stat & STA_NOINIT) return RES_NOTRDY;                           /* ��״̬���                   */
	if (Stat & STA_PROTECT) return RES_WRPRT;                           /* ����λ���                   */

    if(sector + count > TotalSectors) return RES_PARERR;             /* ���ռ���                   */
         
  SD_Write_Block(sector,buffer);
  
	return RES_OK;
}

/*********************************************************************************************************
** Function name:       disk_ioctl
** Descriptions:        ����I/O����
** input parameters:    drv������������
                        ctrl�����ƴ���
** output parameters:   buff��������Ӧ������Ϣ��ָ��
** Returned value:      ���Ʋ���״̬
*********************************************************************************************************/
DRESULT disk_ioctl (
	BYTE drv,
	BYTE ctrl,
	void *buff
)
{
	DRESULT res;

	if (drv) 
	  return RES_PARERR;                                         /* ��������ڲ������           */
	if (Stat & STA_NOINIT) 
	  return RES_NOTRDY;                           /* ��״̬���                   */

	res = RES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC :	                                                /* ͬ���ѹ���Ļ�����           */
      res = RES_OK;
			break;

		case CTRL_INVALIDATE :	                                          /* ��δ�����ļ�ϵͳʱ��ʾ       */
	    Stat = STA_NOINIT;	                                          /* ���ô���Ϊδ������״̬       */
	    res = RES_OK;
	    break;

		case GET_SECTOR_COUNT :	                                          /* ��ô���������               */
      *(DWORD*)buff = TotalSectors;
      res = RES_OK;
			break;

		case GET_SECTOR_SIZE :	                                          /* ��ô���������С             */
			*(WORD*)buff = SD_SECTOR_SIZE;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	                                            /* ��ò������С               */
      *(WORD*)buff = 1;
      res = RES_OK;
			break;

		default:
			res = RES_OK;
	}
	return res;
}





