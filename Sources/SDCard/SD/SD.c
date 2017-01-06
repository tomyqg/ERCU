#include "SD.h"
#include "includes.h"    //����ϵͳ��ͷ�ļ�

volatile T32_8 gu8SD_Argument;
UINT8 SD_Type;

  
  
/***********************��SD������һ������(������ʧ��Ƭѡ�����к������ݴ�����******************************************************/
UINT8 SD_SendCommand(UINT8 u8SDCommand, UINT8 u8SDCrc) 
{
  UINT8 u8Counter;
  UINT8 u8Temp;
    
  SPI_SS=ENABLE;
  WriteSPIByte(u8SDCommand);

  for(u8Counter=0;u8Counter<4;u8Counter++) 
      WriteSPIByte(gu8SD_Argument.bytes[u8Counter]); 

  WriteSPIByte(u8SDCrc);

  u8Counter=SD_WAIT_CYCLES;
  u8Temp=0xFF;
  do
  {
    u8Temp=ReadSPIByte();
    u8Counter--;
  }while((u8Temp == 0xFF) && u8Counter > 0);//ֻҪ�����ݣ���Ϊ0xFF���˳�   
  
  return u8Temp;
}



void SD_CLKDelay(UINT8 u8Frames) 
{
    while(u8Frames--)
        WriteSPIByte(0xFF);
}

//�ȴ�SD����Ӧ
//Response:Ҫ�õ��Ļ�Ӧֵ
//����ֵ:0,   �õ��˸û�Ӧֵ
//       ����,�õ���Ӧֵʧ��
//UINT8 Resp0xFE;
UINT8 SD_GetResponse(UINT8 Response)
{
	UINT16 Count=0xFFFF;	   						  

	while (((ReadSPIByte())!=Response)&&Count) Count--; 	  
	
	if(Count)
	{
	  return MSD_RESPONSE_NO_ERROR ;
	}
	else 
	{
	  return MSD_RESPONSE_FAILURE;
	}
}

UINT8 idleResp;
static SD_RESULT SD_Idle_Sta(void)
{
	UINT8 resp;
	UINT16 retry;	   	  
   
  retry = 0;
  resp = 255;
  gu8SD_Argument.lword=0;
  
  SPI_SS=ENABLE;
  SD_CLKDelay(10);            // Send 80 clocks
  //SD_CLKDelay(8);          //�������4G�Ĺ���ȥ

  //-----------------SD����λ��idle��ʼ-----------------
  //ѭ����������CMD0��ֱ��SD������0x01,����IDLE״̬. ��ʱ��ֱ���˳�
  do
  {	   
      idleResp= resp = SD_SendCommand(SD_CMD0|0x40, 0x95);//��������������ȫû����Ӧ 
      retry++;
  }while((SD_IDLE!=resp)&&(retry<RETRY_TIMES));
  
  if(RETRY_TIMES == retry)
    return INIT_FAILS; 
	
	return ERR_SD_NONE;					  
}



