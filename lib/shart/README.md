##### Aerobing Research Group 2024
## SHART Data Logger Library

<pre>
　　      🌸\ -  フ
　　　　　 |　 _  _,    mrrrp mrrau?
　 　　　／` ミ＿xノ
　　 　 /　　　 　 |
　　　 /　   　　 ﾉ     <--- my most important accomplishment in college
　 　 │　　|　へ |
　／￣|　　 \　\ \
　| (￣ヽ＿_ヽ_) _)
　＼二つ
</pre>

### Overview
SHART stands for Space and High Altitude Recovery and Telemetry (or something, rocket tracker?). We use widely available commercial sensors to estimate the rocket's altitude, velocity, position, trajectory, etc. This information helps us figure out what went wrong when things inevitably do go wrong, gives us an idea of how we are achieving our goals, and helps us recover the rocket after its flight.

At a high level, the system's functionality can be separated into two parts: collection and transmission. SHART reads data from peripheral sensors over SPI or serial interfaces. It then packages the collected data into a packed struct which can be represented as an array of bytes, stores it on an SD card or integrated memory chip, and sends it to the ground station via a radio module. There is other stuff going on, but this is the essence of SHART. The logic is separated across files as follows:

- The `Shart` class (declared in `shart.h`, split across multiple .cpp files) unifies all functionality with a minimal public interface. It includes methods for initializing everything, collecting data, reconnecting lost chips, and transmitting data. It handles data storage and communication with the minimal custom communications library, `comms.h`. The `Shart` class contains persistent data packet structs which are overwritten between collection cycles.
- `shart.cpp` contains implementations for everything in the public interface of the `Shart` class, including top-level functions for the initialization of Shart, collection from all sensors, and transmission. Everything in the other files is private to `Shart`.
- `sensors.cpp` contains the implementations for lower-level sensor-specific methods of the `Shart` class. Most of these methods are specific to a particular sensor, for example, `collectDataADXL375()` and `initBMP388()`. Most of these are simply written according to driver APIs.
- `gps.cpp` contains GNSS-specific functions. At each iteration of the loop, we check if there is new data from the GPS module, if so, we fill a gps packet and set the `gps_ready` flag.
- `export.cpp` contains the implementations for lower-level transmission and storage methods of the `Shart` class. This includes initialization of storage module and radio along with actual storage and transmission logic.

More details can be found in comments throughout the code. To use the library, simply include `shart.h`.

### Debugging
This library relies entirely on macros for debugging. The actual code for the macros is in `debug.h`. To enable and disable debugging settings, either uncomment or comment the respective definitions in the `shart.config` file:

- `DEBUG_MODE_ERROR` prints errors to serial, e.g. initialization or SD write failures.
- `DEBUG_MODE_DATARATE` prints the output data rate in Hz, every 1 second.
- `DEBUG_MODE_STATUS` prints a line to serial whenever the status of a sensor changes.
- `USB_SERIAL_MODE` sends everything over USB serial instead of radio serial.
- `START_ON_POWERUP` allows shart to start running immediately without receiving bytes.
- `ATTEMPT_RECONNECT` attempts to reinitialize lost chips

If you add a debugging option, make sure to update the README.

### Data Specifications
For details on the output data format, please refer to the `comms` library, which defines the packet headers, the structs for each packet type, and the checksum logic. These packets are designed to be packable, if possible, and should be easily parsable (e.g., using the `struct` library in Python). Python code to parse from serial or a `.poop` file is in `Aerobing-Firmware/python`

### Device Drivers
Most of the drivers that Shart depends on are modified versions of code found online. 

- While we restructured the library, most of the underlying logic of the Ublox NEO-7M driver is heavily based on Loginov's code from GitHub.
- The BMP3XX driver is essentially identical to the Adafruit BMP driver. Some changes have been made, including a (useful) chipID() function and a significant optimization; Adafruits's library uses FORCED mode by default, we changed this to NORMAL mode. We now avoid resetting configurations at each data read. The original may have intended to save power, but we found it too be intolerably slow.
- The ICM20948 library is from Zane Lindstrom on GitHub, who seems to have some insider knowledge about Invensense's proprietary firmware (it was the only library that really worked with the DMP). We did not touch the underlying C API, but we made significant improvements to the C++ interface.
- The ADXL375 library is unmodified from Adafruit.
- Note: in the future, we may switch away from the Adafruit libraries

## Changelog
### Version 0.1
- Created project outline
- Implemented code that only works in ideal conditions
### Version 0.2
- Added SD card reinitialization
- Added variable frame skips to SD card data saving
### Version 0.3
- Added BMP390 
- Added SD card reinitialization timeouts
- Added JY901 reinitialization 
- Added BMP390 reinitialization 
### Version 0.4
- Added LED component indicator lights
- Added radio compatibility
### Version 0.5
- Reworked GPS
### Version 0.6
- Refactored code for cleaner, object-oriented organization, SHART singleton class
- Implemented reconnect functionality for IMU, accelerometer, and altimeter
- Implemented threaded option for reconnects
- Transmission and storage in binary
- Removed ambiguous parallel .ino files
- Replaced JY901 with BNO055
- Replaced ADXL377 with ADXL375
- Added debugging macros
- Migrated to PlatformIO
### Version 0.7
- radio communication protocol
- switched to SDFat, faster sd library
### Version 1.0
- clear distinction in config between usb mode and radio mode (for both sending and receiving packets)
- switched to lsm6dso32 6dof IMU, reduced sensor packet size by 8 bytes
    - Shart passes RAW data (int16_t) to the ground station. Python now handles conversion from raw to actual data (we will refer to the _read() function at https://github.com/adafruit/Adafruit_LSM6DS/blob/master/Adafruit_LSM6DSO32.cpp).
    - IMU is no longer self-calibrating
- Switched from BMP388 to BMP390
- TODO: improve timekeeping (switch to uint64_t, calibration procedure, etc.)
- implemented way to receive commands for start and stop
- added status bitmap to the status packet, bits set to 1 when that component is good, 0 otherwise
- TODO: detect and log when radio disconnects





