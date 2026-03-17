#include <stdio.h>
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"

#include "buzzer.h"

#define BLUE_DELAY_MS 500

static void buzzerTask(void *arg) {
    (void)arg;

    while(1){
    	buzzer_toggle();

		vTaskDelay(pdMS_TO_TICKS(BLUE_DELAY_MS));
	}
}

void FreeRTOS_Init(void)
{
	xTaskCreate(buzzerTask, "buzzer", configMINIMAL_STACK_SIZE+100, NULL, 2, NULL);
}
