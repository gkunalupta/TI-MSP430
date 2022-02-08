#include <msp430.h>				
#include <stdbool.h>
#include <stdint.h>


void uart_init()
{

    UCA0CTL1 = UCSWRST;
    UCA0CTL1 |= UCSSEL1; // clock source for uart is selected SMCLK, UCSSELx bits = 10 for that,
    UCA0BR0 = 104;                            // 1MHz 9600
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1

    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;

    UCA0CTL1 &= ~UCSWRST;
}

void uart_tran_byte(uint8_t data)
{
    while (!(UC0IFG & UCA0TXIFG));
        UCA0TXBUF = data;
}

uint8_t uart_recv_byte()
{
    while(!(UC0IFG & UCA0RXIFG));
    return UCA0RXBUF;
}

void uart_tran_string(const char *myString)      /////to print any string
{
    while (*myString)
    uart_tran_byte(*myString++);

}
void uart_tran_bin(uint8_t val)
{
    int8_t ptr;
    for(ptr=7;ptr>=0;ptr--)
    {
        if ((val & (1<<ptr))==0)
        {
             uart_tran_byte('0');
        }
        else
        {
             uart_tran_byte('1');
        }
    }
}
void uart_tran_dec(uint8_t val)
{
    unsigned char buf[5];
    int8_t ptr;
    for(ptr=0;ptr<5;++ptr)
    {
        buf[ptr] = (val % 10) + '0';
        val /= 10;
    }
    for(ptr=4;ptr>0;--ptr)
    {
        if (buf[ptr] != '0')
        break;
    }
    for(;ptr>=0;--ptr)
    {
         uart_tran_byte(buf[ptr]);
    }
}
void newlin_uart()
{
    uart_tran_byte('\n');
}
/**
 * blink.c
 */
void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;		// stop watchdog timer


	 DCOCTL = 0;                               // Select lowest DCOx and MODx settings
	 BCSCTL1 = CALBC1_1MHZ;                 /* BCSCTL1 Calibration Data for 1MHz */
	 DCOCTL = CALDCO_1MHZ;                  /* DCOCTL  Calibration Data for 1MHz */
	 // So DCO is main clock source whose value is 1mhz,
	 BCSCTL2 &= ~( SELS | DIVS0 | DIVS1);
	 //now SMCLK will be used for periphrals, smclk by default is derived from dcoclk and its divider is 1
	 //so SMCLK will be used for deriving uart peripheral
	 uart_init();
	 P1DIR |= 0x01;					// configure P1.0 as output
	 volatile unsigned int i;		// volatile to prevent optimization
	 uint8_t x;

	while(1)
	{
	  //  uart_tran_byte('k');

	    uart_tran_string("Kunal");

	    uart_tran_dec(38);

		P1OUT ^= 0x01;				// toggle P1.0
		for(i=20000; i>0; i--);     // delay
		x = uart_recv_byte();
		uart_tran_byte(x);
	    for(i=200; i>0; i--);     // delay
	}
}
