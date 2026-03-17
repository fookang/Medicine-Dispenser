#ifndef __BUZZER__
#define __BUZZER__

// Set buzzer output to PTE21
#define BUZZER 21

void Buzzer_Init(void);

void buzzer_on(void);

void buzzer_off(void);

void buzzer_toggle(void);

void buzzer_wake(void);

#endif
