#ifndef extern_vars
#include"stm32f4xx_hal.h"

#define extern_vars
#define FLAGGED 1
#define NOT_FLAGGED 0

extern uint8_t IR_INT_STATE;
extern uint8_t PI_INT_STATE;
extern uint8_t CLAW_INT_STATE;
extern uint8_t EDGE_LEFT_STATE;
extern uint8_t EDGE_RIGHT_STATE;

#endif
