#ifndef __BUTTON__
#define __BUTTON__

#include <stdint.h>
#include "FreeRTOS.h"

#define SWITCH_PIN 4 //PTA4

void Button_Init(void);
void Button_PortA_ISR(uint32_t flags, BaseType_t *hpw);

#endif
