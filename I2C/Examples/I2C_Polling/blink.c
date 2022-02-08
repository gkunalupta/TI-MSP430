#include <msp430.h>				
#include <stdbool.h>
#include <stdint.h>
//#include "i2c_master.h"

/**
 * blink.c
 */

#define gb_pcf8574A_ADDR 0b0111111  //Slave Write


void clock_init()
{

    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                 /* BCSCTL1 Calibration Data for 1MHz */
    DCOCTL = CALDCO_1MHZ;                  /* DCOCTL  Calibration Data for 1MHz */
    // So DCO is main clock source whose value is 1mhz,
    BCSCTL2 &= ~( SELS | DIVS0 | DIVS1);
    //now SMCLK will be used for periphrals, smclk by default is derived from dcoclk and its divider is 1
    //so SMCLK will be used for deriving uart peripheral
}

void i2c_pins_conf()
{
    P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
}
void i2c_master_init()
{
    i2c_pins_conf();

    UCB0CTL1 |= UCSWRST;                      // set the UCSSWRST bit, so as to keep USCI in reset condition
                                              // configuration of USCI module should be done when USCI is reset to avoid unpredictable behavious
    UCB0CTL0 |= (UCMST | UCMODE_3 | UCSYNC);     // I2C Master, synchronous mode
    UCB0CTL1 |=  UCSSEL_2; // use SMCLK
    UCB0BR0 = 10;
    UCB0BR1 = 0;
}
void i2c_start()
{

    UCB0CTL1 &= ~UCSWRST;

    UCB0I2CSA = gb_pcf8574A_ADDR; // writing desired slave address on UCBxI2CSA register
    UCB0CTL1 |= UCTR; // setting in master transmitting mode

    UCB0CTL1 |= UCTXSTT; // Start condition generated

    while(!(UC0IFG  & UCB0TXIFG)); // wait for start condition to be generated
}
void i2c_master_tran_byte( uint8_t data)
{
    UCB0TXBUF = data;
  while(!(UC0IFG  & UCB0TXIFG)); // wait for start condition to be transmitted
}

void i2c_stop()
{
    while((UCB0STAT & BUSY)!= 0);
    UCB0CTL1 |= UCTXSTP;
}
void main(void)
{

	WDTCTL = WDTPW | WDTHOLD;		// stop watchdog timer
	clock_init();
	P1DIR |= 0x01;					// configure P1.0 as output

	volatile unsigned int i;		// volatile to prevent optimization
	 i2c_master_init();
	while(1)
	{
	    i2c_start();
	    i2c_master_tran_byte( 'y');
	    i2c_stop();

		P1OUT ^= 0x01;				// toggle P1.0
		for(i=10000; i>0; i--);     // delay
	}
}
