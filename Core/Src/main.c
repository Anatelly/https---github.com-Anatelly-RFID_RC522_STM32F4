#define MAIN

#include "include.h"

int main(void)
{
  init_task();
  RFID_init();
  Lock_init();

  while (1)
  {
    RFID_reinit();
    
    Lock_task();
    // MY_change_key();
    // MY_write_password();
  }

}

//========================================================================================================

// static void MY_read_n_blocks(void)
// {
//   if(RFID_getUID(rfid.uid) == MI_OK) {
//     MFRC522_SelectTag(rfid.uid);
//     for(int i=0; i<1; i++) {
//       if(i%4 == 3) {
//         continue;
//       } else {
//           if(RFID_ReadBlock(i, rfid.buff, rfid.defkey, rfid.uid) == MI_OK) {
//             write(sck_2, rfid.buff, MAX_LEN);
//           } else
//             write(sck_2, "0", 1);
//       }
//       HAL_Delay(10);
//     }
//   } 
// }
//========================================================================================================


void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}


