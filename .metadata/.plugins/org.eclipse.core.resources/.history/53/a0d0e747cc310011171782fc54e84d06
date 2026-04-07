#include "board.h"
#include "FreeRTOS.h"
#include "task.h"

#include "constant.h"
#include "DHT.h"
#include "uart.h"

#include "fsl_debug_console.h"

/* Threshold for high temperature alarm and count limit for consecutive readings */
#define HIGH_TEMP_THRESHOLD 40
#define HIGH_TEMP_COUNT_LIMIT 3

static void DHT_pin_output(void)
{
    GPIOD->PDDR |= 1 << DHT_PIN;
}

static void DHT_pin_input(void)
{
    GPIOD->PDDR &= ~(1 << DHT_PIN);
}

static void DHT_write_low(void)
{
    GPIOD->PCOR |= 1 << DHT_PIN;
}

static void DHT_write_high(void)
{
    GPIOD->PSOR |= 1 << DHT_PIN;
}

static uint8_t DHT_read_pin(void)
{
    return (GPIOD->PDIR & (1 << DHT_PIN)) ? 1 : 0;
}

/*
 * Delay for a specified number of microseconds.
 * @param count Number of microseconds to delay.
 */
static void delay_us(uint32_t count)
{
    uint32_t start = TPM0->CNT;
    while ((TPM0->CNT - start) < count)
        ;
}

/*
 * Wait for the DHT pin to reach the specified level within a timeout.
 * @param level Desired pin level (0 or 1).
 * @param timeout_us Timeout in microseconds.
 * @return 1 if level is reached, 0 if timeout occurs.
 */
static int wait_for_level(uint8_t level, uint32_t timeout_us)
{
    uint32_t start = TPM0->CNT;

    while ((TPM0->CNT - start) < timeout_us)
    {
        if (DHT_read_pin() == level)
        {
            return 1;
        }
    }

    return 0;
}

/*
 * Read data from DHT11 sensor.
 * @param out Pointer to structure to store sensor data.
 * @return 1 on success, 0 on failure.
 */

static int DHT_read(DHT_data_t *out)
{
    uint8_t bits[5] = {0};

    /* Output low for at least 18ms start signal */
    DHT_pin_output();
    DHT_write_low();
    vTaskDelay(pdMS_TO_TICKS(20));

    taskENTER_CRITICAL();

    /* Release line and switch to input */
    DHT_write_high();
    delay_us(30);
    DHT_pin_input();

    /* Wait for sensor response */
    if (!wait_for_level(0, 100))
    {
        taskEXIT_CRITICAL();
        PRINTF("fail A\r\n");
        return 0;
    }
    if (!wait_for_level(1, 100))
    {
        taskEXIT_CRITICAL();
        PRINTF("fail B\r\n");
        return 0;
    }
    if (!wait_for_level(0, 100))
    {
        taskEXIT_CRITICAL();
        PRINTF("fail C\r\n");
        return 0;
    }

    /* Read 40 bits */
    for (int i = 0; i < 40; i++)
    {
        if (!wait_for_level(1, 100))
        {
            taskEXIT_CRITICAL();
            PRINTF("fail bit high %d\r\n", i);
            return 0;
        }

        /* sample in middle of bit */
        delay_us(35);

        if (DHT_read_pin())
            bits[i / 8] |= (1 << (7 - (i % 8)));

        if (!wait_for_level(0, 100))
        {
            taskEXIT_CRITICAL();
            PRINTF("fail bit low %d\r\n", i);
            return 0;
        }
    }

    taskEXIT_CRITICAL();

    /* Checksum */
    if (((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3])) != bits[4])
        return 0;

    out->humidity_percent = bits[0];
    out->temperature_c = bits[2];
    return 1;
}

/*
 * FreeRTOS task responsible for periodic sensor reads and alarm reporting.
 * Reads data every 10 seconds, checks for 3 consecutive high temperatures, and sends alarm if needed.
 */
static void DHTTask(void *arg)
{
    (void)arg;

    DHT_data_t data;
    int highTempCount = 0;
    int highAlarmSent = 0;

    while (1)
    {
        if (DHT_read(&data))
        {
            PRINTF("Temp: %u C\r\n", data.temperature_c);
            PRINTF("Humidity: %u %%\r\n", data.humidity_percent);

            if (data.temperature_c > HIGH_TEMP_THRESHOLD)
            {
                if (highTempCount < HIGH_TEMP_COUNT_LIMIT)
                {
                    highTempCount++;
                }
                if ((highTempCount >= HIGH_TEMP_COUNT_LIMIT) && !highAlarmSent)
                {
                    TPacket tpkt = {0};
                    tpkt.device_type = DHT_SENSOR_DEV;
                    tpkt.command = CMD_NONE;
                    tpkt.data[0] = data.temperature_c;
                    tpkt.data[1] = data.humidity_percent;

                    uart_send(&tpkt);
                    highAlarmSent = 1;
                }
            }
            else
            {
                highTempCount = 0;
                highAlarmSent = 0;
            }
        }
        else
        {
            PRINTF("DHT11 read failed\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/*
 * This sets up the data pin and starts the internal FreeRTOS task
 * responsible for periodic sensor reads and alarm reporting.
 */
void DHT_init(void)
{
    // Turn on clock gating
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;

    // Set GPIO
    PORTD->PCR[DHT_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTD->PCR[DHT_PIN] |= PORT_PCR_MUX(1);

    // Enable PU
    PORTD->PCR[DHT_PIN] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

    // Set as output
    GPIOD->PDDR |= 1 << DHT_PIN;

    // Set on DHT
    GPIOD->PSOR |= 1 << DHT_PIN;

    // Create DHT task
    xTaskCreate(DHTTask, "DHT", configMINIMAL_STACK_SIZE + 100, NULL, 2, NULL);
    PRINTF("DHT11 created\r\n");
}
