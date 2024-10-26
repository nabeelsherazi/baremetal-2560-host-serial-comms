# Bare metal 2560 serial communication with nanopb and COBS

little example of bare metal programming an arduino mega 2560 r3 to send serial messages back over the connected USB cable.

messages are encoded with nanopb and framed with COBS.

a listener script on the other end depayloads and reads the messages.

## build & flash

makefile assumes your arduino is on /dev/ttyACM0. you can adjust this (and a lot of other knobs) in there if needed.

```
sudo apt-get install gcc-avr binutils-avr avr-libc gdb-avr avrdude protobuf-compiler

git submodule update --init --recursive

make build

make flash
```

## listen

```
pip3 install -r tools/requirements.txt

python3 tools/listener.py # add --debug for more verbosity
```

you should see "hello world!" being depayloaded, parsed, and printed, as well as the onboard LED blinking once a second.

if there's an encoding or payloading error, the onboard led will blink faster, at 10 times a second.