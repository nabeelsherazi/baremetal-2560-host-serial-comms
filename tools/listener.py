#! /usr/bin/env python3

from cobs import cobs
import serial
import string_pb2
import google.protobuf.message
import argparse
import logging

# Parse command line arguments
parser = argparse.ArgumentParser(
    description="Serial listener for COBS-encoded messages"
)
parser.add_argument(
    "--port", type=str, default="/dev/ttyACM0", help="Serial port to listen on"
)
parser.add_argument(
    "--baudrate", type=int, default=9600, help="Baud rate for serial connection"
)
parser.add_argument(
    "--debug", action="store_true", help="Print debug messages to console"
)

args = parser.parse_args()

# Set up logging
logging.basicConfig(
    level=logging.DEBUG if args.debug else logging.INFO,
    format="%(asctime)s - %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger()

# Set up the serial connection
ser = serial.Serial(args.port, args.baudrate, timeout=1, parity=serial.PARITY_EVEN)
buffer = bytearray()  # Buffer to store incoming bytes until a full frame is received

while True:
    # Read incoming bytes
    data = ser.read(8)
    if not data:
        continue

    logger.debug(f"(Read {len(data)} bytes)")

    # Append data to buffer
    buffer.extend(data)
    logger.debug(f"Current buffer size: {len(buffer)}")
    logger.debug(f"(Buffer: {buffer})")

    # Check for delimiter (0x00), which marks the end of a COBS frame
    while b"\x00" in buffer:
        # Find the index of the first 0x00, marking the end of the frame
        delimiter_index = buffer.index(b"\x00")

        # Extract the frame (up to but not including the 0x00)
        frame = buffer[:delimiter_index]
        logger.debug(f"(Frame: {frame})")

        # Remove the frame and delimiter from the buffer
        buffer = buffer[delimiter_index + 1 :]

        # Decode the line and print it
        try:
            # Parse the protobuf message
            message = string_pb2.StringMessage()
            message.ParseFromString(cobs.decode(frame))
            logger.info(message.payload)

        except cobs.DecodeError as e:
            logger.error(f"Error decoding COBS frame: {e}")

        except google.protobuf.message.DecodeError as e:
            logger.error(f"Error decoding protobuf message: {e}")

        except Exception as e:
            logger.error(f"Unexpected error: {e}")
