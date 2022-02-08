/*
 * GB_MSP430G2553_GPIO.c
 *
 *  Created on: 06-Feb-2022
 *      Author: kunal
 */
#include <stdbool.h>
#include <stdint.h>

#include <GB_MSP430G2553_GPIO.h>
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
