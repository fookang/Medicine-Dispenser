#include "FreeRTOS.h"
#include "task.h"

#include "buzzer.h"
#include "button.h"
#include "servo.h"

void FreeRTOS_Init(void)
{
    Servo_Init();
    Button_Init();
    Buzzer_Init();
    vTaskStartScheduler();
}
