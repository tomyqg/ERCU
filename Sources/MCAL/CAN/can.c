#include "can.h"
#include "can_app.h"

#if  DIAG_ENABLE > 0
#include "diag.h"
#endif

const can_msg_config_t can_msgs_cfg_array[] = 
{
  {ENC8_DIAG_RESPONSE_ID, TRUE, FALSE, ENC8RespMsgDecode},
  {ERCU_DIAG_REQUEST_ID, TRUE, TRUE, NULL},    
  #if  DIAG_ENABLE > 0
    {MSG_DIAG_RX_ID, TRUE, FALSE, MsgDIAG_RX_Decode},//MSG_DIAG_RX_RECEIVE
    {MSG_DIAG_TX_ID, TRUE, TRUE, NULL},    //MSG_DIAG_TX_SEND
  #endif
  {RCU_P_ID, TRUE, TRUE, RCU_P_Msg_pack},    //RCU_P_MSG_SEND
  {FC1_ID, TRUE, FALSE, NULL/*FC1_Msg_unpack*/},  /*FC1_Msg RECEIVE*/ 
  {AQ1_ID, TRUE, FALSE, NULL/*AQ1_Msg_unpack*/},  /*AQ1_MSG_RECEIVE*/ 
  {SIG1_ID, TRUE, FALSE, NULL/*SIG1_Msg_unpack*/}, /*SIG1_MSG_RECEIVE*/
  //{DATE_ID, TRUE, FALSE, Date_Msg_unpack}, /*DATE_MSG_RECEIVE*/ 
};

can_msg_buf_cfg_t can_msgs_buf_array[] =
{
    {CAN_MSG_EMPTY, 8, ENC8RespRxBuff, 0, 0},  
    {CAN_MSG_EMPTY, 8, ERCUReqTxBuff, 0, 0},  
  #if  DIAG_ENABLE > 0
    {CAN_MSG_EMPTY, 8, gMSG_DIAG_RX.data, 0, 0},  //MSG_DIAG_RX_RECEIVE
    {CAN_MSG_EMPTY, 8, gMSG_DIAG_TX.data, 0, 0},  //MSG_DIAG_TX_SEND 不是周期性报文
  #endif
  {CAN_MSG_EMPTY, 8, RCUPresentMsg, 1, 0},  //RCU_P_MSG_SEND
  {0, 8, FC1RxBuff, 1, 0},          /*FC1_Msg RECEIVE*/
  {0, 8, AQ1RxBuff, 1, 0},          /*AQ1_MSG_RECEIVE*/
  {0, 8, SIG1RxBuff, 1, 0},         /*SIG1_MSG_RECEIVE*/
  //{0, 8, DateRxBuff, 1, 0},         /*DATE_MSG_RECEIVE*/
};

