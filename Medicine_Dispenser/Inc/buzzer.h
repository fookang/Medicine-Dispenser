#ifndef __BUZZER__
#define __BUZZER__

/**
 * @brief GPIO pin index used for buzzer output on PORTE.
 *
 * Hardware mapping: PTE21.
 */
#define BUZZER 21

/**
 * @brief Initializes buzzer GPIO, creates buzzer task and semaphore.
 */
void Buzzer_Init(void);

/**
 * @brief Drives buzzer output high.
 */
void buzzer_on(void);

/**
 * @brief Drives buzzer output low.
 */
void buzzer_off(void);

/**
 * @brief Toggles buzzer GPIO output state.
 */
void buzzer_toggle(void);

/**
 * @brief Signals the buzzer task to perform its task.
 */
void buzzer_wake(void);

#endif
