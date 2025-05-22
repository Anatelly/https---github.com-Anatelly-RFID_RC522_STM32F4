#include "include.h"

static void RFID_defaultKey(uint8_t *key);

void RFID_init(void)
{
    MFRC522_Init();
    software_timer_start(&rfid.timer, 500);
    RFID_defaultKey(rfid.defkey);
}

void RFID_reinit(void)
{
    if(software_timer(&rfid.timer))
        MFRC522_Init();
}

void RFID_close(void)
{
    MFRC522_Halt();
}

uint8_t RFID_getUID(uint8_t *uid_buff)
{
    if (MFRC522_Request(PICC_REQIDL, uid_buff) == MI_OK) {
      if(MFRC522_Anticoll(uid_buff) == MI_OK)
        return MI_OK;
    }
    return MI_ERR;
}

uint8_t RFID_WriteReadBlock(uint8_t addrBlock,
                            uint8_t *dataTRANS, uint8_t *dataREC,
                            uint8_t *key, uint8_t *uid)
{
    uint8_t addrAuth = addrBlock + (3-addrBlock%4); //вычисляем блок аутентификации для конкретного сектора
    MFRC522_SelectTag(rfid.uid);
    if(MFRC522_Auth(PICC_AUTHENT1A, addrAuth, key, uid) == MI_OK)
        if(MFRC522_Write(addrBlock, dataTRANS) == MI_OK)
            if(MFRC522_Read(addrBlock, dataREC) == MI_OK)
                return MI_OK;
    return MI_ERR;
}

uint8_t RFID_WriteBlock(uint8_t addrBlock, uint8_t *data, uint8_t *key, uint8_t *uid)
{
    
    uint8_t addrAuth = addrBlock + (3-addrBlock%4); //вычисляем блок аутентификации для конкретного сектора
    if(MFRC522_Auth(PICC_AUTHENT1A, addrAuth, key, uid) == MI_OK)
        if(MFRC522_Write(addrBlock, data) == MI_OK)
            return MI_OK;
    return MI_ERR;
}

uint8_t RFID_ReadBlock(uint8_t addrBlock, uint8_t *data, uint8_t *key, uint8_t *uid)
{
    uint8_t addrAuth = addrBlock + (3-addrBlock%4); //вычисляем блок аутентификации для конкретного сектора
    if(MFRC522_Auth(PICC_AUTHENT1A, addrAuth, key, uid) == MI_OK)
        if(MFRC522_Read(addrBlock, data) == MI_OK)
            return MI_OK;
    return MI_ERR;
}

uint8_t RFID_ReadSector(uint8_t addrSector, uint8_t *data, uint8_t *key, uint8_t *uid)
{
    addrSector *= 4; //теперь тут конкретный адрес начального блока сектора
    // uint8_t addrAuth = addrBlock + (3-addrBlock%4); //вычисляем блок аутентификации для конкретного сектора
    uint8_t addrAuth = addrSector + 3;
    MFRC522_SelectTag(rfid.uid);
    if(MFRC522_Auth(PICC_AUTHENT1A, addrAuth, key, uid) == MI_OK) {
        for(int i=0; i<3; i++) {
            if(MFRC522_Read(addrSector+i, data+i*MAX_LEN) != MI_OK)
                return MI_ERR;
        }
        return MI_OK;
    }
    return MI_ERR;
}

uint8_t RFID_ChangeKey(uint8_t addrAuth, uint8_t *old_key, uint8_t *new_param, uint8_t *uid)
{
    MFRC522_SelectTag(rfid.uid);
    if(MFRC522_Auth(PICC_AUTHENT1A, addrAuth, old_key, uid) == MI_OK)
        if(MFRC522_Write(addrAuth, new_param) == MI_OK)
            return MI_OK;
    return MI_ERR;
}

static void RFID_defaultKey(uint8_t *key)
{
    for (uint8_t i = 0; i < 6; i++) { // Наполняем ключ
        key[i] = 0xFF;       // Ключ по умолчанию 0xFFFFFFFFFFFF
    }
}