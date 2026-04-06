#include "FreeRTOS.h"
#include "task.h"

#include "buzzer.h"
#include "button.h"
#include "servo.h"
#include "uart.h"
#include "ultrasonic.h"
#include "DHT.h"

#include "fsl_debug_console.h"

/*
 * Initializes application modules that create FreeRTOS objects/tasks,
 * then starts the scheduler.
 */
void FreeRTOS_Init(void)
{
    Servo_Init();		/* Servo uses hardware/PWM control, so no dedicated high-priority task is needed. */
    Button_Init(2);		/* Medium priority: button input should be responsive but is not time-critical. */
    Buzzer_Init(1); 	/* Low priority: buzzer feedback is non-critical and can run in the background. */
    init_uart(3,2); 	/* RX higher than TX: receiving is more urgent to avoid missing incoming data. */
    ultrasonic_init(4); /* High priority: ultrasonic measurement is timing-sensitive and directly controls servo actuation. */
    DHT_init(1);		/* Low priority: DHT readings are slow and not timing-critical. */
    vTaskStartScheduler();
}
