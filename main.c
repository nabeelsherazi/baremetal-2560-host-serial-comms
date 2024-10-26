// Adapted from https://github.com/enthusiasticgeek/Elegoo_Mega_2560

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <util/setbaud.h>

#include <pb_decode.h>
#include <pb_encode.h>
#include "generated/string.pb.h"

#include "cobs-c/cobs.h"

#define BLINK_DELAY_MS 1000
#define ERROR_BLINK_DELAY_MS 100

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
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
}

int uart_getchar(FILE* stream) {
  loop_until_bit_is_set(UCSR0A, RXC0);
  return UDR0;
}

void error_blink_forever() {
  while (1) {
    /* set pin 7 high to turn led on */
    PORTB |= _BV(PORTB7);
    _delay_ms(ERROR_BLINK_DELAY_MS);

    /* set pin 7 low to turn led off */
    PORTB &= ~_BV(PORTB7);
    _delay_ms(ERROR_BLINK_DELAY_MS);
  }
}

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

/* This is the buffer where we will store our message. */
uint8_t buffer[128];
size_t message_length;
bool status;

__attribute__((noreturn)) void main(void) {
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

    /* Encode our message */
    {
      /* Allocate space on the stack to store the message data.
       *
       * Nanopb generates simple struct definitions for all the messages.
       * - check out the contents of simple.pb.h!
       * It is a good idea to always initialize your structures
       * so that you do not have garbage data from RAM in there.
       */
      StringMessage message = StringMessage_init_zero;

      /* Create a stream that will write to our buffer. */
      pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

      snprintf(message.payload, sizeof(message.payload), "Hello, world!");

      /* Now we are ready to encode the message! */
      status = pb_encode(&stream, StringMessage_fields, &message);
      message_length = stream.bytes_written;

      /* Then just check for any errors.. */
      if (!status) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        error_blink_forever();
      }

      /* Payload the message into a COBS frame */
      uint8_t cobs_buffer[128];
      size_t cobs_length;
      cobs_encode_result result =
          cobs_encode(cobs_buffer, sizeof(cobs_buffer), buffer, message_length);
      if (result.status != COBS_ENCODE_OK) {
        printf("COBS encoding failed: %d\n", result.status);
        error_blink_forever();
      }

      cobs_length = result.out_len;
      cobs_buffer[cobs_length] = 0; /* Add a zero byte at the end */

      //   printf("Transmitted COBS frame: %d bytes", cobs_length);

      /* transmit the COBS frame over serial */
      for (size_t i = 0; i < cobs_length + 1; i++) {
        putchar(cobs_buffer[i]);
      }
    }
  }
}