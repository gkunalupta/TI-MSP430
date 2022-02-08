/*
 * i2c_master.c
 *
 *  Created on: 08-Feb-2022
 *      Author: kunal
 */

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>

#define gb_pcf8574_ADDR  0b0100111
#define gb_pcf8574A_ADDR 0b0111111  //Slave Write
#define GB_RTC_ADDR 0b1101000
#define SSD1306_WA 0b0111100

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
    UCB0I2CSA = gb_pcf8574A_ADDR; // writing desired slave address on UCBxI2CSA register
    UCB0CTL1 |= UCTR; // setting in master transmitting mode

    UCB0CTL1 &= ~UCSWRST;

     //  UCB0I2CSA = gb_pcf8574_ADDR; // writing desired slave address on UCBxI2CSA register
    //   UCB0I2CSA = GB_RTC_ADDR; // writing desired slave address on UCBxI2CSA register
     //  UCB0CTL0 &= ~(UCSLA10);  // selecting size of slave address as 7 bit
      // UCB0CTL1 |= UCTR; // setting in master transmitting mode
   //  IFG2 &= ~UCB0TXIFG;
       UCB0CTL1 |= UCTXSTT; // Start condition generated
      //while((UCB0STAT & BUSY)!= 0);
       while(!(UC0IFG  & UCB0TXIFG)); // wait for start condition to be generated
    //  IFG2 &= ~UCB0TXIFG;
 //     while(!(UCB0CTL1 & UCTXSTT)); // waiting for slave to ack address
}
void i2c_master_tran_byte( uint8_t data)
{
  // while((UCB0STAT & BUSY)!= 0);
   //while(!(UC0IFG  & UCB0TXIFG)); // wait for start condition to be transmitted
    UCB0TXBUF = data;
  while(!(UC0IFG  & UCB0TXIFG)); // wait for start condition to be transmitted
}

void i2c_stop()
{
    while((UCB0STAT & BUSY)!= 0);
        UCB0CTL1 |= UCTXSTP;
}
