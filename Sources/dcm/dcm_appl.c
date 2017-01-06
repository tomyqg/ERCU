/********************************************************************
File name: dcm_appl.c


*********************************************************************/
#include "dcm.h"


#define PROGRAM_CONST 0x8dea743b





uint8_t TEST_m_st_CurrentMode = 0;
uint8_t TEST_m_st_Request = 0;
uint8_t TEST_m_st_ShutoffDieselInj = 0;
uint8_t TEST_m_st_OpenLngValve = 0;
uint8_t TEST_m_st_ShutoffRequest = 0;
uint32_t TEST_m_ti_LngInjTime = 0;
uint32_t TEST_m_ti_NGInjVlvTest = 0; 

uint8_t Appl_EOLConditionCheck(void);
uint8_t Appl_SeedKeyMatched(uint32_t seed, uint32_t key); 
uint8_t Appl_StopRoutine(uint8_t lid, uint8_t *exitStatus);
uint8_t Appl_ReqRoutineResult(uint8_t lid, uint8_t *exitStatus);
uint8_t Appl_StartRoutineByLocalId( uint8_t lid, uint8_t par_size, uint8_t *par, uint8_t *exitStatus );

uint8_t Appl_EOLConditionCheck(void)
{
	return 1;
}


uint8_t Appl_SeedKeyMatched(uint32_t seed, uint32_t key)
{
   uint32_t tmp1;
   tmp1 = ~seed;
   tmp1 &= PROGRAM_CONST;
   tmp1 += 0x8501; 
   if( tmp1 == key ) return 1;
   else return 0;
}
 
uint8_t Appl_StopRoutine(uint8_t lid, uint8_t *exitStatus)
{
	TEST_m_st_Request = 0;
	TEST_m_st_ShutoffRequest = 0;
	return 0;
}


uint8_t Appl_ReqRoutineResult(uint8_t lid, uint8_t *exitStatus)
{
	return 1;
}


uint8_t Appl_StartRoutineByLocalId( uint8_t lid, uint8_t par_size, uint8_t *par, uint8_t *exitStatus )
{
	switch(lid)
	{
		case 0x00:	//LNG valve test
		break;
	
		default:
		TEST_m_st_Request = 0;
		return 1;
		break;
	}
		
	return 0;
}

