#include <msp430.h>				
#include <stdbool.h>
#include <stdint.h>

/**
 * blink.c
 */
#define RED_LED 6
#define GREEN_LED 0
#define Switch 3

#define OUTPUT 1
#define INPUT 0

//Macro Variables for equating State in digitalWrite()
#define HIGH 1
#define LOW 0

void pinMode( uint8_t pin , uint8_t volatile *pin_data_direction_register, bool Mode )
{
    if(Mode==OUTPUT)
    *pin_data_direction_register|= (1 << pin);
    else if(Mode==INPUT)
    *pin_data_direction_register&= ~(1 << pin);
}
void digitalWrite( uint8_t pin ,uint8_t volatile *port_data_pin_register, bool state)
{
    if(state==HIGH)
    *port_data_pin_register|= (1 << pin);
    else if(state==LOW)
    *port_data_pin_register&= ~(1 << pin);
}
uint8_t digitalRead( uint8_t pin , uint8_t volatile *port_input_pin_register)
{
    return ((*port_input_pin_register) & (1<<pin));
}



void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;		// stop watchdog timer
	volatile unsigned int i;        // volatile to prevent optimization
	pinMode(GREEN_LED, &P1DIR, OUTPUT);
	pinMode(Switch, &P1DIR, INPUT);
	while(1)
	{

        if(digitalRead(Switch, &P1IN))
            digitalWrite(GREEN_LED, &P1OUT, HIGH);
        else
            digitalWrite(GREEN_LED, &P1OUT, LOW);
	}
}
