#include "include.h"

/* ======================================================================	*/
/* Функция обмена байтами по SPI    										*/
/* ======================================================================	*/
uint8_t spiXmit (SPI_TypeDef *SPIx, uint8_t byte)
{
    SPIx->DR = byte;
    for (int i = 0; i<0xFFFFFF; i++) {
        if(SPIx->SR & SPI_SR_RXNE) {
            return SPIx->DR;
        }
    }
}

void Write_MFRC522(uchar addr, uchar val)
{
  PORT_CS_HAL->ODR &=~ PIN_CS_HAL;
  spiXmit (hspi2.Instance, addr<<1);
  spiXmit (hspi2.Instance, val);
  PORT_CS_HAL->ODR |= PIN_CS_HAL;
}

uchar Read_MFRC522(uchar addr)
{
  uint8_t tmp;
  uint8_t addr_new = SPI_READ_SIGN | (addr<<1);

  PORT_CS_HAL->ODR &=~ PIN_CS_HAL;
  spiXmit (hspi2.Instance, addr_new);
  tmp = spiXmit (hspi2.Instance, 0);
  PORT_CS_HAL->ODR |= PIN_CS_HAL;

  return tmp;
}

void SetBitMask(uchar reg, uchar mask)
{
  uchar tmp;
  tmp = Read_MFRC522(reg);
  Write_MFRC522(reg, tmp | mask);
}

void ClearBitMask(uchar reg, uchar mask)
{
  uchar tmp;
  tmp = Read_MFRC522(reg);
  Write_MFRC522(reg, tmp & (~mask));
}

void AntennaOn()
{
  Read_MFRC522(TxControlReg);
  SetBitMask(TxControlReg, 0x03);
}

void AntennaOff()
{
  ClearBitMask(TxControlReg, 0x03);
}

void MFRC522_Reset()
{
  Write_MFRC522(CommandReg, PCD_RESETPHASE);
}

void MFRC522_Init(void)
{

  PORT_CS_HAL->BSRR = PIN_CS_HAL;
  PORT_RESET_HAL->BSRR = PIN_RESET_HAL;
  MFRC522_Reset();

  Write_MFRC522(TModeReg, 0x80);
  Write_MFRC522(TPrescalerReg, 0xA9);
  Write_MFRC522(TReloadRegH, 0x03);
  Write_MFRC522(TReloadRegL, 0xE8);

  Write_MFRC522(TxAutoReg, 0x40);
  Write_MFRC522(ModeReg, 0x3D);

  AntennaOn();
}

uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
  uchar status = MI_ERR;
  uchar irqEn = 0x00;
  uchar waitIRq = 0x00;
  uchar lastBits;
  uchar n;
  uint i;

  switch (command)
  {
  case PCD_AUTHENT:
  {
    irqEn = 0x12;
    waitIRq = 0x10;
    break;
  }
  case PCD_TRANSCEIVE:
  {
    irqEn = 0x77;
    waitIRq = 0x30;
    break;
  }
  default:
    break;
  }

  Write_MFRC522(CommIEnReg, irqEn | 0x80);
  ClearBitMask(CommIrqReg, 0x80);
  SetBitMask(FIFOLevelReg, 0x80);

  Write_MFRC522(CommandReg, PCD_IDLE);

  for (i = 0; i < sendLen; i++)
  {
    Write_MFRC522(FIFODataReg, sendData[i]);
  }

  Write_MFRC522(CommandReg, command);
  if (command == PCD_TRANSCEIVE)
  {
    SetBitMask(BitFramingReg, 0x80);
  }

  i = 2000;
  do
  {

    n = Read_MFRC522(CommIrqReg);
    i--;
  } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

  ClearBitMask(BitFramingReg, 0x80);

  if (i != 0)
  {
    if (!(Read_MFRC522(ErrorReg) & 0x1B))
    {
      status = MI_OK;
      if (n & irqEn & 0x01)
      {
        status = MI_NOTAGERR;
      }

      if (command == PCD_TRANSCEIVE)
      {
        n = Read_MFRC522(FIFOLevelReg);
        lastBits = Read_MFRC522(ControlReg) & 0x07;
        if (lastBits)
        {
          *backLen = (n - 1) * 8 + lastBits;
        }
        else
        {
          *backLen = n * 8;
        }

        if (n == 0)
        {
          n = 1;
        }
        if (n > MAX_LEN)
        {
          n = MAX_LEN;
        }

        // Reading the received data in FIFO
        for (i = 0; i < n; i++)
        {
          backData[i] = Read_MFRC522(FIFODataReg);
        }
      }
    }
    else
    {
      status = MI_ERR;
    }
  }

  return status;
}

