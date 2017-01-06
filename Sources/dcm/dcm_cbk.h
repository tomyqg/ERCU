#ifndef DCM_CBK_H
#define DCM_CBK_H

extern uint8_t Appl_EOLConditionCheck(void);
extern uint8_t Appl_EcuResetCheck(void);
extern uint8_t Appl_SeedKeyMatched(uint32_t seed, uint32_t key); 
extern uint8_t Appl_StopRoutine(uint8_t lid, uint8_t *exitStatus);
extern uint8_t Appl_ReqRoutineResult(uint8_t lid, uint8_t *exitStatus);
extern uint8_t Appl_StartRoutineByLocalId( uint8_t lid, uint8_t par_size, uint8_t *par, uint8_t *exitStatus );


#endif
