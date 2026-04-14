# Medicine Dispenser — Port Reference

Quick pin/port mapping for this firmware (MCXC444).

## GPIO / Peripheral Pin Map

| Function | MCU Pin | Port | Direction | Peripheral / Mode | Source |
| --- | --- | --- | --- | --- | --- |
| User button (SW3) | PTA4 | PORTA | Input | GPIO + falling-edge IRQ | `button.h`, `button.c` |
| Buzzer | PTE0 | PORTE | Output | GPIO | `buzzer.h`, `buzzer.c` |
| Servo 1 signal | PTE20 | PORTE | Output | TPM1_CH0 PWM (MUX=3) | `servo.h`, `servo.c` |
| Servo 2 signal | PTE21 | PORTE | Output | TPM1_CH1 PWM (MUX=3) | `servo.h`, `servo.c` |
| UART2 TX | PTE22 | PORTE | Output | UART2_TX (MUX=4) | `uart.h`, `uart.c` |
| UART2 RX | PTE23 | PORTE | Input | UART2_RX (MUX=4) | `uart.h`, `uart.c` |
| Ultrasonic TRIG | PTC2 | PORTC | Output | GPIO | `ultrasonic.h`, `ultrasonic.c` |
| Ultrasonic ECHO | PTC1 | PORTC | Input | GPIO + edge IRQ | `ultrasonic.h`, `ultrasonic.c` |
| DHT11 data | PTD3 | PORTD | In/Out (single-wire) | GPIO | `DHT.h`, `DHT.c` |
| Heartbeat data | PTE30 | PORTE | Input | GPIO | `heartbeat.h`, `heartbeat.c` |

## Interrupt Map

| IRQ Handler | Used For | Source |
| --- | --- | --- |
| `PORTA_IRQHandler` | Button input on PTA4 | `button.c, interrupt.c` |
| `PORTC_PORTD_IRQHandler` | Ultrasonic echo on PTC1 | `ultrasonic.c, interrupt.c` |
| `UART2_FLEXIO_IRQHandler` | UART2 TX/RX byte handling | `uart.c` |
| `ADC0_IRQHandler` | ADC0 conversion completion for heartbeat | `interrupt.c, heartbeat.c` |

## Timer / Clock Usage

| Block | Purpose | Notes | Source |
| --- | --- | --- | --- |
| TPM0 | 1 µs time base | Used by ultrasonic pulse timing and DHT short delays | `timer.h`, `timer.c` |
| TPM1 | Servo PWM | 20 ms period, channels CH0/CH1 | `servo.h`, `servo.c` |


## UART Settings

- Instance: `UART2`
- Baud rate: `9600`
- TX/RX pins: `PTE22` / `PTE23`
- IRQ priority: `128`

