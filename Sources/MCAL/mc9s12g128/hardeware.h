/*
*********************************************************************************************************
* File       : hardware.h
*Description : s12g128的硬件设置头文件
*            : 适用芯片mc9s12g128
* Note(s)    : 
*********************************************************************************************************
*/
#ifndef _HARDWARE_H
#define _HARDWARE_H

#include "derivative.h"
#include "hidef.h"

#define OSC_FRQ_MHZ 4 /*[MHz]*/
#define BUS_FRQ_MHZ 24 /*[MHz]*/ 

#define LED1_D2_ON()  {PTP_PTP6 = 0;}
#define LED1_D2_OFF() {PTP_PTP6 = 1;}
#define LED1_D2_DIR_OUT() {DDRP_DDRP6 = 1;}
#define LED1_D2_TOOGLE()  {PTP_PTP6 = !PTP_PTP6;}

#define BAT_ON()      {PTT_PTT6 = 1;}
#define BAT_OFF()     {PTT_PTT6 = 0;}
#define BAT_DIR_OUT() {DDRT_DDRT6 = 1;}

#define MG2639_PWR     PTJ_PTJ5
#define MG2639_PWR_DDR DDRJ_DDRJ5
#define MG2639_RST     PTJ_PTJ6
#define MG2639_RST_DDR DDRJ_DDRJ6

#define MG2639_PWRKEY_N_LOW()     {MG2639_PWR = 1;}  //MCU high,MG2639 PWRKEY_N low
#define MG2639_PWRKEY_N_HIGH()    {MG2639_PWR = 0;}
#define MG2639_PWRKEY_N_DIR_OUT() {MG2639_PWR_DDR = 1;}
#define MG2639_SYSRST_N_LOW()     {MG2639_RST = 1;}
#define MG2639_SYSRST_N_HIGH()    {MG2639_RST = 0;}
#define MG2639_SYSRST_N_DIR_OUT() {MG2639_RST_DDR = 1;}

/***全局变量定义*********************************************/


/***全局函数的定义*******************************************/



/***变量的全局声明*******************************************/


/***函数的全局声明*******************************************/
extern void IO_Init(void);
extern void PLL_PEE_Init(void);   
extern void RTI_Init(void);
extern void DEV_Init(void);


















#endif


















