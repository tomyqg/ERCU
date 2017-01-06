#include "SPI.h"

void SPI_Init(void)
{
	_SPI_SS= 1;

	//SPI1BR = 0x42; //5*8=40, 16M/40=400K hz    �����������ʵ���
	//SPI1BR = 0x23; //3*16=48, 16M/48=333K hz    �����������ʵ���
	SPI1BR = 0x33; //3*16=48, 24M/64=375K hz    �����������ʵ���
	//==============��SD����ʼ�����������ʶ�����=============================
	
	SPI1CR2 = 0x00;     
	SPI1CR1 |= SPI1CR1_MSTR_MASK; //����Ϊ������������Ϊ�ӻ�
	//SPI1C1 |= SPI1C1_CPOL_MASK;   
	//! CPOL=0, CPHA=1,LSBFE=0,Ĭ��====SPIʱ�Ӹ���Ч�������زɼ����ݡ����д������λ��ʼ
	//SPI1C1 &= ~ SPI1C1_CPHA_MASK; 

	SPI1CR1 |= SPI1CR1_SPE_MASK;//���ʹ��
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
	SPI1DRL=0xFF; //дһ�ε����ݼĴ�������һ�η����ṩʱ�ӣ��Է���������λ����
	
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
  SPI1BR = 0x00;//���2��Ƶ 		
}
