/***************************************************************************
File Name: dcm.c

Modification Record:
2013-03-09, lzy, add diag_mode

****************************************************************************/

#include "dcm.h"
#include "dcm_cbk.h"
//#include "GlobalInterfaceVar.h"
//#include "Dem_data.h"

uint8_t ucSeed[4];
uint8_t DCM_m_st_SecurityPassed = 0;
uint8_t DCM_m_st_SeedRequest = 0;
uint8_t DCM_m_ct_SecurityFail = 0;




void Dcm_StartDefaultDiagMode()
{
	diag_tx_buff[0] = 0x40+SID_StartDiagnosticSession;
    diag_tx_buff[1] = LID_SDS_DefaultDiagMode;
	tx_state.data_length = 0x02;
	Diag_Transmit();
}


void Dcm_StartEOLProgrammingMode()
{
	if( Appl_EOLConditionCheck() )
	{
		tx_state.data_length = 0x03;
		diag_tx_buff[0] = 0x7f;
	    diag_tx_buff[1] = SID_StartDiagnosticSession;
	    diag_tx_buff[2] = NEG_ACK_CONDITIONSNOTCORRECT;
	}
	else
	{
		tx_state.data_length = 0x02;
		diag_tx_buff[0] = 0x40+SID_StartDiagnosticSession;
    	diag_tx_buff[1] = LID_SDS_EndOfLineProgrammingMode;
	}
	Diag_Transmit();
}

void Dcm_StartProgrammingMode()
{
	tx_state.data_length = 0x03;
	diag_tx_buff[0] = 0x7f;
    diag_tx_buff[1] = SID_StartDiagnosticSession;
    diag_tx_buff[2] = NEG_ACK_SERVICENOTSUPPORTED;
	Diag_Transmit();
}

void Dcm_StartDevelopmentMode()
{
	tx_state.data_length = 0x03;
	diag_tx_buff[0] = 0x7f;
    diag_tx_buff[1] = SID_StartDiagnosticSession;
    diag_tx_buff[2] = NEG_ACK_SERVICENOTSUPPORTED;
	Diag_Transmit();
}
        
        









void Dcm_RequestSeed(void)
{
	  uint32_t tmp;
	
    tmp = OSTimeGet();
    ucSeed[0] = (uint8_t) (tmp>>24 & 0xff);
    if( ucSeed[0] == 0x00 )
        ucSeed[0] = 0xea;
    ucSeed[1] = (uint8_t) ((tmp>>16) & 0xff);
    ucSeed[2] = (uint8_t) ((tmp>>8) & 0xff);
    ucSeed[3] = (uint8_t) ((tmp>>0) & 0xff);
    
  	diag_tx_buff[0] = 0x40+SID_SecurityAccess;
  	diag_tx_buff[1] = 01;
  	diag_tx_buff[2] = ucSeed[0];
  	diag_tx_buff[3] = ucSeed[1];
  	diag_tx_buff[4] = ucSeed[2];
  	diag_tx_buff[5] = ucSeed[3];
  	tx_state.data_length = 6; 
  	Diag_Transmit();
    DCM_m_st_SeedRequest = 1;    
}

void Dcm_SendKey(uint32_t key)
{
	uint32_t seed;
	seed = (uint32_t)ucSeed[0];
	seed <<= 8;
	seed += ucSeed[1];
	seed <<= 8;
	seed += ucSeed[2];
	seed <<= 8;
	seed += ucSeed[3];
	if( (DCM_m_ct_SecurityFail<1) && (DCM_m_st_SeedRequest) && (Appl_SeedKeyMatched(seed, key)) )
	{
		DCM_m_st_SecurityPassed = 1;
	  	diag_tx_buff[0] = 0x40+SID_SecurityAccess;
	  	diag_tx_buff[1] = 02;
	  	tx_state.data_length = 2; 
	}
	else 
	{
		DCM_m_ct_SecurityFail = 10;
		DCM_m_st_SecurityPassed = 0;
	  	diag_tx_buff[0] = 0x7f; 
	  	diag_tx_buff[1] = SID_SecurityAccess;
	  	diag_tx_buff[2] = NEG_ACK_INVALIDKEY;
	  	tx_state.data_length = 3;
	}
	
  	Diag_Transmit();
	DCM_m_st_SeedRequest = 0;	
}

void Dcm_ReadActiveDTC(void)//ERCUÊÕµ½Ô¶³Ì¿ØÖÆÃüÁîºó£¬»áÏòENC8·¢ËÍÏàÓ¦µÄÇëÇó±¨ÎÄ£¬²¢µÈ´ýENC8µÄÓ¦´ð¡£
{/*
  tx_state.data_length = Dem_DtcAssembly((uint8_t *)&diag_tx_buff[1],ACTIVE_DTC) + 1;
  diag_tx_buff[0] = 0x40+SID_ReadDTC;
  Diag_Transmit();
*/}



