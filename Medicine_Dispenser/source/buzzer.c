#include "buzzer.h"
#include <stdio.h>
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Binary semaphore used to wake the buzzer task from other contexts. */
SemaphoreHandle_t buzzerSem;

static void buzzerTask(void *arg)
{
    (void)arg;

    while (1)
    {
        if (xSemaphoreTake(buzzerSem, portMAX_DELAY) == pdTRUE)
        {
            buzzer_toggle();
        }
    }
}

/*
 *Configures buzzer GPIO and creates buzzer task and semaphore.
 */
void Buzzer_Init(void)
{
    // Turn on clock gating
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

    // Set GPIO
    PORTE->PCR[BUZZER] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[BUZZER] |= PORT_PCR_MUX(1);

    // Set as output
    GPIOE->PDDR |= 1 << BUZZER;

    // Set off buzzer
    GPIOE->PCOR |= 1 << BUZZER;

    buzzerSem = xSemaphoreCreateBinary();
    xTaskCreate(buzzerTask, "buzzer", configMINIMAL_STACK_SIZE + 100, NULL, 2, NULL);
}

/*
 * Turns the buzzer on.
 */
void buzzer_on(void)
{
    GPIOE->PSOR |= 1 << BUZZER;
}

/*
 * Turns the buzzer off.
 */
void buzzer_off(void)
{
    GPIOE->PCOR |= 1 << BUZZER;
}

/*
 * Toggles the buzzer state.
 */
void buzzer_toggle(void)
{
    GPIOE->PTOR |= 1 << BUZZER;
}

/*
 * Wakes the buzzer task to toggle the buzzer.
 */
void buzzer_wake(void)
{
    xSemaphoreGive(buzzerSem);
}