uchar MFRC522_Request(uchar reqMode, uchar *TagType)
{
  uchar status;
  uint backBits;

  Write_MFRC522(BitFramingReg, 0x07);

  TagType[0] = reqMode;
  status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

  if ((status != MI_OK) || (backBits != 0x10))
  {
    status = MI_ERR;
  }

  return status;
}

uchar MFRC522_Anticoll(uchar *serNum)
{
  uchar status;
  uchar i;
  uchar serNumCheck = 0;
  uint unLen;

  Write_MFRC522(BitFramingReg, 0x00);

  serNum[0] = PICC_ANTICOLL;
  serNum[1] = 0x20;
  status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

  if (status == MI_OK)
  {
    for (i = 0; i < 4; i++)
    {
      serNumCheck ^= serNum[i];
    }
    if (serNumCheck != serNum[i])
    {
      status = MI_ERR;
    }
  }
  return status;
}

void CalulateCRC(uchar *pIndata, uchar len, uchar *pOutData)
{
  uchar i, n;

  ClearBitMask(DivIrqReg, 0x04);
  SetBitMask(FIFOLevelReg, 0x80);

  for (i = 0; i < len; i++)
  {
    Write_MFRC522(FIFODataReg, *(pIndata + i));
  }
  Write_MFRC522(CommandReg, PCD_CALCCRC);

  i = 0xFF;
  do
  {
    n = Read_MFRC522(DivIrqReg);
    i--;
  } while ((i != 0) && !(n & 0x04));

  pOutData[0] = Read_MFRC522(CRCResultRegL);
  pOutData[1] = Read_MFRC522(CRCResultRegH);
}

uchar MFRC522_Write(uchar blockAddr, uchar *writeData)
{
  uchar status;
  uint recvBits;
  uchar i;
  uchar buff[18];

  buff[0] = PICC_WRITE;
  buff[1] = blockAddr;
  CalulateCRC(buff, 2, &buff[2]);
  status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

  if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
  {
    status = MI_ERR;
  }

  if (status == MI_OK)
  {
    for (i = 0; i < 16; i++)
    {
      buff[i] = *(writeData + i);
    }
    CalulateCRC(buff, 16, &buff[16]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
    {
      status = MI_ERR;
    }
  }

  return status;
}

uchar MFRC522_Read(uchar blockAddr, uchar *recvData)
{
  uchar status;
  uint unLen;

  recvData[0] = PICC_READ;
  recvData[1] = blockAddr;
  CalulateCRC(recvData, 2, &recvData[2]);
  status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

  if ((status != MI_OK) || (unLen != 0x90))
  {
    status = MI_ERR;
  }

  return status;
}

uchar MFRC522_Auth(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum)
{
  uchar status;
  uint recvBits;
  uchar i;
  uchar buff[12];

  buff[0] = authMode;
  buff[1] = BlockAddr;
  for (i = 0; i < 6; i++)
  {
    buff[i + 2] = *(Sectorkey + i);
  }
  for (i = 0; i < 4; i++)
  {
    buff[i + 8] = *(serNum + i);
  }
  status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

  if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08)))
  {
    status = MI_ERR;
  }

  return status;
}

uchar MFRC522_SelectTag(uchar *serNum)
{
  uchar i;
  uchar status;
  uchar size;
  uint recvBits;
  uchar buffer[9];

  buffer[0] = PICC_SElECTTAG;
  buffer[1] = 0x70;
  for (i = 0; i < 5; i++)
  {
    buffer[i + 2] = *(serNum + i);
  }
  CalulateCRC(buffer, 7, &buffer[7]);
  status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

  if ((status == MI_OK) && (recvBits == 0x18))
  {
    size = buffer[0];
  }
  else
  {
    size = 0;
  }

  return size;
}

