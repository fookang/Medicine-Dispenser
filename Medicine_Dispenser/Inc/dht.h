#ifndef __DHT__
#define __DHT__

#include <stdint.h>

/**
 * @brief GPIO pin index used for DHT11 single-wire data line.
 *
 * Hardware mapping: PTD3.
 */
#define DHT_PIN 3

/**
 * @brief Structure to hold DHT11 sensor data readings.
 */
typedef struct
{
    uint8_t temperature_c;
    uint8_t humidity_percent;
} DHT_data_t;

/**
 * @brief Initialize DHT11 GPIO and create the periodic DHT task.
 *
 * This sets up the data pin and starts the internal FreeRTOS task
 * responsible for periodic sensor reads and alarm reporting.
 *
 * @param priority Priority for the DHT task.
 */
void DHT_Init(int priority);

#endif
