// Adapted from https://github.com/enthusiasticgeek/Elegoo_Mega_2560

#include <avr/io.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <stdio.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include "cobs-c/cobs.h"
#include "generated/string.pb.h"

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
  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);  // 8-bit data

  // Configure even parity
  UCSR0C |= _BV(UPM01);
  UCSR0C &= ~(_BV(UPM00));

  UCSR0B = _BV(RXEN0) | _BV(TXEN0);  // Enable RX and TX
}

int UART_PutChar(char c, FILE* stream) {
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
  return 0;
}

int UART_GetChar(FILE* stream) {
  loop_until_bit_is_set(UCSR0A, RXC0);
  return UDR0;
}

void Blink_Error() {
  while (1) {
    PORTB ^= _BV(PORTB7);  // Toggle LED on/off
    _delay_ms(ERROR_BLINK_DELAY_MS);
  }
}

// UART file streams
FILE uart_output = FDEV_SETUP_STREAM(UART_PutChar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, UART_GetChar, _FDEV_SETUP_READ);

// Global counter
uint32_t counter = 0;

void Transmit_COBS_Frame(const uint8_t* data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    putchar(data[i]);
  }
}

void Encode_And_Send_Message() {
  StringMessage message = StringMessage_init_zero;
  snprintf(message.payload, sizeof(message.payload), "Hello, world! #%lu",
           counter++);

  uint8_t message_buffer[128];
  pb_ostream_t stream =
      pb_ostream_from_buffer(message_buffer, sizeof(message_buffer));

  if (!pb_encode(&stream, StringMessage_fields, &message)) {
    printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
    Blink_Error();
  }

  uint8_t write_buffer[128];
  cobs_encode_result result = cobs_encode(write_buffer, sizeof(write_buffer),
                                          message_buffer, stream.bytes_written);

  if (result.status != COBS_ENCODE_OK) {
    printf("COBS encoding failed: %d\n", result.status);
    Blink_Error();
  }

  // Add delimiter byte for COBS (cobs-c doesn't do this automatically)
  write_buffer[result.out_len] = 0;
  Transmit_COBS_Frame(write_buffer, result.out_len + 1);
}

__attribute__((noreturn)) void main(void) {
  DDRB |= _BV(DDB7);  // Set pin 7 of PORTB for output
  UART_Init();
  stdout = &uart_output;
  stdin = &uart_input;

  puts("Booted successfully!");

  while (1) {
    PORTB ^= _BV(PORTB7);  // Toggle LED
    _delay_ms(BLINK_DELAY_MS);
    Encode_And_Send_Message();
  }
}
