# Variables
SOURCE = main.c
TARGET = firmware
MCU = atmega2560
PARTNO = m2560
PROGRAMMER = wiring
F_CPU = 16000000UL
PORT = /dev/ttyACM0
BAUD = 9600

# Compiler and flags
CC = avr-gcc
CFLAGS = -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU) -DBAUD=$(BAUD)
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
AVRDUDE_FLAGS = -c $(PROGRAMMER) -p $(PARTNO) -P $(PORT) -b 115200 -D # Use -D to disable auto erase for flash

# Targets

.PHONY: clean

all: build

# Build target
build: $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET).elf $(SOURCE)
	$(OBJCOPY) -O ihex -R .flash $(TARGET).elf $(TARGET).hex

# Flash target
flash: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -V -U flash:w:$(TARGET).hex

# Clean target
clean:
	rm $(TARGET).elf $(TARGET).hex