void MFRC522_Halt()
{
  uint unLen;
  uchar buff[4];

  buff[0] = PICC_HALT;
  buff[1] = 0;
  CalulateCRC(buff, 2, &buff[2]);

  MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}

uchar Read_Single_Card()
{
  uchar requestStatus, anticollStatus;

  requestStatus = MFRC522_Request(PICC_REQIDL, rfid.uid);
  if (requestStatus == MI_OK)
  {
    anticollStatus = MFRC522_Anticoll(rfid.uid);
    if (anticollStatus == MI_OK)
    {
      MFRC522_SelectTag(rfid.uid);

      // memcpy(sNum, rfid.uid, UID_SIZE);
      // write(sck_2,(char*)sNum, UID_SIZE);
      // delay_ms(50);

      MFRC522_Halt();
      return MI_OK;
    }
  }
  return MI_ERR;
}

void Read_Multiple_Cards()
{
  uchar requestStatus, anticollStatus;
  uchar cardUIDs[MAX_CARDS][UID_SIZE];
  int cardCount = 0;
  int failedAttempts = 0;

  while (failedAttempts < MAX_ATTEMPTS && cardCount < MAX_CARDS)
  {
    requestStatus = MFRC522_Request(PICC_REQIDL, rfid.uid);
    if (requestStatus == MI_OK)
    {
      anticollStatus = MFRC522_Anticoll(rfid.uid);

      if (anticollStatus == MI_OK)
      {
        MFRC522_SelectTag(rfid.uid);

        for (int i = 0; i < UID_SIZE; i++)
        {
          cardUIDs[cardCount][i] = rfid.uid[i];
        }
        cardCount++;
        failedAttempts = 0;

        MFRC522_Halt();
      }
      else
      {
        failedAttempts++;
      }
    }
    else
    {
      failedAttempts++;
    }
  }

  uint8_t serNum[20] = {0};
  for (int i = 0; i < cardCount; i++)
  {
    memcpy(serNum, cardUIDs[i], UID_SIZE);

    write(sck_2,(char*)serNum, UID_SIZE);
    delay_ms(50);
  }
}

void Write_Content_Card(uchar authMode, uchar *myString, uchar block, uchar *Sectorkey)
{
  uchar requestStatus, anticollStatus, authStatus, writeStatus;
  requestStatus = MFRC522_Request(PICC_REQIDL, rfid.uid);
  if (requestStatus == MI_OK)
  {
    anticollStatus = MFRC522_Anticoll(rfid.uid);
    if (anticollStatus == MI_OK)
    {
      MFRC522_SelectTag(rfid.uid);
      authStatus = MFRC522_Auth(authMode, block, Sectorkey, rfid.uid);
      if (authStatus == MI_OK)
      {
        writeStatus = MFRC522_Write(block, myString);
        if (writeStatus == MI_OK)
        {
          uint8_t msg[] = "Dado gravado com sucesso!\n\r";
          write(sck_2,(char*)msg, sizeof(msg));
          MFRC522_Init();
          delay_ms(1000);
        }
      }
    }
  }
}

void Read_Content_Card(uchar authMode, uchar block, uchar *Sectorkey)
{
  uchar requestStatus, anticollStatus, authStatus, readStatus;
  uchar buffer[16];
  requestStatus = MFRC522_Request(PICC_REQIDL, rfid.uid);
  if (requestStatus == MI_OK)
  {
    anticollStatus = MFRC522_Anticoll(rfid.uid);
    if (anticollStatus == MI_OK)
    {
      MFRC522_SelectTag(rfid.uid);
      authStatus = MFRC522_Auth(authMode, block, Sectorkey, rfid.uid);
      if (authStatus == MI_OK)
      {
        readStatus = MFRC522_Read(block, buffer);
        if (readStatus == MI_OK)
        {
            write(sck_2,(char*)buffer, sizeof(buffer));
            MFRC522_Init();
            delay_ms(1000);
        }
      }
    }
  }
}