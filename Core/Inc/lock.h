#ifndef __LOCK_H
#define __LOCK_H

#define PIN_R_EN     	    B,14,L,OUTPUT_PUSH_PULL,SPEED_2MHZ
#define PIN_L_EN     	    B,13,L,OUTPUT_PUSH_PULL,SPEED_2MHZ
#define PIN_POWER     	    B,12,L,OUTPUT_PUSH_PULL,SPEED_2MHZ

typedef enum {
    state_close,
    state_open
}Lock_state_t;

void Lock_init(void);

void Lock_change_key(void);
void Lock_write_password(void);
void Lock_task(void);

#endif /* __LOCK_H */