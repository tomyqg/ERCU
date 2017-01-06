;********************************************************************************************************
;                                               uC/OS-II
;                                         The Real-Time Kernel
;
;                         (c) Copyright 2002, Jean J. Labrosse, Weston, FL
;                                          All Rights Reserved
;
;
;                                       PAGED S12 Specific code
;                                            (METROWERKS)
;
; File         : OS_CPU_A.S
; Notes        : THIS FILE *MUST* BE LINKED INTO NON_BANKED MEMORY!
; 修改         : 飞思卡尔社区-hairong 
;********************************************************************************************************

NON_BANKED:       section  

;********************************************************************************************************
;                                           I/O PORT ADDRESSES
;********************************************************************************************************

CRGFLG_RTIF:      equ	   $0037		     ; Address of CRGFLG reg  定时器的标志

;********************************************************************************************************
;                                        PPAGE register ADDRESSES
;********************************************************************************************************

PPAGE:            equ    $0015         ; Addres of PPAGE register (assuming MC9S12G128 part)
;RPAGE:            equ    $0016         ; Addres of RPAGE register (assuming MC9S12G128 part)
;EPAGE:            equ    $0017         ; Addres of EPAGE register (assuming MC9S12G128 part)
;GPAGE:            equ    $0010         ; Addres of GPAGE register (assuming MC9S12G128 part)
_SCI2DRL_:        equ    $00EF
_SCI1DRL_:        equ    $00E7
_SCI2SR_:         equ    $00EC
_SCI1SR_:         equ    $00D4

;********************************************************************************************************
;                                          PUBLIC DECLARATIONS
;********************************************************************************************************
   
    xdef   OSStartHighRdy
    xdef   OSCtxSw
    xdef   OSIntCtxSw
    xdef   OSTickISR
    xdef   SCI1RcvIsr
    xdef   SCI2RcvIsr
    
;********************************************************************************************************
;                                         EXTERNAL DECLARATIONS
;                                          外部函数声明并引用
;********************************************************************************************************
   
    xref   OSIntExit
    xref   OSIntNesting  
    xref   OSPrioCur    
    xref   OSPrioHighRdy
    xref   OSRunning   
    xref   OSTaskSwHook 
    xref   OSTCBCur     
    xref   OSTCBHighRdy 
    xref   OSTickISR_Handler 
    xref   OSTimeTick
    
    
    xref   Sci1IntHandle
    xref   Sci2IntHandle
    
;********************************************************************************************************
;                               START HIGHEST PRIORITY TASK READY-TO-RUN
;                               (启动最高优先级任务)
; Description : This function is called by OSStart() to start the highest priority task that was created
;               by your application before calling OSStart().
;
; Arguments   : none
;
; Note(s)     : 1) The stack frame is assumed to look as follows:
;   
;                  OSTCBHighRdy->OSTCBStkPtr +  0       PPAGE
;                                            +  1       CCR
;                                            +  2       B
;                                            +  3       A
;                                            +  4       X (H)
;                                            +  5       X (L)
;                                            +  6       Y (H)
;                                            +  7       Y (L)
;                                            +  8       PC(H)
;                                            +  9       PC(L)
;
;               2) OSStartHighRdy() MUST:
;                      a) Call OSTaskSwHook() then,
;                      b) Set OSRunning to TRUE,
;                      c) Switch to the highest priority task by loading the stack pointer of the
;                         highest priority task into the SP register and execute an RTI instruction.
;********************************************************************************************************

OSStartHighRdy:

    call   OSTaskSwHook                ;  4~, Invoke user defined context switch hook            

    ldab   #$01                        ;  2~, Indicate that we are multitasking
    stab   OSRunning                   ;  4~                  

    ldx    OSTCBHighRdy                ;  3~, Point to TCB of highest priority task ready to run 
    lds    0,x                         ;  3~, Load SP into 68HC12
    
    pula   
    staa   PPAGE
                                     
    rti                                ;  4~, Run task                                           


	


;********************************************************************************************************
;                                       TASK LEVEL CONTEXT SWITCH
;
; Description : This function is called when a task makes a higher priority task ready-to-run.
;
; Arguments   : none
;
; Note(s)     : 1) Upon entry, 
;                  OSTCBCur     points to the OS_TCB of the task to suspend
;                  OSTCBHighRdy points to the OS_TCB of the task to resume
;
;               2) The stack frame of the task to suspend looks as follows:
;
;                  SP +  0       PC(H)
;                     +  1       PC(L)
;
;               3) The stack frame of the task to resume looks as follows:
; 
;                  OSTCBHighRdy->OSTCBStkPtr +  0       PPAGE
;                                            +  1       CCR
;                                            +  2       B
;                                            +  3       A
;                                            +  4       X (H)
;                                            +  5       X (L)
;                                            +  6       Y (H)
;                                            +  7       Y (L)
;                                            +  8       PC(H)
;                                            +  9       PC(L)
;********************************************************************************************************

