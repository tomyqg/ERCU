#include "SCI.h"



volatile INT8U *sci_periph[SCI_NUMS] = {
  &SCI0BDH,
  &SCI1BDH,
  &SCI2BDH
};

BOOLEAN sci_status[SCI_NUMS]={FALSE,FALSE,FALSE};

void SCI_OpenCommunication(INT8U sci_num,INT16U baudrateDivisor)
{  
  unsigned volatile char *sci_pt;

  sci_pt = sci_periph[sci_num];
  
  if (!sci_status[sci_num]) 
  {
    sci_status[sci_num] = TRUE;      
    sci_pt[REG_OFFSET_SCIBDH] = (unsigned char)((baudrateDivisor>>8) & 0xFF);
    sci_pt[REG_OFFSET_SCIBDL] = (unsigned char)((baudrateDivisor>>0) & 0xFF);
      
    // Normal operation, 8 data bits stop and no Parity
    sci_pt[REG_OFFSET_SCICR1] = 0x00;
    // Trasmitter and Receiver Enable 
    sci_pt[REG_OFFSET_SCICR2] = 0x0C;
    //sci_pt[REG_OFFSET_SCICR2] |= SCI1CR2_RIE_MASK;
  }
}


void SCI_CloseCommunication(INT8U sci_num)
{ 
  unsigned volatile char *sci_pt;

  sci_pt = sci_periph[sci_num];
    
  if (sci_status[sci_num]) 
  {
    sci_status[sci_num] = FALSE;
    //timer = TIME_OUT;
    // Verify that Receive Data Register is FULL
    while(!(sci_pt[REG_OFFSET_SCISR1]&0x20))
      ;
    if (sci_pt[REG_OFFSET_SCISR1]&0x20)
      (void)sci_pt[REG_OFFSET_SCIDRL];// Clear RDRF Flag
      
    // Trasmitter and Receiver Disable
    sci_pt[REG_OFFSET_SCICR2] = 0;
    sci_pt[REG_OFFSET_SCICR1] = 0;
    sci_pt[REG_OFFSET_SCIBDH] = 0x00;  
    sci_pt[REG_OFFSET_SCIBDL] = 0x00;
  }
}



void SCI2_Send1Byte(INT8U data)
{
  unsigned volatile char *sci_pt;
  unsigned int i;

  SCI2_INT_DISABLE();
  
  sci_pt = sci_periph[2];
  if(!sci_status[2])
    return;
  i = 0xFFFF;
  do
  {
    i --;
  }while(!(sci_pt[REG_OFFSET_SCISR1]&SCI2SR1_TDRE_MASK) && i);
  
  if (0 != i)
    sci_pt[REG_OFFSET_SCIDRL] = data;
  
  i = 0xFFFF;
  do
  {
    i --;
  }while(!(sci_pt[REG_OFFSET_SCISR1]&SCI2SR1_TC_MASK) && i);
    
  SCI2_INT_ENABLE();        
}




void SCI2_SendByLength(const INT8U str[], INT16U len) 
{
  if(len>0)
  { 
    while(len)
    {
      len--;
      SCI2_Send1Byte(*str++);
    }
  }
} 

void SCI2_SendStr(const INT8U str[]) 
{
  while(*str)
  {
    SCI2_Send1Byte(*str++);
  }
}
