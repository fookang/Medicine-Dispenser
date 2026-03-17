#ifndef __BUTTON__
#define __BUTTON__

#include <stdint.h>
#include "FreeRTOS.h"

/**
 * @brief GPIO pin index used for the user switch on PORTA.
 *
 * Hardware mapping: PTA4. (SW3)
 */
#define SWITCH_PIN 4

/**
 * @brief Initializes button GPIO and interrupt handling.
 *
 * This function configures PTA4 as an input with pull-up, enables
 * falling-edge interrupt generation, creates the button semaphore,
 * and starts the internal button task.
 */
void Button_Init(void);

/**
 * @brief Handles PORTA interrupt events for the button input.
 *
 * @param flags Interrupt status flags from PORTA.
 * @param hpw Pointer to higher-priority-task-woken flag used by FreeRTOS ISR APIs.
 */
void Button_PortA_ISR(uint32_t flags, BaseType_t *hpw);

#endif
