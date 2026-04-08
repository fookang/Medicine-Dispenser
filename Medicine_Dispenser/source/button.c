#include "button.h"
#include <stdio.h>
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "clock_config.h"

#include "servo.h"
#include "constant.h"
#include "uart.h"
#include "buzzer.h"
#include "heartbeat.h"

#include "fsl_debug_console.h"

/* Binary semaphore released by the PORTA ISR when button is pressed. */
SemaphoreHandle_t buttonSem;


static void buttonTask(void *arg)
{
    (void)arg;
    while(1)
    {
    	if (xSemaphoreTake(buttonSem, portMAX_DELAY) == pdTRUE)
    	{
    		buzzer_stop();
    	}
	}
}

/*
 * Clears the button interrupt flag and notifies button task via semaphore.
 */
void Button_PortA_ISR(uint32_t flags, BaseType_t *hpw)
{
	if(flags & (1 << SWITCH_PIN_BUZZER))
	{
	    PORTA->ISFR |= (1 << SWITCH_PIN_BUZZER);
		xSemaphoreGiveFromISR(buttonSem, hpw);
	}
}

void Button_PORTC_ISR(uint32_t flags, BaseType_t *hpw)
{
	if(flags & (1 << SWITCH_PIN_HEARTBEAT))
	{
	    PORTC->ISFR |= (1 << SWITCH_PIN_HEARTBEAT);
	    Heartbeat_StartMeasurement(hpw);
	}
}

static void buzzer_button_init()
{
	//Disable interrupts
	NVIC_DisableIRQ(PORTA_IRQn);

	//Enable clock gating to PORTA
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK);

	//Configure MUX of PTA4
	PORTA->PCR[SWITCH_PIN_BUZZER] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[SWITCH_PIN_BUZZER] |= PORT_PCR_MUX(1);

	//Set pullup resistor
	PORTA->PCR[SWITCH_PIN_BUZZER] &= ~PORT_PCR_PS_MASK;
	PORTA->PCR[SWITCH_PIN_BUZZER] |= PORT_PCR_PS(1);
	PORTA->PCR[SWITCH_PIN_BUZZER] &= ~PORT_PCR_PE_MASK;
	PORTA->PCR[SWITCH_PIN_BUZZER] |= PORT_PCR_PE(1);

	//Set as input
	GPIOA->PDDR &= ~(1 << SWITCH_PIN_BUZZER);

	//Configure the interrupt for falling edge
	PORTA->PCR[SWITCH_PIN_BUZZER] &= ~PORT_PCR_IRQC_MASK;
	PORTA->PCR[SWITCH_PIN_BUZZER] |= PORT_PCR_IRQC(0b1010);

	//Set NVIC priority to 192
	NVIC_SetPriority(PORTA_IRQn, 192);

	//Clear pending interrupts and enable interrupts
	NVIC_ClearPendingIRQ(PORTA_IRQn);
	NVIC_EnableIRQ(PORTA_IRQn);
}

static void heartbeat_button_init()
{

    // Disable interrupts
    NVIC_DisableIRQ(PORTC_PORTD_IRQn);

    // Turn on clocking
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;

    // Configure as GPIO
    PORTC->PCR[SWITCH_PIN_HEARTBEAT] &= ~PORT_PCR_MUX_MASK;
    PORTC->PCR[SWITCH_PIN_HEARTBEAT] |= PORT_PCR_MUX(1);

    // Configure pull up resisotr
    PORTC->PCR[SWITCH_PIN_HEARTBEAT] &= ~PORT_PCR_PS_MASK;
    PORTC->PCR[SWITCH_PIN_HEARTBEAT] |= PORT_PCR_PS(1);

    // Set GPIO as input
    GPIOC->PDDR &= ~(1 << SWITCH_PIN_HEARTBEAT);

    // Enable pull up resistor
    PORTC->PCR[SWITCH_PIN_HEARTBEAT] &= ~PORT_PCR_PE_MASK;
    PORTC->PCR[SWITCH_PIN_HEARTBEAT] |= PORT_PCR_PE(1);

    // Configure interrupt for falling edge
    PORTC->PCR[SWITCH_PIN_HEARTBEAT] &= ~PORT_PCR_IRQC_MASK;
    PORTC->PCR[SWITCH_PIN_HEARTBEAT] |= PORT_PCR_IRQC(0b1010);

    // Set interrupt as lowest priority
    NVIC_SetPriority(PORTC_PORTD_IRQn, 192);

    // Clear existing interrupts
    NVIC_ClearPendingIRQ(PORTC_PORTD_IRQn);

    // Enable interrupts
    NVIC_EnableIRQ(PORTC_PORTD_IRQn);
}

/*
 * Configures PTA4 button input, interrupt routing, and associated RTOS objects.
 */
void Button_Init(int priority)
{
	buzzer_button_init();
	heartbeat_button_init();

	buttonSem = xSemaphoreCreateBinary();
	xTaskCreate(buttonTask, "button", configMINIMAL_STACK_SIZE+100, NULL, 2, NULL);


}

