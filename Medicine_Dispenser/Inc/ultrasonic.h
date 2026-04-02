#ifndef __ULTRA_SONIC__
#define __ULTRA_SONIC__

#include <stdint.h>
#include "FreeRTOS.h"

/**
 * @brief GPIO pin index used for ultrasonic sensor output on PORTC.
 *
 * Hardware mapping: PTC2 (TRIG) and PTC1 (ECHO).
 */
#define TRIG_PIN 2
#define ECHO_PIN 1

/**
 * @brief ADC channel index for ultrasonic sensor echo signal.
 *
 * Hardware mapping: PTC1 corresponds to ADC channel 15.
 */
#define ADC_CHANNEL 15

/**
 * @brief Initializes ultrasonic sensor GPIO pins.
 */
void ultrasonic_init(void);

/*
 * @brief ISR for PORTC interrupt triggered by ultrasonic sensor echo pin.
 * @param flags Interrupt status flags from PORTC.
 * @param hpw Pointer to higher-priority-task-woken flag used by FreeRTOS ISR APIs.
 */
void Ultrasonic_PORTC_IRQHandler(uint32_t flags, BaseType_t *hpw);

#endif