OSCtxSw:
    pshy                                    ; Manually push preempted task's context on to the stack
    pshx
    psha
    pshb    
    pshc
    
    ldaa   PPAGE
    psha  
                                                                                      
    ldy    OSTCBCur                   ;  3~, OSTCBCur->OSTCBStkPtr = Stack Pointer     
    sts    0,y                        ;  3~,                                           

    call   OSTaskSwHook               ;  4~, Call user task switch hook                       
    
    ldx    OSTCBHighRdy               ;  3~, OSTCBCur  = OSTCBHighRdy
    stx    OSTCBCur                   ;  3~                         
    
    ldab   OSPrioHighRdy              ;  3~, OSPrioCur = OSPrioHighRdy                        
    stab   OSPrioCur                  ;  3~
    
    lds    0,x                        ;  3~, Load SP into 68HC12                              
    
    pula   
    staa   PPAGE
    
    rti                               ;  8~, Run task                                         


;********************************************************************************************************
;                                    INTERRUPT LEVEL CONTEXT SWITCH
;
; Description : This function is called by OSIntExit() to perform a context switch to a task that has
;               been made ready-to-run by an ISR. The PPAGE register of the preempted task has already 
;               been stacked during the start of the ISR that is currently running.
;
; Arguments   : none
;********************************************************************************************************

OSIntCtxSw:

    call   OSTaskSwHook               ;  4~, Call user task switch hook                
    
    ldx    OSTCBHighRdy               ;  3~, OSTCBCur  = OSTCBHighRdy
    stx    OSTCBCur                   ;  3~                         
    
    ldab   OSPrioHighRdy              ;  3~, OSPrioCur = OSPrioHighRdy                        
    stab   OSPrioCur                  ;  3~
    
    lds    0,x                        ;  3~, Load the SP of the next task
    
    pula 
    staa   PPAGE
                                                           
    rti                               ;  8~, Run task                                  


;********************************************************************************************************
;                                           SYSTEM TICK ISR
;
; Description : This function is the ISR used to notify uC/OS-II that a system tick has occurred.  You 
;               must setup the S12XE's interrupt vector table so that an OUTPUT COMPARE interrupt 
;               vectors to this function.
;
; Arguments   : none
;
; Notes       :  1) The 'tick ISR' assumes the we are using the Output Compare specified by OS_TICK_OC
;                   (see APP_CFG.H and this file) to generate a tick that occurs every OS_TICK_OC_CNTS 
;                   (see APP_CFG.H) which corresponds to the number of FRT (Free Running Timer) 
;                   counts to the next interrupt.
;
;                2) All USER interrupts should be modeled EXACTLY like this where the only
;                   line to be modified is the call to your ISR_Handler and perhaps the call to
;                   the label name OSTickISR1.
;********************************************************************************************************

OSTickISR:

    ldaa   PPAGE
    psha   

    inc    OSIntNesting                ;  4~, Notify uC/OS-II about ISR

    ldab   OSIntNesting                ;  4~, if (OSIntNesting == 1) {    
    cmpb   #$01                        ;  2~
    bne    OSTickISR1                  ;  3~

    ldy    OSTCBCur                    ;  3~,     OSTCBCur->OSTCBStkPtr = Stack Pointer     
    sts    0,y                         ;  3~, }                                          

OSTickISR1:
    
	  bset   CRGFLG_RTIF,#128            ;  add code to clr the interrupt flag

    call   OSTimeTick                  ;  调用时钟节拍处理

;    cli                               ;  2~, Enable interrupts to allow interrupt nesting
       
    call   OSIntExit                   ;  6~+, Notify uC/OS-II about end of ISR
    
    pula 
    staa   PPAGE
                                                                   
    rti                                ;  12~, Return from interrupt, no higher priority tasks ready.
 









 
;********************************************************************************************************
;                                           SCI1 ISR  (GPS) 
;********************************************************************************************************
SCI1RcvIsr:

    ldaa   PPAGE
    psha   

    inc    OSIntNesting                ;  4~, Notify uC/OS-II about ISR

    ldab   OSIntNesting                ;  4~, if (OSIntNesting == 1) {    
    cmpb   #$01                        ;  2~
    bne    lable1                  ;  3~

    ldy    OSTCBCur                    ;  3~,     OSTCBCur->OSTCBStkPtr = Stack Pointer     
    sts    0,y
    
lable1:
    ldab  _SCI1SR_
    ldab  _SCI1DRL_
    call  Sci1IntHandle
    call  OSIntExit                   ;  6~+, Notify uC/OS-II about end of ISR
    
    pula 
    staa   PPAGE
                                                                   
    rti                                ;  12~, Return from interrupt, no higher priority tasks ready.
    
    
    
;********************************************************************************************************
;                                           SCI2 ISR  (SMS) 
;********************************************************************************************************
SCI2RcvIsr:

    ldaa   PPAGE
    psha   

    inc    OSIntNesting                ;  4~, Notify uC/OS-II about ISR

    ldab   OSIntNesting                ;  4~, if (OSIntNesting == 1) {    
    cmpb   #$01                        ;  2~
    bne    lable2                  ;  3~

    ldy    OSTCBCur                    ;  3~,     OSTCBCur->OSTCBStkPtr = Stack Pointer     
    sts    0,y
    
lable2:
    ldab  _SCI2SR_
    ldab  _SCI2DRL_
    call  Sci2IntHandle
    call  OSIntExit                   ;  6~+, Notify uC/OS-II about end of ISR
    
    pula 
    staa   PPAGE
                                                                   
    rti                                ;  12~, Return from interrupt, no higher priority tasks ready.
 
     