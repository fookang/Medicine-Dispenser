#include "board.h"
#include "button.h"

void PORTA_IRQHandler(void) {
    NVIC_ClearPendingIRQ(PORTA_IRQn);

    BaseType_t hpw = pdFALSE;
	uint32_t flags = PORTA->ISFR;

	// ISR for button interrupt
    Button_PortA_ISR(flags, &hpw);

	portYIELD_FROM_ISR(hpw);

}