void Dcm_ReadInactiveDTC(void)//ERCUÊÕµ½Ô¶³Ì¿ØÖÆÃüÁîºó£¬»áÏòENC8·¢ËÍÏàÓ¦µÄÇëÇó±¨ÎÄ£¬²¢µÈ´ýENC8µÄÓ¦´ð¡
{/*
  tx_state.data_length = Dem_DtcAssembly((uint8_t *)&diag_tx_buff[1],INACTIVE_DTC) + 1;
  diag_tx_buff[0] = 0x40+SID_ReadDTC;
  Diag_Transmit();
*/}

void Dcm_ReadAllDTC(void)//ERCUÊÕµ½Ô¶³Ì¿ØÖÆÃüÁîºó£¬»áÏòENC8·¢ËÍÏàÓ¦µÄÇëÇó±¨ÎÄ£¬²¢µÈ´ýENC8µÄÓ¦´ð¡
{/*
  tx_state.data_length = Dem_DtcAssembly((uint8_t *)&diag_tx_buff[1],ALL_DTC) + 1;
  diag_tx_buff[0] = 0x40+SID_ReadDTC;
  Diag_Transmit();
*/}

void Dcm_ReadDataByLocalId(uint8_t lid)
{//ERCUÊÕµ½Ô¶³Ì¿ØÖÆÃüÁîºó£¬»áÏòENC8·¢ËÍÏàÓ¦µÄÇëÇó±¨ÎÄ£¬²¢µÈ´ýENC8µÄÓ¦´ð¡

	if(!DCM_m_st_SecurityPassed)
	{
	  	diag_tx_buff[0] = 0x7f;
	  	diag_tx_buff[1] = SID_ReadDataByLocalId;
	  	diag_tx_buff[2] = NEG_ACK_ACCESSDENIED;
	  	tx_state.data_length = 3; 
  		Diag_Transmit();
  		return;
	}

	switch(lid)
	{/*
		case 0x00:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(AirSplyPRaw >> 8);
			diag_tx_buff[3] = (uint8_t)(AirSplyPRaw & 0xff);
			tx_state.data_length = 4; 
			break;
		case 0x01:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(CluActrPosnActRaw >> 8);
			diag_tx_buff[3] = (uint8_t)(CluActrPosnActRaw & 0xff);
			tx_state.data_length = 4; 
			break;
		case 0x02:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(GateActrPosnActRaw >> 8);
			diag_tx_buff[3] = (uint8_t)(GateActrPosnActRaw & 0xff);
			tx_state.data_length = 4; 
			break;
		case 0x03:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(GearActrPosnActRaw >> 8);
			diag_tx_buff[3] = (uint8_t)(GearActrPosnActRaw & 0xff);
			tx_state.data_length = 4; 
			break;
		case 0x04:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(RngGrpActrPosnSwtHiRaw >> 8);
			diag_tx_buff[3] = (uint8_t)(RngGrpActrPosnSwtHiRaw & 0xff);
			tx_state.data_length = 4; 
			break;
		case 0x05:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(RngGrpActrPosnSwtLoRaw >> 8);
			diag_tx_buff[3] = (uint8_t)(RngGrpActrPosnSwtLoRaw & 0xff);
			tx_state.data_length = 4; 
			break;
		case 0x06:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(SpltGrpActrPosnSwtHiRaw >> 8);
			diag_tx_buff[3] = (uint8_t)(SpltGrpActrPosnSwtHiRaw & 0xff);
			tx_state.data_length = 4; 
			break;
		case 0x07:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(SpltGrpActrPosnSwtLoRaw >> 8);
			diag_tx_buff[3] = (uint8_t)(SpltGrpActrPosnSwtLoRaw & 0xff);
			tx_state.data_length = 4; 
			break;
		case 0x08:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(TrsmInSpdRaw >> 8);
			diag_tx_buff[3] = (uint8_t)(TrsmInSpdRaw & 0xff);
			tx_state.data_length = 4; 
			break;
		case 0x09:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(TrsmOilTRaw >> 8);
			diag_tx_buff[3] = (uint8_t)(TrsmOilTRaw & 0xff);
			tx_state.data_length = 4; 
			break;
		case 0x0A:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(TrsmOutSpdRaw >> 8);
			diag_tx_buff[3] = (uint8_t)(TrsmOutSpdRaw & 0xff);
			tx_state.data_length = 4; 
			break;
		case 0x0B:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(IgnitionStsAct);
			tx_state.data_length = 3; 
			break;
		case 0x0C:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(TrsmCtrlTestSts);
			tx_state.data_length = 3; 
			break;
		case 0x0D:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(TrsmOutSpdPhase);
			tx_state.data_length = 3; 
			break;
		case 0x0E:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(CluActrVlvDutyCyc1);
			tx_state.data_length = 3; 
			break;
		case 0x0F:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(CluActrVlvDutyCyc2);
			tx_state.data_length = 3; 
			break;
		case 0x10:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(CluActrVlvDutyCyc3);
			tx_state.data_length = 3; 
			break;
		case 0x11:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(CluActrVlvDutyCyc4);
			tx_state.data_length = 3; 
			break;
		case 0x12:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(CluActrVlvFrqPwm1);
			tx_state.data_length = 3; 
			break;
		case 0x13:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(CluActrVlvFrqPwm2);
			tx_state.data_length = 3; 
			break;
		case 0x14:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(CluActrVlvFrqPwm3);
			tx_state.data_length = 3; 
			break;
		case 0x15:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(CluActrVlvFrqPwm4);
			tx_state.data_length = 3; 
			break;
		case 0x16:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(GateActrVlvCtl1);
			tx_state.data_length = 3; 
			break;
		case 0x17:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(GateActrVlvCtl2);
			tx_state.data_length = 3; 
			break;
		case 0x18:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(GearActrPLvlVlvCtl1);
			tx_state.data_length = 3; 
			break;
		case 0x19:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(GearActrVlvCtl1);
			tx_state.data_length = 3; 
			break;
		case 0x1A:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(GearActrVlvCtl2);
			tx_state.data_length = 3; 
			break;
		case 0x1B:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(GearActrVlvCtl3);
			tx_state.data_length = 3; 
			break;
		case 0x1C:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(RngGrpVlvCtl1);
			tx_state.data_length = 3; 
			break;
		case 0x1D:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(RngGrpVlvCtl2);
			tx_state.data_length = 3; 
			break;
		case 0x1E:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(SpltGrpVlvCtl1);
			tx_state.data_length = 3; 
			break;
		case 0x1F:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(SpltGrpVlvCtl2);
			tx_state.data_length = 3; 
			break;
		case 0x20:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(TrsmCtlSt);
			tx_state.data_length = 3; 
			break;
		case 0x21:		
			diag_tx_buff[0] = 0x40+SID_ReadDataByLocalId;
			diag_tx_buff[1] = lid;
			diag_tx_buff[2] = (uint8_t)(VehDrvgSt);
			tx_state.data_length = 3; 
			break;*/
			
			
		default:
			diag_tx_buff[0] = 0x7f;
			diag_tx_buff[1] = SID_ReadDataByLocalId;
			diag_tx_buff[2] = NEG_ACK_REQUESTOUTOFRANGE;
			tx_state.data_length = 3; 
			break;
	}
  	Diag_Transmit();


}

