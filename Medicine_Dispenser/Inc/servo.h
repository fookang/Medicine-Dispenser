#ifndef __SERVO__
#define __SERVO__

/**
 * @brief Servo pin index used for servo output on PORTE.
 *
 * Hardware mapping: PTE20.
 */
#define SERVO1 20

/**
 * @brief Servo channel index used for TPM1 channels for pwm.
 */
#define SERVO1_CHANNEL 0

/*
 * @brief Enum for servo positions.
 */
typedef enum
{
	OPEN,
	CLOSE,
} ServoPosition;

/*
 * @brief Initializes servo GPIO and PWM handling.
 *
 * This function configures PTE20 for TPM1 PWM output,
 * sets up TPM1 for edge-aligned PWM with a 20 ms period, and initializes the servo to the closed position.
 */
void Servo_Init(void);

/*
 * @brief Toggles the servo between open and closed positions.
 */
void toggle_servo(void);

/*
 * @brief Sets the servo to the open position.
 */
void open_servo(void);

/*
 * @brief Sets the servo to the closed position.
 */
void close_servo(void);

#endif