/*
 * Function:CAN_Init
 * Param<void>:
 * Return<io_err_t>:
 * REQ IDs:TPMS_SW_BASE_0045,TPMS_SW_BASE_0046,TPMS_SW_BASE_0047(TBD),TPMS_SW_APP_0216
 * Discription:
 * Note: 
 * MODRR1  MODRR0  CAN_0_Rx  CAN_0_Tx ; MODRR3  MODRR2  CAN_4_Rx  CAN_4_Tx
 *   0       0        PM0       PM1   ;    0       0       PJ6       PJ7
 *   0       1        PM2       PM3   ;    0       1       PM4       PM5
 *   1       0        PM4       PM5   ;    1       0       PM6       PM7
 *   1       1        PJ6       PJ7   ;    1       1          RESERVED
*/
io_err_t CAN_Init(void)//250K
{

    CAN_REG(CTL1_CANE) = 1;  /*Enable MSCAN*/
    CAN_REG(CTL0_INITRQ) = 1; /*Request initial module*/
    
    while(!CAN_REG(CTL1_INITAK));/*wait for enter intitial module mode*/
 
    
    CAN_REG(CTL1_CLKSRC) = 0; /*0 for osc clk(here is 4M),1 for bus clk*/
    CAN_REG(CTL1_LISTEN) = 0; /*don't used listen mode*/
    CAN_REG(CTL1_LOOPB)  = 0; /*self test disable*/
    CAN_REG(CTL1_BORM)   = 0; /*enable automatic bus-off recovery*/


    /*set baud-rate based 16 MHz oscillator clock for BRP = 1,else oscillator clock is 8 MHz ,set BRP = 0*/
    /*
    * 0 for 1 mbps
    * 1 for 500kbps, other parameter information based this:Tq = CANCLK/(BRP + 1) = 125ns
    * 3 for 250 kbps
    * 7 for 125 kbps
    */
    CAN_REG(BTR0_BRP)     = 0;  /*Prescale value = BRP + 1*/

    CAN_REG(BTR0_SJW)     = 3;  /*eg. 2: Resync Jump Width = (2 + 1) * Tq = 375 ns*/
    CAN_REG(BTR1_TSEG_10) = 10; /*eg. Tseg1 = Tq * (TSEG_1 + 1) = 1500 ns,81.25% if TSEG_10 = 10 and TSEG_20 = 3, 75%*/
    CAN_REG(BTR1_TSEG_20) = 3;  /*eg. Tseg2 = Tq * (TSEG_1 + 1) = 375 ns*/
    //(10+1)+(3+1)+1=16, 4M/16=250K
    CAN_REG(BTR1_SAMP)    = 0;  /*1 for Three samples per bit, else One*/

    CAN_REG(IDAC_IDAM)   = 0; /*0 for Two 32-bit acceptance filters,but not mask any message*/
    CAN_REG(IDAR0) = 0x00;
    CAN_REG(IDAR1) = 0x00;
    CAN_REG(IDAR2) = 0x00;
    CAN_REG(IDAR3) = 0x00;
    CAN_REG(IDAR4) = 0x00;
    CAN_REG(IDAR5) = 0x00;
    CAN_REG(IDAR6) = 0x00;
    CAN_REG(IDAR7) = 0x00;
    CAN_REG(IDMR0) = 0xFF;
    CAN_REG(IDMR1) = 0xFF;
    CAN_REG(IDMR2) = 0xFF;
    CAN_REG(IDMR3) = 0xFF;
    CAN_REG(IDMR4) = 0xFF;
    CAN_REG(IDMR5) = 0xFF;
    CAN_REG(IDMR6) = 0xFF;
    CAN_REG(IDMR7) = 0xFF;

    /*Exit initial module mode*/
    CAN_REG(CTL0_INITRQ) = 0;

    while(CAN_REG(CTL1_INITAK));/*wait for exit intitial module mode*/
    

    CAN_REG(RFLG_RXF) = 1;   /*Clear rx full flag*/
    CAN_REG(RIER_RXFIE) = 0; /*Disable rx interrupt*/
    CAN_REG(TIER_TXEIE) = 0; /*Disable tx interrupts,but enabled when start tx*/
    /*Enable CAN error and bus-off Interrupt (When Enter or Leave Thoese Error, Interrupt occur)*/
    //CAN_REG(RIER_RSTATE) = 2; /*1 for bus-off only, 2 for Receive-error and bus-off*/
    //CAN_REG(RIER_TSTATE) = 2;
    //CAN_REG(RIER_CSCIE) = 1;
    return IO_ERR_OK;
}





