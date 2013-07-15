
#define LED1_PIN_NO			0
#define LED1_PIN_MASK		1<<LED1_PIN_NO
#define LED1_PORT			0
#define LED1_INIT			{LPC_IOCON->RESET_PIO0_0 = 1;}

#ifdef _MAIN_

bool	sysTicked = false;

#endif
