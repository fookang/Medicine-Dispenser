#include "buzzer.h"
#include <stdio.h>
#include "board.h"

void Buzzer_Init()
{
	// Turn on clock gating
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

	// Set GPIO
    PORTE->PCR[BUZZER] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[BUZZER] |= PORT_PCR_MUX(1);

    // Set as output
    GPIOE->PDDR |= 1 << BUZZER;

    // Set off buzzer
    GPIOE->PSOR |= 1 << BUZZER;
}

void buzzer_on()
{
    GPIOE->PCOR |= 1 << BUZZER;
}

void buzzer_off()
{
    GPIOE->PSOR |= 1 << BUZZER;
}

void buzzer_toggle()
{
    GPIOE->PTOR |= 1 << BUZZER;
}