/*
 * Function:CAN_Tx
 * Param<can_msg_t msg_index>: Message information index in can_msgs_cfg_array(global)
 *                             and message buffer information index in can_msgs_buf_array(global)
 * Return<io_err_t>: IO_ERR_OK if transmit message trig or transmit buffer idle(when msg_index is -1), else IO_ERR_BUSY
 * REQ IDs:TPMS_SW_BASE_0048,TPMS_SW_BASE_0049,TPMS_SW_BASE_0050,TPMS_SW_BASE_0051
 * Discription:
 * Note: For check last message is transmit or not, prameter msg_index used -1.
 *       Return IO_ERR_BUSY, means BUSY,else Success.
*/
io_err_t CAN_Tx(can_msg_t msg_index)
{
    uint8_t msg_buffer_index = 0;
    uint8_t idle_buf = 0;

    idle_buf = CAN_REG(TFLG_TXE);
    /*check tx buffer only*/
    if (0 == idle_buf)
    {
        if (CAN_MSG_EMPTY_CHECK(msg_index))
        {
            can_msgs_buf_array[msg_index].msg_st = CAN_MSG_NEW_MASK;
        }
        else
        {
            return IO_ERR_BUSY;
        }
    }    
    else
    {        
        CAN_REG(TBSEL_TX) = idle_buf; /*The lowest numbered bit places the respective transmit buffer in the CANTXFG register space*/
         /*set ID*/
        if (TRUE == can_msgs_cfg_array[msg_index].is_ext_id)
        {
            /*data frame*/
            *((uint32_t *)(&CAN_REG(TXIDR0))) = (((can_msgs_cfg_array[msg_index].id & 0x1FFC0000) << 3) \
                                                                 |0x00180000|((can_msgs_cfg_array[msg_index].id & 0x0003FFFF)<<1));
        }
        else
        {
            /*Data frame*/
            *((uint16_t *)(&CAN_REG(TXIDR0))) = (uint16_t)(can_msgs_cfg_array[msg_index].id << 5);
        }              
     
        /*load buffer*/
        if (can_msgs_cfg_array[msg_index].process_fun != NULL)
        {
            can_msgs_cfg_array[msg_index].process_fun();
        } 
        
        CAN_REG(TXDLR) = can_msgs_buf_array[msg_index].dlc;
        for (msg_buffer_index = 0; msg_buffer_index < can_msgs_buf_array[msg_index].dlc; msg_buffer_index++)
        {
            *((uint8_t *)(&CAN_REG(TXDSR0) + msg_buffer_index)) = can_msgs_buf_array[msg_index].pbuffer[msg_buffer_index];
        }
        
        can_msgs_buf_array[msg_index].msg_st = CAN_MSG_EMPTY;
         
        /*Start transmit message*/
        CAN_REG(TFLG_TXE) = 1;
        //CAN_REG(TIER_TXEIE) = 1;
    }
   
    return IO_ERR_OK;
}

/*
 * Function:CAN_TxCheck
 * Param<void>:
 * Return<void>:
 * REQ IDs:TPMS_SW_BASE_0059
 * Discription:
 * Note: 
*/
void CAN_TxCheck(void)
{
    can_msg_t msg_index;
    
    for (msg_index = 0; msg_index < CAN_MSGS_COUNT; msg_index++)
    {
        if ((TRUE == can_msgs_cfg_array[msg_index].is_tx) && (can_msgs_buf_array[msg_index].msg_st != CAN_MSG_EMPTY))
        {
            (void)CAN_Tx(msg_index);
        }
    }
}

