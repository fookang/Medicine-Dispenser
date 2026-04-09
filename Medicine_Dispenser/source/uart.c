#include <stdio.h>
#include "board.h"
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "uart.h"
#include "constant.h"
#include "buzzer.h"
#include "servo.h"

#include "fsl_debug_console.h"

static volatile uint8_t tx_busy = 0;

uint8_t send_buffer[sizeof(TPacket)];
uint8_t recv_buffer[sizeof(TPacket)];

#define QLEN 5

QueueHandle_t rec_queue;
QueueHandle_t send_queue;
SemaphoreHandle_t txDoneSem;

/*
 * Sends a packet of data over UART2.
 * This function is exported for use by other modules.
 */
void uart_send(const TPacket *packet)
{
	xQueueSend(send_queue, (void *)packet, portMAX_DELAY);
}

/*
 * Sends a packet of data over UART2.
 */
static void sendPacket(const TPacket *packet)
{
	memcpy(send_buffer, packet, sizeof(TPacket));

	// Enable the TIE interrupt
	UART2->C2 |= UART_C2_TIE_MASK;

	// Enable the transmitter
	UART2->C2 |= UART_C2_TE_MASK;
}

/*
 * Task for sending UART data.
 */
static void sendTask(void *arg)
{
	(void)arg;

	while (1)
	{
		TPacket packet;
		if (xQueueReceive(send_queue, &packet, portMAX_DELAY) == pdTRUE)
		{
			PRINTF("Sending packet:\r\n");
			packet.magic = MAGIC;
			PRINTF("  device_type = %u\r\n", packet.device_type);
			PRINTF("  command     = %u\r\n", packet.command);
			PRINTF("  data        = ");
			for (int i = 0; i < MAX_DATA_LEN; i++)
			{
				PRINTF("%02X ", packet.data[i]);
			}
			PRINTF("\r\n");
			sendPacket(&packet);
			xSemaphoreTake(txDoneSem, portMAX_DELAY);
		}
	}
}

/*
 * Task for receiving UART data.
 * Parse received packets and perform actions based on the device type and command.
 */
static void recvTask(void *arg)
{
	(void)arg;

	while (1)
	{
		TPacket packet;
		if (xQueueReceive(rec_queue, &packet, portMAX_DELAY) == pdTRUE)
		{
			PRINTF("Received packet:\r\n");
			PRINTF("  device_type = %u\r\n", packet.device_type);
			PRINTF("  command     = %u\r\n", packet.command);
			PRINTF("  data        = ");
			for (int i = 0; i < MAX_DATA_LEN; i++)
			{
				PRINTF("%02X ", packet.data[i]);
			}
			PRINTF("\r\n");
			if (packet.device_type == BUZZER_DEV)
			{
				switch (packet.command)
				{
				case BUZZER_ON:
					buzzer_wake();
					break;

				case BUZZER_OFF:
					buzzer_stop();
					break;

				case BUZZER_CHANGE:
				{
					// Convert hours to MS
					uint32_t time = packet.data[0] * 1000 * 60 * 60;
					buzzer_set_period_ms(time);
				}
				default:
					// dont change anything
				}
			}
			if (packet.device_type == SERVO_DEV)
			{
				switch (packet.command)
				{
				case SERVO_OPEN:
					openServo();
					break;

				case SERVO_CLOSE:
					closeServo();
					break;

				default:
					// Dont do anything

				}
			}
		}
	}
}

/*
 * UART2 interrupt handler.
 */
