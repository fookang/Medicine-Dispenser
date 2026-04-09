#ifndef __CONSTANTS_INC__
#define __CONSTANTS_INC__

#define MAX_DATA_LEN 16
#define MAGIC 0x67

typedef enum {
	BUZZER_DEV = 0,
	BUTTON_DEV = 1,
	HB_SENSOR_DEV = 2,
	DHT_SENSOR_DEV = 3,
	SERVO_DEV = 4
} DeviceType;

typedef enum {
	SERVO_OPEN = 0,
	SERVO_CLOSE = 1,
	BUZZER_ON = 2,
	BUZZER_OFF = 3,
	BUZZER_CHANGE = 4,
	HB_GET_DATA = 5,
	CMD_NONE = 6
} Command;

typedef struct
{
	uint8_t magic;
	uint8_t device_type;
	uint8_t command;
	uint8_t padding[2];
	uint8_t data[MAX_DATA_LEN];
} TPacket;

#endif
