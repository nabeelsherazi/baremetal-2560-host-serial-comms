# Variables
MCU = atmega2560
PARTNO = m2560
PROGRAMMER = wiring
PORT = /dev/ttyACM0
F_CPU = 16000000UL
BAUD = 9600
TARGET = firmware
SOURCE_DIR = .
GENERATED_DIR = generated
PROTO_DIR = proto
EXTERNAL_DIR = external
NANOPB_DIR = $(EXTERNAL_DIR)/nanopb
TOOLS_DIR = tools

# Source files
SOURCES = $(wildcard $(SOURCE_DIR)/*.c) \
          $(GENERATED_DIR)/string.pb.c \
          $(NANOPB_DIR)/pb_encode.c \
          $(NANOPB_DIR)/pb_decode.c \
          $(NANOPB_DIR)/pb_common.c \
          $(EXTERNAL_DIR)/cobs-c/cobs.c

# Compiler and tools
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

# Compiler flags
CFLAGS = -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU) -DBAUD=$(BAUD) \
         -I$(NANOPB_DIR) -I$(EXTERNAL_DIR)

# AVRDUDE flags
AVRDUDE_FLAGS = -c $(PROGRAMMER) -p $(PARTNO) -P $(PORT) -b 115200 -D

# Include the nanopb provided Makefile rules
include $(NANOPB_DIR)/extra/nanopb.mk
PROTOC_OPTS = --proto_path=$(NANOPB_DIR)/generator/proto

# Targets
.PHONY: all build flash clean proto ython

all: build python

# Build target
build: $(TARGET).hex

$(TARGET).elf: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .flash $< $@

# Flash target
flash: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -V -U flash:w:$<

# Clean target
clean:
	rm -rf $(TARGET).elf $(TARGET).hex $(GENERATED_DIR) $(wildcard $(TOOLS_DIR)/*_pb2.py)

# Protocol buffer compilation
proto: $(GENERATED_DIR)/string.pb.c

$(GENERATED_DIR)/string.pb.c: $(PROTO_DIR)/string.proto
	mkdir -p $(GENERATED_DIR)
	$(PROTOC) $(PROTOC_OPTS) --nanopb_out=$(GENERATED_DIR) --proto_path=$(PROTO_DIR) $< nanopb.proto

# Generate Python protobuf bindings
python: $(TOOLS_DIR)/string_pb2.py

$(TOOLS_DIR)/string_pb2.py: $(PROTO_DIR)/string.proto
	protoc $(PROTOC_OPTS) --python_out=$(TOOLS_DIR) --proto_path=$(PROTO_DIR) $< nanopb.proto