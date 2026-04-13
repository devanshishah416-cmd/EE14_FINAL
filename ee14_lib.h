/**
 * @file ee14lib.h
 * @brief Complete EE14 library header for the STM32L432KC Nucleo board.
 *
 * This is the single header to include in every file in your project.
 * It declares every function that exists across:
 *   gpio.c, uart.c, serial.c, adc.c, timer.c, i2c.c
 *
 * Use this version — it has everything in one place.
 */

#ifndef EE14LIB_H
#define EE14LIB_H

#include "stm32l432xx.h"
#include <stdbool.h>
#include <stdint.h>

/* ═══════════════════════════════════════════════════════════════════════
   PIN NAMES
   Maps the Nucleo silkscreen labels to enum values.
   VCP_RX is hard-wired to the host bridge, not broken out to headers.
   ═════════════════════════════════════════════════════════════════════ */
typedef enum {
    A0, A1, A2, A3, A4, A5, A6, A7,
    D0, D1, D2, D3, D4, D5, D6, D7,
    D8, D9, D10, D11, D12, D13,
    VCP_RX
} EE14Lib_Pin;

/* ═══════════════════════════════════════════════════════════════════════
   ERROR CODES
   ═════════════════════════════════════════════════════════════════════ */
typedef int EE14Lib_Err;
#define EE14Lib_Err_OK                 0
#define EE14Lib_Err_INEXPLICABLE_FAILURE  -1
#define EE14Lib_Err_NOT_IMPLEMENTED    -2
#define EE14Lib_ERR_INVALID_CONFIG     -3

/* ═══════════════════════════════════════════════════════════════════════
   GPIO CONSTANTS
   ═════════════════════════════════════════════════════════════════════ */

/* Pin direction */
#define INPUT               0b00
#define OUTPUT              0b01
#define ALTERNATE_FUNCTION  0b10
#define ANALOG              0b11

/* Pull-up / pull-down */
#define PULL_OFF   0b00
#define PULL_UP    0b01
#define PULL_DOWN  0b10

/* Output type */
#define PUSH_PULL   0b0
#define OPEN_DRAIN  0b1

/* Output speed */
#define LOW_SPD   0b00
#define MED_SPD   0b01
#define HI_SPD    0b10
#define V_HI_SPD  0b11

/* ═══════════════════════════════════════════════════════════════════════
   GPIO  (gpio.c)
   ═════════════════════════════════════════════════════════════════════ */
EE14Lib_Err gpio_config_alternate_function(EE14Lib_Pin pin, unsigned int function);
EE14Lib_Err gpio_config_mode   (EE14Lib_Pin pin, unsigned int mode);
EE14Lib_Err gpio_config_pullup (EE14Lib_Pin pin, unsigned int mode);
EE14Lib_Err gpio_config_otype  (EE14Lib_Pin pin, unsigned int otype);
EE14Lib_Err gpio_config_ospeed (EE14Lib_Pin pin, unsigned int ospeed);
void        gpio_write         (EE14Lib_Pin pin, bool value);
bool        gpio_read          (EE14Lib_Pin pin);

/* ═══════════════════════════════════════════════════════════════════════
   SERIAL / UART  (uart.c / serial.c)
   ═════════════════════════════════════════════════════════════════════ */

/* Initialize USART2 (the one wired to the USB debug port on the Nucleo) */
void host_serial_init(const unsigned int baud);

/* Blocking write — waits until every byte is sent */
void serial_write(USART_TypeDef *USARTx, const char *buffer, int len);

/* Non-blocking write — sends as many bytes as the TX buffer will take right now,
   returns the number actually written */
int  serial_write_nonblocking(USART_TypeDef *USARTx, const char *buffer, int len);

/* Blocking read — spins until one byte arrives */
char serial_read(USART_TypeDef *USARTx);

/* ═══════════════════════════════════════════════════════════════════════
   ADC  (adc.c)
   ═════════════════════════════════════════════════════════════════════ */

/* Internal: powers up the ADC hardware. Called automatically by
   adc_config_single() — you should not need to call this directly. */
void adc_wakeup(void);

/* Configure the ADC to read from a single pin.
   Call once per pin change, then call adc_read_single() repeatedly.
   Returns EE14Lib_ERR_INVALID_CONFIG if the pin has no ADC channel. */
EE14Lib_Err  adc_config_single(const EE14Lib_Pin pin);

/* Trigger one conversion and return the result.
   Range: 0–4095 (12-bit, right-aligned, which is the default). */
unsigned int adc_read_single(void);

/* ═══════════════════════════════════════════════════════════════════════
   TIMER  (timer.c)
   ═════════════════════════════════════════════════════════════════════ */

/* Configure a timer in free-running (count-up) mode.
   prescaler divides the 80 MHz system clock down to a usable tick rate.
   Example: prescaler=79 → 1 tick per microsecond. */
EE14Lib_Err timer_config_freerun(TIM_TypeDef* const timer,
                                  const unsigned int prescaler);

/* Read the current counter value of a free-running timer. */
uint32_t    timer_get_count(TIM_TypeDef* const timer);

/* ═══════════════════════════════════════════════════════════════════════
   I2C  (i2c.c)
   ═════════════════════════════════════════════════════════════════════ */
EE14Lib_Err i2c_init (I2C_TypeDef* i2c, EE14Lib_Pin scl, EE14Lib_Pin sda);
bool        i2c_write(I2C_TypeDef* i2c, unsigned char device_address,
                      unsigned char* data, unsigned char len);
bool        i2c_read (I2C_TypeDef* i2c, unsigned char device_address,
                      unsigned char* data, unsigned char len);

#endif /* EE14LIB_H */
