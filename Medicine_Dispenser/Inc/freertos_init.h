#ifndef __FREERTOS__
#define __FREERTOS__

/**
 * @brief Initializes RTOS-dependent modules and starts the scheduler.
 *
 * This function initializes application peripherals/tasks that rely on
 * FreeRTOS, then transfers control to the FreeRTOS scheduler.
 */
void FreeRTOS_Init(void);

#endif
