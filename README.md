# Medicine Dispenser

This project is an embedded medicine dispenser built around two controllers:
an MCXC444 board that drives the hardware, and an ESP32-S2 that provides the user-facing
interface and network features.

The MCXC444 firmware handles the physical dispenser logic: buttons, buzzer, servo,
ultrasonic sensing, DHT11 input, and heartbeat sensing. The ESP32-S2 acts as a companion
controller that relays commands over UART, shows heart-rate values on an OLED display,
and forwards alerts to Telegram.

## Project Overview

The two boards communicate using a simple packet format over UART2 at 9600 baud.
Packets include a magic byte for framing, a device type, a command, and data payload.
The ESP32 can request a heart-rate reading from the MCXC444, while the MCU can report
sense data and trigger hardware actions such as the servo or buzzer.

## What This Repository Contains

- `Medicine_Dispenser/`: MCXC444 firmware project
- `ESP32/`: ESP32-S2 controller project
- `Diagram/`: architecture diagram for the system

## Runtime Responsibilities

- Button input on the MCXC444 for local control
- Servo control for dispenser movement
- Buzzer alerts for user feedback
- Ultrasonic sensing for distance detection
- DHT11 temperature and humidity reading
- Heartbeat sensing through ADC input
- UART communication with the ESP32-S2

## Hardware Reference

The tables below are a quick hardware reference for the main pins, interrupts, timers,
and UART settings used by the code.

## GPIO / Peripheral Pin Map

| Function               | MCU Pin | Port  | Direction            | Peripheral / Mode       | Source                         |
| ---------------------- | ------- | ----- | -------------------- | ----------------------- | ------------------------------ |
| User button (SW3)      | PTA4    | PORTA | Input                | GPIO + falling-edge IRQ | `button.h`, `button.c`         |
| Heartbeat button (SW2) | PTC3    | PORTC | Input                | GPIO + falling-edge IRQ | `button.h`, `button.c`         |
| Buzzer                 | PTE0    | PORTE | Output               | GPIO                    | `buzzer.h`, `buzzer.c`         |
| Servo 1 signal         | PTE20   | PORTE | Output               | TPM1_CH0 PWM (MUX=3)    | `servo.h`, `servo.c`           |
| UART2 TX               | PTE22   | PORTE | Output               | UART2_TX (MUX=4)        | `uart.h`, `uart.c`             |
| UART2 RX               | PTE23   | PORTE | Input                | UART2_RX (MUX=4)        | `uart.h`, `uart.c`             |
| Ultrasonic TRIG        | PTC2    | PORTC | Output               | GPIO                    | `ultrasonic.h`, `ultrasonic.c` |
| Ultrasonic ECHO        | PTC1    | PORTC | Input                | GPIO + edge IRQ         | `ultrasonic.h`, `ultrasonic.c` |
| DHT11 data             | PTD3    | PORTD | In/Out (single-wire) | GPIO                    | `dht.h`, `dht.c`               |
| Heartbeat data         | PTE30   | PORTE | Input                | ADC0_SE23 (MUX=0)       | `heartbeat.h`, `heartbeat.c`   |

## Interrupt Map

| IRQ Handler               | Used For                                             | Source                                |
| ------------------------- | ---------------------------------------------------- | ------------------------------------- |
| `PORTA_IRQHandler`        | Button input on PTA4                                 | `button.c, interrupt.c`               |
| `PORTC_PORTD_IRQHandler`  | Ultrasonic echo on PTC1 and heartbeat button on PTC3 | `ultrasonic.c, button.c, interrupt.c` |
| `UART2_FLEXIO_IRQHandler` | UART2 TX/RX byte handling                            | `uart.c`                              |
| `ADC0_IRQHandler`         | ADC0 conversion completion for heartbeat             | `interrupt.c, heartbeat.c`            |

## Timer / Clock Usage

| Block | Purpose        | Notes                                                | Source               |
| ----- | -------------- | ---------------------------------------------------- | -------------------- |
| TPM0  | 1 µs time base | Used by ultrasonic pulse timing and DHT short delays | `timer.h`, `timer.c` |
| TPM1  | Servo PWM      | 20 ms period, channel CH0                            | `servo.h`, `servo.c` |

## UART Settings

- Instance: `UART2`
- Baud rate: `9600`
- TX/RX pins: `PTE22` / `PTE23`
- IRQ priority: `128`