/*
 * Function:CAN_PeriodHandle
 * Param<BOOL tx_rx>:TRUE for handle TX else handle RX
 * Return<void>:
 * REQ IDs:TPMS_SW_BASE_0457
 * Discription:
 * Note: 
*/
void CAN_PeriodHandle(BOOL tx_rx)
{
    can_msg_t msg_index;
    
    for (msg_index = 0; msg_index < CAN_MSGS_COUNT; msg_index++)
    {
      if (can_msgs_buf_array[msg_index].period > 0)
      {
          if (tx_rx == can_msgs_cfg_array[msg_index].is_tx)
          {
              if (can_msgs_buf_array[msg_index].tick > 0)
              {
                  can_msgs_buf_array[msg_index].tick--;
              }

              if (0 == can_msgs_buf_array[msg_index].tick)
              {
                  can_msgs_buf_array[msg_index].tick = can_msgs_buf_array[msg_index].period;
                  if (IO_ERR_OK == CAN_Tx(msg_index))
                  {
                      CAN_MSG_TIMEOUT_CLEAR(msg_index);
                  }
                  else
                  {
                      CAN_MSG_TIMEOUT_SET(msg_index);
                  }
              }
          }
          else
          {
              if (0 != CAN_MSG_CHECK(msg_index))
              {
                  can_msgs_buf_array[msg_index].tick = can_msgs_buf_array[msg_index].period;
                  CAN_MSG_TIMEOUT_CLEAR(msg_index);
              }
              else
              {
                  if (can_msgs_buf_array[msg_index].tick > 0)
                  {
                      can_msgs_buf_array[msg_index].tick--;
                  }
                  if (0 == can_msgs_buf_array[msg_index].tick)
                  {
                      can_msgs_buf_array[msg_index].tick = can_msgs_buf_array[msg_index].period;
                      CAN_MSG_TIMEOUT_SET(msg_index);
                  }
              }
          }
      }
    }
}

/*
 * Function:CAN_Rx
 * Param<void>:
 * Return<void>:
 * REQ IDs:TPMS_SW_BASE_0052,TPMS_SW_BASE_0053,TPMS_SW_BASE_0054,TPMS_SW_BASE_0055,TPMS_SW_BASE_0056,
 *         TPMS_SW_BASE_0057,TPMS_SW_BASE_0058
 * Discription: Receive all message from rx-buffer.
 *              If ID match, store message in corresponding message buffer.
 *              Only check messages in can_msgs_cfg_array, other messages ignore.
 * Note: 
*/
void CAN_Rx(void)
{
  uint8_t  msg_index;
  uint8_t  dt_index;
  uint32_t real_id = 0;
  bool     ext_id = FALSE;

  while (1 == CAN_REG(RFLG_RXF))
  {
      ext_id = CAN_REG(RXIDR1_IDE);
      if (TRUE == ext_id) /*CAN ID type match*/
      {
          real_id = *((uint32_t *)(uint16_t)(&CAN_REG(RXIDR0))), real_id = ((real_id >> 1) & 0x0003FFFF)|((real_id >> 3) & 0x1FFC0000);
      }
      else
      {
          real_id = *((uint16_t *)(uint16_t)(&CAN_REG(RXIDR0))) >> 5;
      }
      /*Search ID config array*/
      for (msg_index = 0; msg_index < CAN_MSGS_COUNT; msg_index++)
      {
          if ((FALSE == can_msgs_cfg_array[msg_index].is_tx) && (can_msgs_cfg_array[msg_index].is_ext_id == ext_id) && (can_msgs_cfg_array[msg_index].id == real_id))
          {
              can_msgs_buf_array[msg_index].dlc = CAN_REG(RXDLR) & 0x0F;
              for (dt_index = 0; dt_index < can_msgs_buf_array[msg_index].dlc; dt_index++)
              {
                  can_msgs_buf_array[msg_index].pbuffer[dt_index] = *((uint8_t *)(&CAN_REG(RXDSR0) + dt_index));
              }
              
              if (can_msgs_cfg_array[msg_index].process_fun != NULL)
              {
                  can_msgs_cfg_array[msg_index].process_fun();
              }
              
              if (0 != (can_msgs_buf_array[msg_index].msg_st & CAN_MSG_NEW_MASK))
              {
                  can_msgs_buf_array[msg_index].msg_st = CAN_MSG_NEW_OVER;
              }
              else
              {
                  can_msgs_buf_array[msg_index].msg_st = CAN_MSG_NEW_MASK;
              }
          }
      }
      CAN_REG(RFLG_RXF) = 1;
  }
}

