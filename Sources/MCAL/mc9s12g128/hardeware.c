/*
*********************************************************************************************************
* File       : hardware.c
*Description : s12g128的硬件设置，包括时钟和RTI实时中断的配置
*            : 适用芯片mc9s12g128
* Note(s)    : 最后修改日期 11-12-12
*            : 适用芯片mc9s12g128
*********************************************************************************************************
*/
#include "common.h"          /* 总头文件*/

void IO_Init(void)
{
  BAT_ON();
  BAT_DIR_OUT();
  LED1_D2_ON();
  LED1_D2_DIR_OUT();
  MG2639_SYSRST_N_HIGH();  
  MG2639_SYSRST_N_DIR_OUT()
  MG2639_PWRKEY_N_HIGH();
  MG2639_PWRKEY_N_DIR_OUT();    
}



void PLL_PEE_Init(void)
{     
    /* CPMUPROT: PROT=0 */
    CPMUPROT_PROT = 0;                    /* Disable protection of clock configuration registers */
    /* CPMUCLKS: PSTP=0 */
    CPMUCLKS_PSTP = 0;
    /* CPMUCLKS: PLLSEL=1 */
    CPMUCLKS_PLLSEL = 1;/* Enable the PLL to allow write to divider registers */
    /* CPMUCLKS: RTIOSCSEL=1 RTI clock source is OSCCLK
                 RTIOSCSEL=0 RTI clock source is IRCCLK*/
    CPMUCLKS_RTIOSCSEL = 1;
    /* Set Freq = 1 MHz */
    CPMUREFDIV_REFDIV = OSC_FRQ_MHZ - 1; /* Set the divider register(Write only when PROT is 0,and PLLSEL is 1) */
    CPMUREFDIV_REFFRQ = 0;    
    /* Set Fvco is 2 * BUS_FRQ_MHZ */
    CPMUSYNR_SYNDIV = BUS_FRQ_MHZ - 1;   /* Set the multiplier register */
    CPMUSYNR_VCOFRQ = 0; /*0 For 32 MHz ~ 48 MHz*/
    
    /* Fpll = Fvco / 2 when PLL locked */
    CPMUPOSTDIV = 0x00U;
    
    CPMUOSC_OSCE = 1;                     /* Enable the oscillator */
    /* CPMUPLL: FM1=0,FM0=0 */
    CPMUPLL = 0x00U;                     /* Set the PLL frequency modulation(0 for FM off)*/
    while(0u == CPMUFLG_LOCK && 0u == CPMUFLG_UPOSC)
    {
        /* Wait until the oscillator is qualified by the PLL */
    }
    CPMUFLG = 0xFF;//Clear all flags in the CPMUFLG register to be able to detect any future status bit change
    /* CPMUPROT: PROT=0 */
    CPMUPROT = 0x00U;                    /* Enable protection of clock configuration registers */
    /* CPMUCOP: RSBCK=0,WRTMASK=0 */
    CPMUINT_OSCIE = 0;
    CPMUINT_LOCKIE = 0;
    CPMUINT_RTIE = 0;
}


//PLL_PEE_Init /* Set Freq = 1 MHz */
void RTI_Init(void)
{
  CPMURTI_RTDEC = 1;
  CPMURTI_RTR = 0b0101001;//1M/(50*1000)=20, 50ms
  CPMUINT_RTIE = 1;
}


void DEV_Init(void)
{
  IO_Init();
  PLL_PEE_Init();
}





















