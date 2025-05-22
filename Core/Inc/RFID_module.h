#ifndef __RFID_522_H
#define __RFID_522_H

#include "rc522.h"

#define BUFF_SIZE  256
typedef struct 
{
    uint8_t buff[BUFF_SIZE];
    uint8_t uid[UID_SIZE];
    uint8_t data[MAX_LEN];
    uint8_t defkey[KEY_LEN];
    timeout_t timer;
    
} RFID_522_struct_t;

void RFID_init(void);
void RFID_reinit(void);
void RFID_close(void);

uint8_t RFID_WriteReadBlock(uint8_t addrBlock,
                            uint8_t *dataTRANS, uint8_t *dataREC,
                            uint8_t *key, uint8_t *uid);
uint8_t RFID_WriteBlock(uint8_t addrBlock, uint8_t *data, uint8_t *key, uint8_t *uid);
uint8_t RFID_ReadBlock(uint8_t addrBlock, uint8_t *data, uint8_t *key, uint8_t *uid);
uint8_t RFID_ReadSector(uint8_t addrSector, uint8_t *data, uint8_t *key, uint8_t *uid);

uint8_t RFID_ChangeKey(uint8_t addrAuth, uint8_t *old_key, uint8_t *new_param, uint8_t *uid);
uint8_t RFID_getUID(uint8_t *uid_buff);

#endif /* __RFID_522_H */