//UINT8 InitStep;
/*****************************************************************************/
SD_RESULT SD_Init(void) 
{
  UINT8 retry, resp;
  UINT8 buff[6];
  
  InitSPI(); 
  
  if(SD_Idle_Sta())
  {
    SPI_SS=DISABLE;
    //InitStep = 1;
    return(INIT_FAILS);      
  }
  OSTimeDly(1);//��Ҫ�㹻����ʱ
  
  
  //��ȡ��Ƭ��SD�汾��Ϣ
  gu8SD_Argument.lword=0x1AA;/*0x100  2.7V - 3.6V */  /*0xaa test pattern */
  resp = SD_SendCommand(SD_CMD8|0x40,0x87);
  
  if(V1_RESPONSE == resp)
  {
    //�����V1.0����CMD8ָ���û�к�������
    //Ƭѡ�øߣ�������������
    SPI_SS=DISABLE;
    //�෢8��CLK����SD������������
    SD_CLKDelay(1);
    //���ÿ�����ΪSDV1.0����������⵽ΪMMC�������޸�ΪMMC
    SD_Type = SD_TYPE_V1;
    
    //!<-----------------SD����MMC����ʼ����ʼ------------------------------------------------------	 
    //������ʼ��ָ��CMD55+ACMD41
    // �����Ӧ��˵����SD�����ҳ�ʼ�����
    // û�л�Ӧ��˵����MMC�������������Ӧ��ʼ��
    retry = 0;
    resp = 255;
    gu8SD_Argument.lword=0;
    do
    {
      if(0x01 != SD_SendCommand(SD_CMD55|0x40, 0)) //ֻҪ����0xff,�ͽ��ŷ���
      {
        SPI_SS=DISABLE;
        //InitStep = 2;
        return(INIT_FAILS);      
      }
      resp = SD_SendCommand(SD_CMD41|0x40, 0);
      retry++;
    }while((SD_OK!=resp)&&(retry<RETRY_TIMES));
    // �ж��ǳ�ʱ���ǵõ���ȷ��Ӧ
    // ���л�Ӧ����SD����û�л�Ӧ����MMC��	  
    //----------MMC�������ʼ��������ʼ------------
    if(RETRY_TIMES<=retry)
    {
      retry = 0;
      resp = 255;
      //����MMC����ʼ�����û�в��ԣ�
      do
      {	   
        resp = SD_SendCommand(SD_CMD1|0x40, 0); 
        retry++;
      }while((SD_OK!=resp)&&(retry<RETRY_TIMES));
      if(RETRY_TIMES<=retry)
      {
        //InitStep = 3;
        return (INIT_FAILS);   //MMC����ʼ����ʱ		    
      }
      //д�뿨����
      SD_Type = SD_TYPE_MMC;
      //----------MMC�������ʼ����������------------	    
      //��ֹCRCУ��
      if(SD_OK!=SD_SendCommand(SD_CMD59|0x40,0x95)) 
      {
        SPI_SS=DISABLE;
        //InitStep = 4;
        return(INIT_FAILS);      
      }
      SPI_SS=DISABLE;
      (void)ReadSPIByte();  // Dummy SPI cycle 
      
      SPI_SS=ENABLE;
      gu8SD_Argument.lword=SD_BLOCK_SIZE;
      if(SD_OK!=SD_SendCommand(SD_CMD16|0x40,0x95))
      {
         SPI_SS=DISABLE;
         //InitStep = 5;
         return(INIT_FAILS);      
      }
      SPI_SS=DISABLE;
      (void)ReadSPIByte();  // Dummy SPI cycle 
    }
    //-----------------SD����MMC����ʼ������---------------------------------------------------------------    
  } 
    //SD��ΪV1.0�汾�ĳ�ʼ������	 
    //������V2.0���ĳ�ʼ��
    //������Ҫ��ȡOCR���ݣ��ж���SD2.0����SD2.0HC��
  else if(V2_RESPONSE == resp)
  {
    //V2.0�Ŀ���CMD8�����ᴫ��4�ֽڵ����ݣ�Ҫ�����ٽ���������
    buff[0] =ReadSPIByte();  //should be 0x00
    buff[1] =ReadSPIByte();  //should be 0x00
    buff[2] =ReadSPIByte();  //should be 0x01
    buff[3] =ReadSPIByte();  //should be 0xAA
    //SPI_SS=DISABLE;
    //(void)ReadSPIByte();  // Dummy SPI cycle 
    //�жϸÿ��Ƿ�֧��2.7V-3.6V�ĵ�ѹ��Χ
    //if(buff[2]==0x01 && buff[3]==0xAA) //���жϣ�����֧�ֵĿ�����
        
    //������ʼ��ָ��CMD55+ACMD41
    retry = 0;
    resp = 255;
    do{
      gu8SD_Argument.lword=0;
      if(0xFF == SD_SendCommand(SD_CMD55|0x40,0))//ֻҪ����0xff,�ͽ��ŷ��� 
      {
        SPI_SS=DISABLE;
        //InitStep = 7;
        return(INIT_FAILS);      
      }
      gu8SD_Argument.lword=0x40000000;//V2.0
      resp = SD_SendCommand(SD_CMD41|0x40, 0);	   
      retry++;
    }while((SD_OK!=resp)&&(retry<RETRY_TIMES));
    if(RETRY_TIMES<=retry)
    {
      SPI_SS=DISABLE;
      //InitStep = 8;
      return (INIT_FAILS);
    }
     
    //��ʼ��ָ�����ɣ���������ȡOCR��Ϣ		   
    //-----------����SD2.0���汾��ʼ--------------------------------
    gu8SD_Argument.lword=0;
    if(SD_OK != SD_SendCommand(SD_CMD58|0x40,0)) 
    {
      SPI_SS=DISABLE;
      //InitStep = 9;
      return(INIT_FAILS);      
    }
    //��OCRָ����󣬽�������4�ֽڵ�OCR��Ϣ
    buff[0] =ReadSPIByte();  
    buff[1] =ReadSPIByte();  
    buff[2] =ReadSPIByte();  
    buff[3] =ReadSPIByte();
    SPI_SS=DISABLE;
    (void)ReadSPIByte();  // Dummy SPI cycle
    //�����յ���OCR�е�bit30λ��CCS����ȷ����ΪSD2.0����SDHC
    //���CCS=1��SDHC   CCS=0��SD2.0
    if(buff[0]&0x40)
    {
      SD_Type = SD_TYPE_V2HC;
    }	 
    else 
    {
      SD_Type = SD_TYPE_V2;	    
    }
    //-----------����SD2.0���汾����---------------------------------  
  }
  
  //����SPIΪ����ģʽ
  HighSpeedSPI();

  return(ERR_SD_NONE);
}


