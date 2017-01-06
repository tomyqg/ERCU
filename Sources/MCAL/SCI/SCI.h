#ifndef __SCI_H_
#define __SCI_H_

#include "derivative.h"      /* derivative-specific definitions */
#include "includes.h"


/********************************************************************************************
***                        SCI Macro                                                
********************************************************************************************/
#define BusCLK_nM 16000000

#define SCI_NUMS 3
#define SCI0     0
#define SCI1     1
#define SCI2     2

//#define   BR        104   // bus frequency 16M, Baud Rate 9600
//#define   BR        26      // bus frequency 16M, Baud Rate 38400(38461)
#define   BR        13      // bus frequency 24M, Baud Rate 115200(115384)
//#define   BR        7      // bus frequency 12M, Baud Rate 115200(115384)

// SCI REGISTERS
#define   REG_OFFSET_SCIBDH    0x00
#define   REG_OFFSET_SCIBDL    0x01
#define   REG_OFFSET_SCICR1    0x02
#define   REG_OFFSET_SCICR2    0x03
#define   REG_OFFSET_SCISR1    0x04
#define   REG_OFFSET_SCISR2    0x05
#define   REG_OFFSET_SCIDRH    0x06
#define   REG_OFFSET_SCIDRL    0x07

#define SCI2_INT_ENABLE() SCI2CR2_RIE=1
#define SCI2_INT_DISABLE() SCI2CR2_RIE=0

#define SCI1_INT_ENABLE()  {SCI1CR2 |= SCI1CR2_RIE_MASK;}
#define SCI1_INT_DISABLE() {SCI1CR2 &= ~ SCI1CR2_RIE_MASK;}

 
/********************************************************************************************
***                        SCI Functions Declarations                                     ***
********************************************************************************************/
extern void SCI_OpenCommunication(INT8U sci_num,INT16U baudrateDivisor);
extern void SCI_CloseCommunication(INT8U sci_num);
extern void SCI2_Send1Byte(INT8U buffer);
extern void SCI2_SendStr(const INT8U *);
extern void SCI2_SendByLength(const INT8U str[], INT16U len) ;



































#endif