void Dcm_StopRoutineByLocalId(uint8_t lid)
{
	uint8_t exitStatus;
	if( Appl_StopRoutine(lid, &exitStatus) )
	{
	  	diag_tx_buff[0] = 0x7f; 
	  	diag_tx_buff[1] = SID_StopRoutineByLocalId;
	  	diag_tx_buff[2] = NEG_ACK_CONDITIONSNOTCORRECT;
	  	tx_state.data_length = 3;
	}
	else
	{
	  	diag_tx_buff[0] = 0x40 + SID_StopRoutineByLocalId; 
	  	diag_tx_buff[1] = lid;
	  	diag_tx_buff[2] = exitStatus;
	  	tx_state.data_length = 3;
	}
  	Diag_Transmit();
}

void Dcm_ReqRoutineResultByLocalId(uint8_t lid)
{
	uint8_t exitStatus;
	if( Appl_ReqRoutineResult(lid, &exitStatus) )
	{
	  	diag_tx_buff[0] = 0x7f; 
	  	diag_tx_buff[1] = SID_ReqRoutineResultByLocalId;
	  	diag_tx_buff[2] = NEG_ACK_CONDITIONSNOTCORRECT;
	  	tx_state.data_length = 3;
	}
	else
	{
	  	diag_tx_buff[0] = 0x40 + SID_ReqRoutineResultByLocalId; 
	  	diag_tx_buff[1] = lid;
	  	diag_tx_buff[2] = exitStatus;
	  	tx_state.data_length = 3;
	}
  	Diag_Transmit();
}

void Dcm_StartRoutineByLocalId( uint8_t lid, uint8_t par_size, uint8_t *par )
{
	uint8_t exitStatus;

	if(!DCM_m_st_SecurityPassed)
	{
	  	diag_tx_buff[0] = 0x7f;
	  	diag_tx_buff[1] = SID_StartRoutineByLocalId;
	  	diag_tx_buff[2] = NEG_ACK_ACCESSDENIED;
	  	tx_state.data_length = 3; 
  		Diag_Transmit();
  		return;
	}

	if( Appl_StartRoutineByLocalId( lid, par_size, par, &exitStatus ) )
	{
	  	diag_tx_buff[0] = 0x7f; 
	  	diag_tx_buff[1] = SID_StartRoutineByLocalId;
	  	diag_tx_buff[2] = NEG_ACK_CONDITIONSNOTCORRECT;
	  	tx_state.data_length = 3;
	}
	else
	{
	  	diag_tx_buff[0] = 0x40 + SID_StartRoutineByLocalId; 
	  	diag_tx_buff[1] = lid;
	  	diag_tx_buff[2] = exitStatus;
	  	tx_state.data_length = 3;
	}
	
  	Diag_Transmit();
}

void Dcm_TesterPresent(uint8_t ack)
{
	if(ack)
	{
	  	diag_tx_buff[0] = 0x40 + SID_TesterPresent; 
	  	tx_state.data_length = 1;
  		Diag_Transmit();
	}
}
        
void Dcm_ReadMemoryByAddress(uint32_t address, uint8_t size)
{
	
}





