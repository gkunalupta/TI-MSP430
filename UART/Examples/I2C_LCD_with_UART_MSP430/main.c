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
void delay_init()
{
   TACTL |= TASSEL_2; // SMCLK Clock source selected , as SMCL = 1MHZ so that is also the timer peripheral input clock
                       // as Timer clock is 1MHZ, so transition for each count of timer counter will take 1us time
   // Timer is halted when MCx bits =0, to enable the timer MCx bits > 0
   // So we will start the timer in delay functions only by selecting the Continous mode
   //which is done by selecting MCx = 10
}
void delay_us(uint16_t us)
{
    TACTL |= MC_2; // continous mode of timer selected, enabling the timer
    TAR = 0;  // Clearing the Timer counter register by equating it to 0
    while(TAR <= us); // wait for Timer to count
  //  TACTL |= MC_0; // making the timer stop
}
void delay_ms(uint16_t ms)
{
    uint16_t i;
      for (i=0; i<=ms; i++)
    delay_us(1000);
}
void delay_sec(uint16_t secs)
{
    uint16_t i;
    for (i=0; i<=secs; i++)
    delay_ms(1000);
}
void gb_i2c_pins_conf()
{
    P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
}
void gb_i2c_master_init()
{
    gb_i2c_pins_conf();

    UCB0CTL1 |= UCSWRST;                      // set the UCSSWRST bit, so as to keep USCI in reset condition
                                              // configuration of USCI module should be done when USCI is reset to avoid unpredictable behavious
    UCB0CTL0 |= (UCMST | UCMODE_3 | UCSYNC);     // I2C Master, synchronous mode
    UCB0CTL1 |=  UCSSEL_2; // use SMCLK
    UCB0BR0 = 10;
    UCB0BR1 = 0;
}
void gb_i2c_master_start()
{

    UCB0CTL1 &= ~UCSWRST;

    UCB0I2CSA = gb_pcf8574A_ADDR; // writing desired slave address on UCBxI2CSA register
    UCB0CTL1 |= UCTR; // setting in master transmitting mode

    UCB0CTL1 |= UCTXSTT; // Start condition generated

    while(!(UC0IFG  & UCB0TXIFG)); // wait for start condition to be generated
}
void gb_i2c_master_send_byte( uint8_t data)
{
    UCB0TXBUF = data;
  while(!(UC0IFG  & UCB0TXIFG)); // wait for start condition to be transmitted
}

void gb_i2c_master_stop()
{
    while((UCB0STAT & BUSY)!= 0);
    UCB0CTL1 |= UCTXSTP;
}


void uart_baud_rate()
{
    UCA0CTL1 |= UCSSEL1; // clock source for uart is selected SMCLK, UCSSELx bits = 10 for that,
      UCA0BR0 = 104;                            // 1MHz 9600
      UCA0BR1 = 0;                              // 1MHz 9600
      UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
}

