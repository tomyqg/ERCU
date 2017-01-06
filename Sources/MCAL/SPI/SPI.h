#ifndef __SPI__
#define __SPI__


//#include "FslTypes.h"
#include "derivative.h"
//#include "OS_CPU.h"
#include "ff_typedefs.h"

/* definitions */
#define SPI_SS    PTJ_PTJ3      /* Slave Select */
#define _SPI_SS   DDRJ_DDRJ3    

#define ENABLE    0
#define DISABLE   1

/* Global Variables */

/* Prototypes */
void SPI_Init(void);
void SPI_Send_byte(UINT8 u8Data);
UINT8 SPI_Receive_byte(void);
void SPI_High_rate(void);


#endif /* __SPI__ */