#ifndef __BUTTON__
#define __BUTTON__

#include <stdint.h>
#include "FreeRTOS.h"

/**
 * @brief GPIO pin index used for the user switch on PORTA and PORTC.
 *
 * Hardware mapping: PTA4. (SW3)
 * Hardware mapping: PTC3. (SW2)
 */
#define SWITCH_PIN_BUZZER 4
#define SWITCH_PIN_HEARTBEAT 3

/**
 * @brief Initializes button GPIO and interrupt handling.
 *
 * This function configures PTA4 as an input with pull-up, enables
 * falling-edge interrupt generation, creates the button semaphore,
 * and starts the internal button task.
 *
 * @param priority Priority for the Button task.
 */
void Button_Init(int priority);

/**
 * @brief Handles PORTA interrupt events for the button input.
 *
 * @param flags Interrupt status flags from PORTA.
 * @param hpw Pointer to higher-priority-task-woken flag used by FreeRTOS ISR APIs.
 */
void Button_PortA_ISR(uint32_t flags, BaseType_t *hpw);

/**
 * @brief Handles PORTC interrupt events for the button input.
 *
 * @param flags Interrupt status flags from PORTC.
 * @param hpw Pointer to higher-priority-task-woken flag used by FreeRTOS ISR APIs.
 */
void Button_PORTC_ISR(uint32_t flags, BaseType_t *hpw);

#endif
