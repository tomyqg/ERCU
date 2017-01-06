#include "SD.h"
#include "includes.h"    //操作系统总头文件

volatile T32_8 gu8SD_Argument;
UINT8 SD_Type;

  
  
/***********************向SD卡发送一个命令(结束后不失能片选，还有后续数据传来）******************************************************/
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
  }while((u8Temp == 0xFF) && u8Counter > 0);//只要有数据，不为0xFF就退出   
  
  return u8Temp;
}



void SD_CLKDelay(UINT8 u8Frames) 
{
    while(u8Frames--)
        WriteSPIByte(0xFF);
}

//等待SD卡回应
//Response:要得到的回应值
//返回值:0,   得到了该回应值
//       其他,得到回应值失败
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
  //SD_CLKDelay(8);          //加上这个4G的过不去

  //-----------------SD卡复位到idle开始-----------------
  //循环连续发送CMD0，直到SD卡返回0x01,进入IDLE状态. 超时则直接退出
  do
  {	   
      idleResp= resp = SD_SendCommand(SD_CMD0|0x40, 0x95);//换成其他数据完全没有响应 
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
  OSTimeDly(1);//需要足够的延时
  
  
  //获取卡片的SD版本信息
  gu8SD_Argument.lword=0x1AA;/*0x100  2.7V - 3.6V */  /*0xaa test pattern */
  resp = SD_SendCommand(SD_CMD8|0x40,0x87);
  
  if(V1_RESPONSE == resp)
  {
    //如果是V1.0卡，CMD8指令后没有后续数据
    //片选置高，结束本次命令
    SPI_SS=DISABLE;
    //多发8个CLK，让SD结束后续操作
    SD_CLKDelay(1);
    //设置卡类型为SDV1.0，如果后面检测到为MMC卡，再修改为MMC
    SD_Type = SD_TYPE_V1;
    
    //!<-----------------SD卡、MMC卡初始化开始------------------------------------------------------	 
    //发卡初始化指令CMD55+ACMD41
    // 如果有应答，说明是SD卡，且初始化完成
    // 没有回应，说明是MMC卡，额外进行相应初始化
    retry = 0;
    resp = 255;
    gu8SD_Argument.lword=0;
    do
    {
      if(0x01 != SD_SendCommand(SD_CMD55|0x40, 0)) //只要不是0xff,就接着发送
      {
        SPI_SS=DISABLE;
        //InitStep = 2;
        return(INIT_FAILS);      
      }
      resp = SD_SendCommand(SD_CMD41|0x40, 0);
      retry++;
    }while((SD_OK!=resp)&&(retry<RETRY_TIMES));
    // 判断是超时还是得到正确回应
    // 若有回应：是SD卡；没有回应：是MMC卡	  
    //----------MMC卡额外初始化操作开始------------
    if(RETRY_TIMES<=retry)
    {
      retry = 0;
      resp = 255;
      //发送MMC卡初始化命令（没有测试）
      do
      {	   
        resp = SD_SendCommand(SD_CMD1|0x40, 0); 
        retry++;
      }while((SD_OK!=resp)&&(retry<RETRY_TIMES));
      if(RETRY_TIMES<=retry)
      {
        //InitStep = 3;
        return (INIT_FAILS);   //MMC卡初始化超时		    
      }
      //写入卡类型
      SD_Type = SD_TYPE_MMC;
      //----------MMC卡额外初始化操作结束------------	    
      //禁止CRC校验
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
    //-----------------SD卡、MMC卡初始化结束---------------------------------------------------------------    
  } 
    //SD卡为V1.0版本的初始化结束	 
    //下面是V2.0卡的初始化
    //其中需要读取OCR数据，判断是SD2.0还是SD2.0HC卡
  else if(V2_RESPONSE == resp)
  {
    //V2.0的卡，CMD8命令后会传回4字节的数据，要跳过再结束本命令
    buff[0] =ReadSPIByte();  //should be 0x00
    buff[1] =ReadSPIByte();  //should be 0x00
    buff[2] =ReadSPIByte();  //should be 0x01
    buff[3] =ReadSPIByte();  //should be 0xAA
    //SPI_SS=DISABLE;
    //(void)ReadSPIByte();  // Dummy SPI cycle 
    //判断该卡是否支持2.7V-3.6V的电压范围
    //if(buff[2]==0x01 && buff[3]==0xAA) //不判断，让其支持的卡更多
        
    //发卡初始化指令CMD55+ACMD41
    retry = 0;
    resp = 255;
    do{
      gu8SD_Argument.lword=0;
      if(0xFF == SD_SendCommand(SD_CMD55|0x40,0))//只要不是0xff,就接着发送 
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
     
    //初始化指令发送完成，接下来获取OCR信息		   
    //-----------鉴别SD2.0卡版本开始--------------------------------
    gu8SD_Argument.lword=0;
    if(SD_OK != SD_SendCommand(SD_CMD58|0x40,0)) 
    {
      SPI_SS=DISABLE;
      //InitStep = 9;
      return(INIT_FAILS);      
    }
    //读OCR指令发出后，紧接着是4字节的OCR信息
    buff[0] =ReadSPIByte();  
    buff[1] =ReadSPIByte();  
    buff[2] =ReadSPIByte();  
    buff[3] =ReadSPIByte();
    SPI_SS=DISABLE;
    (void)ReadSPIByte();  // Dummy SPI cycle
    //检查接收到的OCR中的bit30位（CCS），确定其为SD2.0还是SDHC
    //如果CCS=1：SDHC   CCS=0：SD2.0
    if(buff[0]&0x40)
    {
      SD_Type = SD_TYPE_V2HC;
    }	 
    else 
    {
      SD_Type = SD_TYPE_V2;	    
    }
    //-----------鉴别SD2.0卡版本结束---------------------------------  
  }
  
  //设置SPI为高速模式
  HighSpeedSPI();

  return(ERR_SD_NONE);
}


//等待SD卡写入完成
//返回值:0,成功;   
//    其他,错误代码;
UINT8 SD_WaitDataReady(void)
{
  UINT8 r1=MSD_DATA_OTHER_ERROR;
  UINT32 retry;
  retry=0;
  do
  {
    r1=ReadSPIByte()&0X1F;//读到回应
    if(retry==0xFFFE)return 1; 
		retry++;
		switch (r1)
		{					   
			case MSD_DATA_OK://数据接收正确了	 
				r1=MSD_DATA_OK;
				break;  
			case MSD_DATA_CRC_ERROR:  //CRC校验错误
				return MSD_DATA_CRC_ERROR;  
			case MSD_DATA_WRITE_ERROR://数据写入错误
				return MSD_DATA_WRITE_ERROR;  
			default://未知错误    
				r1=MSD_DATA_OTHER_ERROR;
				break;	 
		}   
  }while(r1==MSD_DATA_OTHER_ERROR); //数据错误时一直等待
  
  retry=0;
  while(ReadSPIByte()==0)//读到数据为0,则数据还未写完成
  {
  	retry++;
  	//delay_us(10);//SD卡写等待需要较长的时间
  	if(retry>=0xFFFFFFFE)return 0xFF;//等待失败了
  };	    
  return 0;//成功了
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

    WriteSPIByte(0xFF);    //发2个Byte的dummy CRC
    WriteSPIByte(0xFF);

    //for(u16Counter=0;u16Counter<BLOCK_SIZE;u16Counter++)
    
    if(SD_WaitDataReady())//等待SD卡数据写入完成
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

  if(SD_GetResponse(0xFE))//等待SD卡发回数据起始令牌0xFE
	{	  
		SPI_SS=DISABLE;
  	//ReadStep = 2;
		return READ_DATA_FAILS;
	}
  
  for(u16Counter=0;u16Counter<BLOCK_SIZE;u16Counter++)
      *pu8DataPointer++=ReadSPIByte();

  //下面是2个伪CRC（dummy CRC）
  (void)ReadSPIByte();  
  (void)ReadSPIByte();  

  SPI_SS=DISABLE;

  (void)ReadSPIByte();  // Dummy SPI cycle
  
  return(ERR_SD_NONE);
}






