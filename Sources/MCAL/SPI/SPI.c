#include "SPI.h"

void SPI_Init(void)
{
	_SPI_SS= 1;

	//SPI1BR = 0x42; //5*8=40, 16M/40=400K hz    根据主机速率调整
	//SPI1BR = 0x23; //3*16=48, 16M/48=333K hz    根据主机速率调整
	SPI1BR = 0x33; //3*16=48, 24M/64=375K hz    根据主机速率调整
	//==============对SD卡初始化这两个速率都可以=============================
	
	SPI1CR2 = 0x00;     
	SPI1CR1 |= SPI1CR1_MSTR_MASK; //配置为主机，不加则为从机
	//SPI1C1 |= SPI1C1_CPOL_MASK;   
	//! CPOL=0, CPHA=1,LSBFE=0,默认====SPI时钟高有效、奇数沿采集数据、串行传输最高位开始
	//SPI1C1 &= ~ SPI1C1_CPHA_MASK; 

	SPI1CR1 |= SPI1CR1_SPE_MASK;//最后使能
}

/************************************************/
void SPI_Send_byte(UINT8 u8Data)
{
	UINT16 cnt;
	cnt = 0xFFF;
	
	(void)SPI1SR;
	
	do{
		cnt--;
	}while((!SPI1SR_SPTEF)&&(cnt));
	
	if(cnt)
	{
		SPI1DRL=u8Data;	
	}
}

/************************************************/
UINT8 SPI_Receive_byte(void)
{
  UINT16 cnt;
	UINT8 data = 0;
	
	(void)SPI1SR; //
	SPI1DRL=0xFF; //写一次到数据寄存器启动一次发送提供时钟，对方的数据移位过来
	
	cnt = 0xFFF;

	do{
		cnt--;
	}while((!SPI1SR_SPIF)&&(cnt));
	
	if(cnt)
	{
	  (void)SPI1SR;
		data = SPI1DRL;
	}

	return(data);
}

/************************************************/
void SPI_High_rate(void)
{
  SPI1BR = 0x22; 
  SPI1BR = 0x00;//最快2分频 		
}