void uart_pins_conf()
{
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;
}
void uart_init()
{

    UCA0CTL1 = UCSWRST;
    uart_baud_rate();
    uart_pins_conf();
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
   unsigned char ptr;
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

//Function Set
//Rs RW DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
// 0 0  0    0   1   DL  N   F   -   -
// 8bit (DL=1) , 4bit (DL=0)
// 1line (N=0) , 2line (N-1)
#define gb_lcd_8bit_1line 0b00110000
#define gb_lcd_8bit_2line 0b00111000
#define gb_lcd_4bit_1line 0b00100000
#define gb_lcd_4bit_2line 0b00101000

//Display ON/OFF control -
//Rs RW DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
// 0 0  0    0   0   0   1   D   C   B
//DN- display on(D=1), CN- Cursor on(C=1 ), BN- blink on(B=1)
//DF-display off(D=0), CF- cursor off(C=1), BF- blink off(B=0)
#define gb_lcd_DN_CN_BN 0b00001111
#define gb_lcd_DN_CF_BF 0b00001100
#define gb_lcd_DN_CN_BF 0b00001110
#define gb_lcd_DF_CF_BF 0b00001000
#define gb_lcd_DF_CN_BN 0b00001011

//Entry mode set
//Rs RW DB7 DB6 DB5 DB4 DB3 DB2 DB1  DB0
// 0 0  0    0   0   0   0   1  I/D  S/H
//CMR -cursor move right(I/D=1) , DMF- display move off(S/H=0)
//CML -cursor move left(I/D=0) , DMR - display move right(S/H=1)
//DML- display move left(S/H=1)
#define gb_lcd_CMR_DMF 0b00000110
#define gb_lcd_CML_DMF 0b00000100
#define gb_lcd_CMR_DMR 0b00000111
#define gb_lcd_CML_DML 0b00000101

#define gb_lcd_clear_all 0b00000001
#define gb_lcd_rhome     0b00000010
#define gb_lcd_SETDDRAMADDR 0x80

/*
// macros for rs,rw,en pins of lcd
#define En 00000100  // Enable bit
#define Rw 00000010  // Read/Write bit
#define Rs 00000001  // Register select bit
*/


#define gb_pcf8574_WADDR 0b01111110  //Slave Write
//#define gb_pcf8574_WADDR 0b01001110
#define gb_pcf8574_RADDR 0b01111111   //Slave Read
#define gb_LCD_BACKLIGHT         0x08
#define gb_LCD_NOBACKLIGHT       0x00
#define gb_display_rate 50


// For Custum characters

#define gb_lcd_SETDDRAMADDR 0x80
#define gb_lcd_SETCGRAMADDR 0x40


const uint8_t gb_LCD_En = 0x04; //00000100
uint8_t gb_LCD_Rw = 0x00; //00000000: 1st bit
uint8_t gb_LCD_Rs = 0x01; //00000001


void GB_lcd_ioexander(unsigned char gb_Data)
{
   gb_i2c_master_start();
   gb_i2c_master_send_byte(gb_Data | gb_LCD_BACKLIGHT);
   //TWI_write_data(Data);
   gb_i2c_master_stop();
}

void GB_pulse_enable( uint8_t gb_data)
{
   GB_lcd_ioexander(gb_data | gb_LCD_En);  //enable pulse high
   delay_us(1);
   GB_lcd_ioexander(gb_data & ~gb_LCD_En); //enable pulse low
   delay_us(50);
}

void GB_lcd_write4bit(unsigned char gb_nibble)
{
   gb_nibble |= (gb_LCD_Rs|gb_LCD_Rw);
   GB_lcd_ioexander(gb_nibble | 0x04);
   delay_us(1);
   GB_lcd_ioexander(gb_nibble & 0xFB);
   delay_us(1);
   delay_ms(gb_display_rate);
}
/*
For command register: RS=0,RW=0

*/
void GB_LCD_command(unsigned char gb_command)
{
   gb_LCD_Rs = 0b00000000;
   gb_LCD_Rw = 0b00000000;
   uint8_t gb_highnib = gb_command & 0xf0;
   uint8_t gb_lownib = (gb_command<<4) & 0xf0;
   GB_lcd_write4bit(gb_highnib);
   GB_lcd_write4bit(gb_lownib);
}
void GB_lcd_setcursor(uint8_t gb_col, uint8_t gb_row)
{
   static uint8_t gb_offsets[] = {0x00,0x40,0x14,0x54 };
   //LCD_command(lcd_SETDDRAMADDR | (col + offsets[row]));
   GB_LCD_command((0x80|(gb_row<<6))+gb_col);
   delay_us(40);
}
void GB_lcd_clear()
{
   GB_LCD_command(gb_lcd_clear_all); //clear display
   delay_ms(20);
}
void GB_lcd_Rethome()
{
   GB_LCD_command(gb_lcd_rhome);
   delay_ms(2);
}


/*
For Data Register: RS=1,RW=0
*/
void GB_LCD_data(unsigned char gb_data)
{
   gb_LCD_Rs = 0b00000001;
   gb_LCD_Rw = 0b00000000;
   uint8_t gb_highnib = gb_data & 0xf0;
   uint8_t gb_lownib = (gb_data<<4) & 0xf0;
   GB_lcd_write4bit(gb_highnib);
   GB_lcd_write4bit(gb_lownib);
}
/*
for printing integers in form 001,023,015,006,007
*/
void GB_lcd_printint(int gb_value)
{

   unsigned char gb_thousands,gb_hundreds,gb_tens,gb_ones;

   gb_thousands = gb_value / 1000;
   if(gb_thousands!=0)
   GB_LCD_data(gb_thousands + 0x30);

   gb_hundreds = ((gb_value - gb_thousands*1000)) / 100;
   //if(hundreds!=0)
   GB_LCD_data( gb_hundreds + 0x30);

   gb_tens=(gb_value%100)/10;
   //if(tens!=0)
   GB_LCD_data( gb_tens + 0x30);

   gb_ones=gb_value%10;
   //if(ones!=0)
   GB_LCD_data( gb_ones + 0x30);

}
/*
for printing integers in form 1,23,15,6,7
Mainly used for printing 0-9
*/
void GB_lcd_printint_num(int gb_value)
{

   unsigned char gb_thousands,gb_hundreds,gb_tens,gb_ones;

   gb_thousands = gb_value / 1000;
   if(gb_thousands!=0)
   GB_LCD_data(gb_thousands + 0x30);

   gb_hundreds = ((gb_value - gb_thousands*1000)-1) / 100;
   if(gb_hundreds!=0)
   GB_LCD_data( gb_hundreds + 0x30);

   gb_tens=(gb_value%100)/10;
   if(gb_tens!=0)
   GB_LCD_data( gb_tens + 0x30);

   gb_ones=gb_value%10;
   //if(ones!=0)
   GB_LCD_data( gb_ones + 0x30);

}
/*
send string function
*/
void GB_LCD_string(const char*gb_str)
{
   unsigned int gb_i;
   for(gb_i=0;gb_str[gb_i]!=0;gb_i++)
   {

       GB_LCD_data(gb_str[gb_i]);
       delay_us(45);
   }
}
void GB_lcd_print1line(const char *gb_buff)
{
   GB_lcd_setcursor(0,0);
   GB_LCD_string(gb_buff);
}
void GB_lcd_print2line(const char *gb_buff)
{

   GB_lcd_setcursor(0,1);
   GB_LCD_string(gb_buff);
}
void GB_lcd_bit(unsigned char gb_val)
{
   //lcd_setcursor(0,1);
   //_delay_us(40);
   int  gb_ptr;
   for(gb_ptr=7;gb_ptr>=0;gb_ptr--)
   {
       if ((gb_val & (1<<gb_ptr))==0)
       {
           GB_LCD_string("0");
       }
       else
       {
           GB_LCD_string("1");
       }
   }
}


/*
* for printing patterns
* */
void GB_setramaddr(uint8_t gb_address)
{
   GB_LCD_command(gb_address);
   //delay_us(10);

}
/*
This function is for sending custom made
characters to address of CGRAM(0x40-0x7F)
Each custom character need 8 CGRAM location
so only 8 custom characters can be made each character
occupying 8 address location of CGRAM.
So at these locations Custom Characters are stored:
(0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78).
Before storing custom characters to these location,
we have to SET CGRAM address to any of these:
(0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78).That is done by command
setramaddr((lcd_SETCGRAMADDR)) in the main function
*/
void GB_LCD_sendpattern(uint8_t gb_loc,  uint8_t gb_pattern[])
{
    unsigned int gb_i;
   if(gb_loc<8)
   {
   GB_setramaddr((gb_lcd_SETCGRAMADDR+(gb_loc*8)));
   for(gb_i=0;gb_i<8;gb_i++)
   GB_LCD_data(gb_pattern[gb_i]);
       //_delay_us(45);
   }
}
void GB_LCD_printpattern(uint8_t gb_loc)
{

   GB_LCD_data((0x00+gb_loc));
}
   void GB_LCD_init()
   {
       gb_i2c_master_init();
       delay_us(100);
       GB_lcd_ioexander(0x00);
       //printString0("kunal\n");
       delay_ms(25);  // LCD Power ON delay always >15ms
       {
           GB_LCD_command(gb_lcd_4bit_2line); //Function set -->8-bit mode is selected,2lines
           delay_ms(5);
           GB_LCD_command(gb_lcd_4bit_2line); //Function set -->8-bit mode is selected,2lines
           delay_us(160);
           GB_LCD_command(gb_lcd_4bit_2line); //Function set -->8-bit mode is selected,2lines
       }
       GB_LCD_command(gb_lcd_4bit_2line); //Function set -->8-bit mode is selected,2lines
       delay_us(45);
       GB_LCD_command(gb_lcd_DN_CF_BF); // Display ON/OFF control -->dispaly is on ,cursor is on and cursor blink is off
       delay_ms(1);
       //lcd_clear();
       GB_LCD_command(gb_lcd_CML_DMF); // Entry mode set --> cursor moves to right and DRAM is incremented by 1 , shift of display is off
       delay_us(40);

       //printString0("gupta\n");
   }

void main(void)
{

    clock_init();
    delay_init();

    WDTCTL = WDTPW | WDTHOLD;       // stop watchdog timer
    P1DIR |= 0x01;                  // configure P1.0 as output

    uint8_t x;
    uart_init();

    GB_LCD_init();
    GB_lcd_clear();
    GB_lcd_setcursor(0,0);
    GB_LCD_string("Hii Ritwik");
    delay_ms(10);
    uart_tran_string("************UART Driver by polling***************\n");
    while(1)
    {
        GB_lcd_setcursor(0,1);
        GB_LCD_string("Recv data is:");
        x = uart_recv_byte();


        //GB_lcd_clear();
        GB_lcd_setcursor(13,1);
        GB_LCD_data(x);
        delay_ms(10);

        P1OUT ^= 0x01;              // toggle P1.0
         //delay_ms(1000);
        //for(i=10000; i>0; i--);     // delay
    }
}
