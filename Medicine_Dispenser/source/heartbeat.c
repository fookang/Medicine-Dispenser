#include "board.h"
#include "heartbeat.h"
#include "buzzer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "fsl_debug_console.h"

#include <stdint.h>
#include <stdio.h>

#include "constant.h"
#include "uart.h"

static SemaphoreHandle_t heartbeatSem;
static SemaphoreHandle_t adcSem;
static volatile uint8_t heartbeatBusy = 0;
static volatile uint16_t g_adcValue = 0;


/*
 * This set up the ADC channel and pin for reading of heartbeat data.
 */
static void heartbeat_adc_init(void)
{
    // Disable & clear interrupt
    NVIC_DisableIRQ(ADC0_IRQn);
    NVIC_ClearPendingIRQ(ADC0_IRQn);

    // Enable clock gating to ADC0
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;

    // Turn on clock gating for PORTE
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

    // Set HEARTBEAT pin as ADC
    PORTE->PCR[HEARTBEAT] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[HEARTBEAT] |= PORT_PCR_MUX(0);

    // Configure the ADC
    // Enable ADC interrupt
    ADC0->SC1[0] |= ADC_SC1_AIEN_MASK;

    // Select single-ended ADC
    ADC0->SC1[0] &= ~ADC_SC1_DIFF_MASK;
    ADC0->SC1[0] |= ADC_SC1_DIFF(0b0);

    // Set 12 bit conversion
    ADC0->CFG1 &= ~ADC_CFG1_MODE_MASK;
    ADC0->CFG1 |= ADC_CFG1_MODE(0b01);

    // Select software conversion trigger
    ADC0->SC2 &= ~ADC_SC2_ADTRG_MASK;

    // Configure alternate voltage reference
    ADC0->SC2 &= ~ADC_SC2_REFSEL_MASK;
    ADC0->SC2 |= ADC_SC2_REFSEL(0b01);

    // Don't use averaging
    ADC0->SC3 &= ~ADC_SC3_AVGE_MASK;
    ADC0->SC3 |= ADC_SC3_AVGE(0);

    // Switch off continuous conversion.
    ADC0->SC3 &= ~ADC_SC3_ADCO_MASK;
    ADC0->SC3 |= ADC_SC3_ADCO(0);

    // Set lowest priority
    NVIC_SetPriority(ADC0_IRQn, 192);
    NVIC_EnableIRQ(ADC0_IRQn);
}

/*
 * Interrupt handler for adc compeletion. 
 * Give semaphore to adcSem to read the required value
 */
void Heartbeat_ADC0_IRQHandler(uint32_t adcValue, BaseType_t *hpw)
{
    g_adcValue = (uint16_t) adcValue;
    xSemaphoreGiveFromISR(adcSem, hpw);
}

static void startADC(void) {
    //mask and set the channel
    ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK;
    ADC0->SC1[0] |= ADC_SC1_ADCH(HEARTBEAT_ADC_CHANNEL);
}

/*
 * Helper function to measure heartrate
 * This function measures heartrate in a period of 10 seconds
 * and extrapolate the value for heart rate per minutes
 *
 */
static uint32_t heartbeat_measure_bpm(void)
{
    uint32_t startTick = xTaskGetTickCount();
    uint32_t durationTicks = pdMS_TO_TICKS(5000);

    uint32_t beatCount = 0;
    uint32_t lastBeatTick = 0;
    uint8_t aboveThreshold = 0;

    int32_t average = 0;
    uint8_t average_init = 0;

    const int32_t deltaHigh = 20;
    const int32_t deltaLow  = 5;

    while ((xTaskGetTickCount() - startTick) < durationTicks)
    {
        startADC();

        if (xSemaphoreTake(adcSem, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            int32_t sample = g_adcValue;
            uint32_t now = xTaskGetTickCount();

            // Ensure average will not constantly be 0
            if (!average_init)
            {
                average = sample;
                average_init = 1;
            }

            average = (average * 15 + sample) / 16;

            // Find the different btw sample and average value
            int32_t delta = sample - average;

            // If the delta is high enough, we count it as a beat
            if ((delta > deltaHigh) && (aboveThreshold == 0))
            {
                if ((now - lastBeatTick) > pdMS_TO_TICKS(300))
                {
                    beatCount++;
                    lastBeatTick = now;
                }

                aboveThreshold = 1;
            }
            // Reset aboveTreshold if delta is not increasing a lot
            // delta is intentionally positive because of the inaccuracy of the sensor
            else if (delta < deltaLow)
            {
                aboveThreshold = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return (beatCount * 60) / 5;
}

/*
 * Heartbeat Task which is in charge of measuring heartrate
 * It measures heartrate via heartbeat_measure_bpm() funciton
 * and send the value over UART
 *
 */
static void heartbeatTask(void *arg)
{
    (void)arg;

    while (1)
    {
        if (xSemaphoreTake(heartbeatSem, portMAX_DELAY) == pdTRUE)
        {
            PRINTF("Measuring heart rate\r\n");

            heartbeatBusy = 1;

            uint32_t bpm = heartbeat_measure_bpm();

            buzzer_wake();
            vTaskDelay(pdMS_TO_TICKS(2000));
            buzzer_stop();

            PRINTF("Heartbeat measurement complete: %lu BPM\r\n", bpm);
            TPacket pkt = {0};
            pkt.device_type = HB_SENSOR_DEV;
            pkt.command = CMD_NONE;
            pkt.data[0] = (uint8_t) bpm;

            uart_send(&pkt);

            heartbeatBusy = 0;
        }
    }
}


/*
 * This sets up the adc pin and starts the internal FreeRTOS task
 * responsible for measuring heartrate and sending values over UART. 
 */
void Heartbeat_Init(int priority)
{
    heartbeatSem = xSemaphoreCreateBinary();
    adcSem = xSemaphoreCreateBinary();

    heartbeat_adc_init();

    xTaskCreate(heartbeatTask,
            "heartbeat",
            configMINIMAL_STACK_SIZE + 200,
            NULL,
            priority,
            NULL);
}

/*
 * Function exposed as API for other task to signal the start of measuring heart rate
 */
void Heartbeat_StartMeasurement(BaseType_t *hpw)
{
    if ((heartbeatSem != NULL) && (heartbeatBusy == 0))
    {
        xSemaphoreGiveFromISR(heartbeatSem, hpw);
    }
}
