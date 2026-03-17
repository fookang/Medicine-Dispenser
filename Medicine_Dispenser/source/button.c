#include "button.h"
#include <stdio.h>
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "clock_config.h"

#include "servo.h"

/* Binary semaphore released by the PORTA ISR when button is pressed. */
SemaphoreHandle_t buttonSem;


static void buttonTask(void *arg)
{
    (void)arg;
    while(1)
    {
    	if (xSemaphoreTake(buttonSem, portMAX_DELAY) == pdTRUE)
    		toggleServo();
	}
}

/*
 * Clears the button interrupt flag and notifies button task via semaphore.
 */
void Button_PortA_ISR(uint32_t flags, BaseType_t *hpw)
{
	if(flags & (1 << SWITCH_PIN))
	{
	    PORTA->ISFR |= (1 << SWITCH_PIN);
		xSemaphoreGiveFromISR(buttonSem, hpw);
	}
}


/*
 * Configures PTA4 button input, interrupt routing, and associated RTOS objects.
 */
void Button_Init(void)
{
    //Disable interrupts
    NVIC_DisableIRQ(PORTA_IRQn);

    //Enable clock gating to PORTA
    SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK);

    //Configure MUX of PTA4
	PORTA->PCR[SWITCH_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[SWITCH_PIN] |= PORT_PCR_MUX(1);

	//Set pullup resistor
	PORTA->PCR[SWITCH_PIN] &= ~PORT_PCR_PS_MASK;
	PORTA->PCR[SWITCH_PIN] |= PORT_PCR_PS(1);
	PORTA->PCR[SWITCH_PIN] &= ~PORT_PCR_PE_MASK;
	PORTA->PCR[SWITCH_PIN] |= PORT_PCR_PE(1);

	//Set as input
	GPIOA->PDDR &= ~(1 << SWITCH_PIN);

	//Configure the interrupt for falling edge
	PORTA->PCR[SWITCH_PIN] &= ~PORT_PCR_IRQC_MASK;
	PORTA->PCR[SWITCH_PIN] |= PORT_PCR_IRQC(0b1010);

	//Set NVIC priority to 128
	NVIC_SetPriority(PORTA_IRQn, 128);

	buttonSem = xSemaphoreCreateBinary();
	xTaskCreate(buttonTask, "button", configMINIMAL_STACK_SIZE+100, NULL, 2, NULL);

	//Clear pending interrupts and enable interrupts
	NVIC_ClearPendingIRQ(PORTA_IRQn);
	NVIC_EnableIRQ(PORTA_IRQn);

}

