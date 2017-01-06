
#ifndef DCM_H
#define DCM_H

#include "diag.h"
//#include "dem.h"
//#include "Rte_Type.h"


#define DCM_READ_ECU_ID_SYS_HARDWARE      0x82U
#define DCM_READ_ECU_ID_APPSW_VERSION     0x83U
#define DCM_READ_ECU_ID_BOOTSW_VERSION    0x84U
#define DCM_READ_ECU_ID_BSW_VERSION       0x88U
#define DCM_READ_ECU_ID_SW_RELEASE_DATE   0x89U

/* Calibration Label */
/* Calibration Label */
extern const uint8 Dcm_AppSwVersion[9];
extern const uint8 Dcm_SysHardwareID[10];
extern const uint8 Dcm_BootSwVersion[6];
extern const uint8 Dcm_BSWVersion[5];
extern const uint8 Dcm_SwReleaseDate[6];

extern uint8_t DCM_m_ct_SecurityFail;

void Dcm_EcuReset(void);
void Dcm_ClearFaultMemory(void);
void Dcm_ReadECUId(uint8_t id_option);
void Dcm_ReadDataByLocalId(uint8_t lid);
void Dcm_RequestSeed(void);
void Dcm_SendKey(uint32_t key);
void Dcm_StartRoutineByLocalId( uint8_t lid, uint8_t par_size, uint8_t *par );
void Dcm_StopRoutineByLocalId(uint8_t lid);
void Dcm_ReqRoutineResultByLocalId(uint8_t lid);
void Dcm_TesterPresent(uint8_t ack);
void Dcm_ReadMemoryByAddress(uint32_t address, uint8_t size);


#endif /* #ifndef DCM_H */
