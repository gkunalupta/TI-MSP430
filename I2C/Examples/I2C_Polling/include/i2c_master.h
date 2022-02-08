/*
 * i2c_master.h
 *
 *  Created on: 08-Feb-2022
 *      Author: kunal
 */

#ifndef INCLUDE_I2C_MASTER_H_
#define INCLUDE_I2C_MASTER_H_

#include <stdbool.h>
#include <stdint.h>
#include <msp430.h>


void i2c_pins_conf();
void i2c_master_init();
void i2c_start();
void i2c_master_tran_byte( uint8_t data);
void i2c_stop();

#endif /* INCLUDE_I2C_MASTER_H_ */
