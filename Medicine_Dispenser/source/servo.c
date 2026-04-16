#include "servo.h"
#include "button.h"
#include "board.h"

#define OPEN_PWM 4000
#define CLOSE_PWM 2000

static ServoPosition servoState;

static void start_PWM(void)
{
    TPM1->SC |= TPM_SC_CMOD(0b1);
}

/*
 * @brief Sets the PWM value for the servo based on its position.
 */
static void set_PWM(ServoPosition pos)
{
    switch (pos)
    {
    case (OPEN):
        TPM1->CONTROLS[SERVO1_CHANNEL].CnV = OPEN_PWM;
        break;
    case (CLOSE):
        TPM1->CONTROLS[SERVO1_CHANNEL].CnV = CLOSE_PWM;
        break;
    default:
        TPM1->CONTROLS[SERVO1_CHANNEL].CnV = CLOSE_PWM;
    }
}

/*
 * Initializes the servo by configuring the TPM1 module for PWM generation
 * and setting the initial position to closed.
 */
void Servo_Init(void)
{
    // Turn on clock gating to TPM1
    SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;

    // Turn on clock gating to PORTE
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

    // Configure the port to be for PWM
    PORTE->PCR[SERVO1] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[SERVO1] |= PORT_PCR_MUX(0b11);

    // Set pins to output
    GPIOE->PDDR |= (1 << SERVO1);

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

    // Configure TPM0 channels.
    //  MS = 10, ELS = 10
    TPM1->CONTROLS[SERVO1_CHANNEL].CnSC &= ~(TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK);
    TPM1->CONTROLS[SERVO1_CHANNEL].CnSC |= (TPM_CnSC_MSB(1) | TPM_CnSC_MSA(0) | TPM_CnSC_ELSB(1) | TPM_CnSC_ELSA(0));

    TPM1->CONTROLS[SERVO1_CHANNEL].CnV = CLOSE_PWM;
    servoState = CLOSE;
    set_PWM(servoState);

    start_PWM();
}

/*
 * @brief Toggles the servo between open and closed positions.
 */
void toggle_servo(void)
{
    servoState = (servoState == CLOSE) ? OPEN : CLOSE;
    set_PWM(servoState);
}

/*
 * @brief Opens the servo by setting it to the OPEN position.
 */
void open_servo(void)
{
    servoState = OPEN;
    set_PWM(servoState);
}

/*
 * @brief Closes the servo by setting it to the CLOSE position.
 */
void close_servo(void)
{
    servoState = CLOSE;
    set_PWM(servoState);
}
