# AeroBing Flight Computer Source Code (needs a name)

## Overview
This project contains the firmware for AeroBing's flight computer. This includes sensor drivers, code for transmission and recovery, and hopefully eventually parachute deployment and apogee detection. We currently use the Teensy 4.1 as our microcontroller.

## Some important directories
* /lib contains all sensor driver libraries, the custom Ublox GPS driver, the communication protocol library (comms), and, of course, the SHART library
* /src contains the .cpp files that will actually be executed (i.e., those containing void() and loop())
* /python contains scripts that parse packets and read .poop files

## Usage
* First install the PlatformIO extension in VSCode
* Wait for it to configure the project
* Run 'pio run -t upload -e [environment]' in the PlatformIO terminal to build and upload code (environments are specified in platformio.ini)
* Socket icon in top right for serial monitor
* alt + z to make serial monitor print nicely without overflowing onto the next lines
* That's it! Refer to other READMEs for library-specific information.

