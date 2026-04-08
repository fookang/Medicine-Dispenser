#include "board.h"
#include "button.h"
#include "ultrasonic.h"
#include "heartbeat.h"

void PORTA_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(PORTA_IRQn);

    BaseType_t hpw = pdFALSE;
    uint32_t flags = PORTA->ISFR;

    // ISR for button interrupt
    Button_PortA_ISR(flags, &hpw);

    portYIELD_FROM_ISR(hpw);
}

void PORTC_PORTD_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(PORTC_PORTD_IRQn);

    BaseType_t hpw = pdFALSE;
    uint32_t flags = PORTC->ISFR;

    // ISR for ultrasonic echo pin interrupt
    Ultrasonic_PORTC_IRQHandler(flags, &hpw);

    // ISR for button interrupt
    Button_PORTC_ISR(flags, &hpw);

    portYIELD_FROM_ISR(hpw);
}

void ADC0_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(ADC0_IRQn);
    BaseType_t hpw = pdFALSE;
    uint32_t adcValue = ADC0->R[0];

    // ISR for heartbeat adc interrupt
    Heartbeat_ADC0_IRQHandler(adcValue, &hpw);

    portYIELD_FROM_ISR(hpw);
}
