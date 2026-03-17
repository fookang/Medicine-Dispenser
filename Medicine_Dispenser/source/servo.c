#include "servo.h"
#include "button.h"
#include <stdio.h>
#include "board.h"

static ServoPosition servoState;

void Servo_Init(void)
{
	// Turn on clock gating to TPM1
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;

	// Turn on clock gating to PORTE
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

	// Configure the port to be for PWM
	PORTE->PCR[SERVO1] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[SERVO1] |= PORT_PCR_MUX(0b11);

	PORTE->PCR[SERVO2] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[SERVO2] |= PORT_PCR_MUX(0b11);

	// Set pins to output
	GPIOE->PDDR |= ((1 << SERVO1) | (1 << SERVO2));

	// Turn off TPM0 by clearing the clock mode
	TPM1->SC &= ~TPM_SC_CMOD_MASK;

	// Clear and set the prescalar 4
	TPM1->SC &= ~TPM_SC_PS_MASK;
	TPM1->SC |= (TPM_SC_TOIE_MASK | TPM_SC_PS(0b010));

	// Set edge-aligned PWM mode
	TPM1->SC &= ~TPM_SC_CPWMS_MASK;

	// Initialize count to 0
	TPM1->CNT = 0;

    // 20 ms period:
    // 8 MHz / 4 = 2 MHz
    // 2 Mhz / 50 = 40000 counts
	TPM1->MOD = 39999;

	//Configure TPM0 channels.
	// MS = 10, ELS = 10
	TPM1->CONTROLS[SERVO1_CHANNEL].CnSC &= ~(TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK);
	TPM1->CONTROLS[SERVO1_CHANNEL].CnSC |= (TPM_CnSC_MSB(1) | TPM_CnSC_MSA(0) | TPM_CnSC_ELSB(1) | TPM_CnSC_ELSA(0));
	TPM1->CONTROLS[SERVO2_CHANNEL].CnSC &= ~(TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK);
	TPM1->CONTROLS[SERVO2_CHANNEL].CnSC |= (TPM_CnSC_MSB(1) | TPM_CnSC_MSA(0) | TPM_CnSC_ELSB(1) | TPM_CnSC_ELSA(0));

	TPM1->CONTROLS[SERVO1_CHANNEL].CnV = 4000;
	servoState = CLOSE;
	setPWM(servoState);

	startPWM();
}

void startPWM(void) {
    TPM1->SC |= TPM_SC_CMOD(0b1);
}

void stopPWM(void) {
    TPM1->SC &= ~TPM_SC_CMOD_MASK;
}

void setPWM(ServoPosition pos) {
	switch(pos) {
	    case(OPEN):
			TPM1->CONTROLS[SERVO1_CHANNEL].CnV = 2000;
	    	break;
	    case(CLOSE):
			TPM1->CONTROLS[SERVO1_CHANNEL].CnV = 4000;
	    	break;
	    default:
	    	TPM1->CONTROLS[SERVO1_CHANNEL].CnV = 4000;
	}
}

void toggleServo(void)
{
    servoState = (servoState == CLOSE) ? OPEN : CLOSE;
    setPWM(servoState);
}
