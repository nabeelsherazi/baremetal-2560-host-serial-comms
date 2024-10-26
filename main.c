// Copyright (c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
// Inspired and modified code
// https://balau82.wordpress.com/2011/03/29/programming-arduino-uno-in-pure-c/
// This is to blink the Elegoo onboard LED

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <util/setbaud.h>

#define BLINK_DELAY_MS 1000

void UART_Init(void) {
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;

#if USE_2X
  UCSR0A |= _BV(U2X0);
#else
  UCSR0A &= ~(_BV(U2X0));
#endif

  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

int uart_putchar(char c, FILE* stream) {
  if (c == '\n') {
    uart_putchar('\r', stream);
  }
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
}

int uart_getchar(FILE* stream) {
  loop_until_bit_is_set(UCSR0A, RXC0);
  return UDR0;
}

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

int main(void) {
  /* set pin 7 of PORTB for output*/
  DDRB |= _BV(DDB7);

  /* setup UART */
  UART_Init();
  stdout = &uart_output;
  stdin = &uart_input;

  puts("Booted successfully!");

  while (1) {
    /* set pin 7 high to turn led on */
    PORTB |= _BV(PORTB7);
    _delay_ms(BLINK_DELAY_MS);

    /* set pin 7 low to turn led off */
    PORTB &= ~_BV(PORTB7);
    _delay_ms(BLINK_DELAY_MS);

    /* write to UART*/
    char input;
    puts("Hello world!");
  }
}