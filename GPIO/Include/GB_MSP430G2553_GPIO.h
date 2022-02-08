/*
 * GB_MSP430G2553_GPIO.h
 *
 *  Created on: 06-Feb-2022
 *      Author: kunal
 */
#include <msp430.h>
#ifndef INCLUDE_GB_MSP430G2553_GPIO_H_
#define INCLUDE_GB_MSP430G2553_GPIO_H_

void pinMode( uint8_t pin , uint8_t volatile *pin_data_direction_register, bool Mode );
void digitalWrite( uint8_t pin ,uint8_t volatile *port_data_pin_register, bool state);
uint8_t digitalRead( uint8_t pin , uint8_t volatile *port_input_pin_register);
// Macro variables for equating MODE in pinMode()
#define OUTPUT 1
#define INPUT 0

//Macro Variables for equating State in digitalWrite()
#define HIGH 1
#define LOW 0

#endif /* INCLUDE_GB_MSP430G2553_GPIO_H_ */