//�ȴ�SD��д�����
//����ֵ:0,�ɹ�;   
//    ����,�������;
UINT8 SD_WaitDataReady(void)
{
  UINT8 r1=MSD_DATA_OTHER_ERROR;
  UINT32 retry;
  retry=0;
  do
  {
    r1=ReadSPIByte()&0X1F;//������Ӧ
    if(retry==0xFFFE)return 1; 
		retry++;
		switch (r1)
		{					   
			case MSD_DATA_OK://���ݽ�����ȷ��	 
				r1=MSD_DATA_OK;
				break;  
			case MSD_DATA_CRC_ERROR:  //CRCУ�����
				return MSD_DATA_CRC_ERROR;  
			case MSD_DATA_WRITE_ERROR://����д�����
				return MSD_DATA_WRITE_ERROR;  
			default://δ֪����    
				r1=MSD_DATA_OTHER_ERROR;
				break;	 
		}   
  }while(r1==MSD_DATA_OTHER_ERROR); //���ݴ���ʱһֱ�ȴ�
  
  retry=0;
  while(ReadSPIByte()==0)//��������Ϊ0,�����ݻ�δд���
  {
  	retry++;
  	//delay_us(10);//SD��д�ȴ���Ҫ�ϳ���ʱ��
  	if(retry>=0xFFFFFFFE)return 0xFF;//�ȴ�ʧ����
  };	    
  return 0;//�ɹ���
}


/*****************************************************************************/
UINT8 SDWriteStep,*SDWritePtr;
UINT32 BlockToWrite;
SD_RESULT SD_Write_Block(UINT32 u16SD_Block,UINT8 *pu8DataPointer) 
{
    UINT16 u16Counter;     BlockToWrite=u16SD_Block; SDWritePtr= pu8DataPointer;

    SPI_SS=DISABLE;

    gu8SD_Argument.lword=u16SD_Block;
    if(SD_Type!=SD_TYPE_V2HC)
    {
      gu8SD_Argument.lword=gu8SD_Argument.lword<< SD_BLOCK_SHIFT;
    }
      
    if(SD_OK != SD_SendCommand(SD_CMD24|0x40,0x95))
    {
        SDWriteStep=1;
        SPI_SS=DISABLE;
        return(WRITE_COMMAND_FAILS);      
    }
    
    WriteSPIByte(0xFE);
    
    for(u16Counter=0;u16Counter<BLOCK_SIZE;u16Counter++)
        WriteSPIByte(*pu8DataPointer++);

    WriteSPIByte(0xFF);    //��2��Byte��dummy CRC
    WriteSPIByte(0xFF);

    //for(u16Counter=0;u16Counter<BLOCK_SIZE;u16Counter++)
    
    if(SD_WaitDataReady())//�ȴ�SD������д�����
    {
        SDWriteStep=2;
        SPI_SS=DISABLE;
        return(WRITE_DATA_FAILS);      
    }

    while(ReadSPIByte()==0x00);  // Dummy SPI cycle

    SPI_SS=DISABLE;
    return(ERR_SD_NONE);
}

//UINT8 ReadStep,CMD12Resp;
 UINT8 *SDReadPtr;
 UINT32 BlockToRead;
/*****************************************************************************/
SD_RESULT SD_Read_Block(UINT32 u16SD_Block,UINT8 *pu8DataPointer) 
{
  UINT16 u16Counter;
  UINT8 retry, resp;

  SPI_SS=DISABLE;  //BlockToRead=u16SD_Block; SDReadPtr= pu8DataPointer;

  gu8SD_Argument.lword=u16SD_Block;
  if(SD_Type!=SD_TYPE_V2HC)
  {      
    gu8SD_Argument.lword=gu8SD_Argument.lword<< SD_BLOCK_SHIFT;
  }

  do{	   
    resp = SD_SendCommand(SD_CMD17|0x40,0x00); 
    retry++;
  }while((SD_OK!=resp)&&(retry<RETRY_TIMES));
  if(RETRY_TIMES<=retry)
  {
    SPI_SS=DISABLE;
    //ReadStep = 1;
    return (READ_COMMAND_FAILS);   		    
  }

  if(SD_GetResponse(0xFE))//�ȴ�SD������������ʼ����0xFE
	{	  
		SPI_SS=DISABLE;
  	//ReadStep = 2;
		return READ_DATA_FAILS;
	}
  
  for(u16Counter=0;u16Counter<BLOCK_SIZE;u16Counter++)
      *pu8DataPointer++=ReadSPIByte();

  //������2��αCRC��dummy CRC��
  (void)ReadSPIByte();  
  (void)ReadSPIByte();  

  SPI_SS=DISABLE;

  (void)ReadSPIByte();  // Dummy SPI cycle
  
  return(ERR_SD_NONE);
}






