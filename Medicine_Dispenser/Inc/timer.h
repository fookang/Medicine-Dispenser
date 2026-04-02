#ifndef __TIMER__
#define __TIMER__

/**
 * @brief Initialize the shared microsecond time base.
 *
 * Configures and starts TPM0 so modules (for example ultrasonic and DHT)
 * can use a common 1 us-resolution counter for short timing operations.
 */
void Init_Timer(void);

#endif
