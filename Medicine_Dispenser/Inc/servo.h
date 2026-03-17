#ifndef __SERVO__
#define __SERVO__

#define SERVO1 20
#define SERVO2 21
#define SERVO1_CHANNEL 0
#define SERVO2_CHANNEL 1

typedef enum {
	OPEN,
	CLOSE,
} ServoPosition;

void Servo_Init(void);
void startPWM(void);
void stopPWM(void);
void setPWM(ServoPosition pos);
void toggleServo(void);

#endif