void UART2_FLEXIO_IRQHandler(void)
{
	// Send and receive pointers
	static int recv_ptr = 0, send_ptr = 0;
	static uint8_t receiving = 0;

	NVIC_ClearPendingIRQ(UART2_FLEXIO_IRQn);
	if (UART2->S1 & UART_S1_TDRE_MASK) // Send data
	{
		if (send_ptr >= sizeof(TPacket))
		{
			BaseType_t hpw = pdFALSE;

			send_ptr = 0;

			// Disable the transmit interrupt
			UART2->C2 &= ~UART_C2_TIE_MASK;

			// Disable the transmitter
			UART2->C2 &= ~UART_C2_TE_MASK;

			xSemaphoreGiveFromISR(txDoneSem, &hpw);
			portYIELD_FROM_ISR(hpw);
		}
		else
		{
			UART2->D = send_buffer[send_ptr++];
		}
	}

	if (UART2->S1 & UART_S1_RDRF_MASK)
	{
		// Read Data to buffer
		uint8_t rx_data = UART2->D;
		if (!receiving)
		{
			if (rx_data == MAGIC)
			{
				recv_ptr = 0;
				recv_buffer[recv_ptr++] = rx_data;
				receiving = 1;
			}
		}
		else
		{
			if (recv_ptr < sizeof(TPacket))
			{
				recv_buffer[recv_ptr++] = rx_data;
			}

			if (recv_ptr >= sizeof(TPacket))
			{
				TPacket packet;
				BaseType_t hpw = pdFALSE;

				memcpy(&packet, recv_buffer, sizeof(TPacket));
				if (packet.magic == MAGIC)
				{
					xQueueSendFromISR(rec_queue, (void *)&packet, &hpw);
					portYIELD_FROM_ISR(hpw);
				}

				recv_ptr = 0;
				receiving = 0;
			}
		}
	}
}

/*
 * Initializes the UART2 peripheral.
 * Sets baud rate to 9600, configures Tx on PTE22 and Rx on PTE23.
 * Set up interrupts and semaphores for sending and receiving data.
 * Creates FreeRTOS tasks for handling sending and receiving of UART data.
 */
void init_uart(int recvPriority, int sendPriority)
{
	NVIC_DisableIRQ(UART2_FLEXIO_IRQn);

	// enable clock to UART2 and PORTE
	SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

	// Ensure Tx and Rx are disabled before configuration
	UART2->C2 &= ~((UART_C2_TE_MASK) | (UART_C2_RE_MASK));

	// connect UART pins for PTE22, PTE23
	PORTE->PCR[UART_TX] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[UART_TX] |= PORT_PCR_MUX(4);

	PORTE->PCR[UART_RX] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[UART_RX] |= PORT_PCR_MUX(4);

	// Set the baud rate
	uint32_t bus_clk = CLOCK_GetBusClkFreq();

	// This version of sbr does integer rounding.
	uint32_t sbr = (bus_clk + (BAUD_RATE * 8)) / (BAUD_RATE * 16);

	// Set SBR. Bits 8 to 12 in BDH, 0-7 in BDL.
	// MUST SET BDH FIRST!
	UART2->BDH &= ~UART_BDH_SBR_MASK;
	UART2->BDH |= ((sbr >> 8) & UART_BDH_SBR_MASK);
	UART2->BDL = (uint8_t)(sbr & 0xFF);

	// Disable loop mode
	UART2->C1 &= ~UART_C1_LOOPS_MASK;
	UART2->C1 &= ~UART_C1_RSRC_MASK;

	// Disable parity2
	UART2->C1 &= ~UART_C1_PE_MASK;

	// 8-bit mode
	UART2->C1 &= ~UART_C1_M_MASK;

	// Enable RX interrupt
	UART2->C2 |= UART_C2_RIE_MASK;

	// Enable the receiver
	UART2->C2 |= UART_C2_RE_MASK;

	NVIC_SetPriority(UART2_FLEXIO_IRQn, UART2_INT_PRIO);
	NVIC_ClearPendingIRQ(UART2_FLEXIO_IRQn);

	rec_queue = xQueueCreate(QLEN, sizeof(TPacket));
	send_queue = xQueueCreate(QLEN, sizeof(TPacket));
	txDoneSem = xSemaphoreCreateBinary();

	xTaskCreate(recvTask, "recvTask", configMINIMAL_STACK_SIZE + 100, NULL, recvPriority, NULL);
	xTaskCreate(sendTask, "sendTask", configMINIMAL_STACK_SIZE + 100, NULL, sendPriority, NULL);

	NVIC_EnableIRQ(UART2_FLEXIO_IRQn);
}
