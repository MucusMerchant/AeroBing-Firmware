/*******************************************************************************
* File Name: shart.h
*
* Description:
*   A class to gather, store, and transmit telemetry data from rocket sensors.
*   This is a singleton class containing all SHART functionality. 
*
*   The Shart singleton also handles the recovery of lost sensors.
*   When sensors get disconnected from vibrations, high Gs, violent combustion, we
*   want a way to reconnect with them. Calls to lost devices may disrupt or
*   terminate the program, so we also want to avoid collecting data from them.
*   Reconnect() calls slow initializers, and is never meant to be run
*   on the main thread. We use the TeensyThreads.h library to avoid long pauses
*   (i.e. lost data) during reconnection.
*
*   Todo:
*   Theory: One of the most destructive stages of rocket flight is landing. If 
*   something goes wrong (and something will), landing is when the rocket 
*   suffers the most. Therefore, we must somehow transmit the data collected 
*   during radio disconnection (which includes apogee!) before we land. That 
*   is where knowledge of the current state of the radio connection is vital. 
*   Once we reconnect, we must transmit essential flight data, like apogee, 
*   immediately.
*
*   A small inegrated memory chip might be useful. When radio is disconnected,
*   write to memory, transmit as soon as radio connects
*
* Author: Andrew Shen-Costello
* Date: Spring, 2024
* Organization: AeroBing!
*
* Version:  1.0.0
*
*******************************************************************************/

#ifndef SHART_H
#define SHART_H
 
#include "shart/util/status_enums.h"
#include "shart/util/debug.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Preprocessor directives for SENSOR and GPS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include <Adafruit_BMP3XX.h>
#include <Adafruit_ADXL375.h>
#include <Teensy-ICM-20948.h>
#include <Adafruit_LSM6DSO32.h>
#include <UbloxGPS.h>
#include <UbxGpsConfig.h>

// GPS pins, not that these are RX and TX on the microcontroller, NOT the GTU7 (i.e. GTU_RX_PIN goes to the TX pin on the GTU)
#define GPS_SERIAL_PORT Serial2
#define GPS_BAUD_RATE   9600

// SPI bus for BMP388, default SPI bus (shared)
#define BMP_SPI_BUS  SPI1
#define ADXL_SPI_BUS SPI
#define ICM_SPI_BUS  SPI1
#define LSM_I2C_BUS  Wire

// SPI chip select pins
#define BMP_CS  0 // CS
#define ADXL_CS 10
#define ICM_CS  38
#define LSM_I2C_ADDR 106U

// LED pins (not implemented)
#define ONBOARD_LED_PIN 13
#define OK_LED_PIN 23
#define BMP_LED_PIN 22
#define JY_LED_PIN 21
#define SD_LED_PIN 20

// Chip IDs for checking connectivity
#define BNO_CHIP_ID  0xA0
#define BMP_CHIP_ID  0x60
#define ADXL_CHIP_ID 0xE5
#define LSM_CHIP_ID  0x6C

// Define bit offsets for status bitmap
#define ICM_STATUS_OFFSET  0
#define BMP_STATUS_OFFSET  1
#define ADXL_STATUS_OFFSET 2
#define LSM_STATUS_OFFSET  3
#define SD_STATUS_OFFSET   4
#define PYRO_STATUS_OFFSET 5

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Preprocessor directoves for EXPORT
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//#include <SD.h> 
#include "RingBuf.h"
#include "SdFat.h"

// Definitions for radio
#define RADIO_SERIAL_PORT  Serial8 // RX6 and TX6 on the teensy, see pinout for pin numbers
#define RADIO_BAUD_RATE    115200//230400 // note that this has an impact on transmission speed
#define RADIO_TIMEOUT_MS   1000 // Radio read timeout in milliseconds
#define RADIO_SEND_EVERY_N 4

// Definitions for SD
#define SD_CONFIG                      SdioConfig(FIFO_SDIO) // Use Teensy SDIO
#define LOG_INTERVAL_USEC              40 // Interval between points for 25 ksps.
#define LOG_FILE_SIZE                  2147483648//536870912  // 512MB allocated before logging to save time, maybe increase on launch day
#define RING_BUF_CAPACITY              200 * 512//16384 //(400 * 512)
#define SD_MAX_NUM_CONNECTION_ATTEMPTS 1
#define LOG_FILENAME                   "data"

// Communications library
#include <comms.h>

// USB serial baud rate
#define USB_SERIAL_BAUD_RATE 9600
#define USB_SERIAL_PORT Serial

// SERIAL_PORT is the port we use for all external serial communication. It is the radio serial port
// by default, but if we specify USB_SERIAL_MODE in debug.config, we will use USB serial instead.
#ifdef USB_SERIAL_MODE
  #define MAIN_SERIAL_PORT USB_SERIAL_PORT
#else
  #define MAIN_SERIAL_PORT RADIO_SERIAL_PORT
#endif

class Shart {
  public:
    Shart();
    void init();
    void collect(); // collect data 
    void send(); // save to SD card and transmit through radio to ground station
    void reconnect(); // non-threaded reconnects are here, for chips that share a bus
    void threadedReconnect(); // this must be thread-safe, as it will not be running on the main thread
    bool getSystemStatus(); // return true if system ok
    void maybeFinish(); // check if ground station has asked us to stop shart

  private:
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // PRIVATE SENSOR MEMBERS
    // initializers
    void initICM20948();
    void initLSM6DSO32();
    void initBMP388();
    void initADXL375();
    void initGTU7();

    // individual sensor collectors
    void collectDataICM20948();
    void collectDataLSM6DSO32();
    void collectDataBMP388();
    void collectDataADXL375();
    void collectDataGTU7();
    void collectTime();
    
    // These perform simple checks on the sensors to tell if they are connected
    // If a sensor is not connected, we do not want to try to collect data from it.
    void updateStatusICM20948();
    void updateStatusBMP388();
    void updateStatusADXL375();
    void updateStatusLSM6DSO32();
    void setStatusByte();

    // Sensor objects from respective libraries
    UbloxGps<NavPvtPacket> gps  = UbloxGps<NavPvtPacket>(GPS_SERIAL_PORT);
    Adafruit_BMP3XX        bmp  = Adafruit_BMP3XX();
    Adafruit_ADXL375       adxl = Adafruit_ADXL375(ADXL_CS, &ADXL_SPI_BUS);
    TeensyICM20948         icm  = TeensyICM20948(ICM_CS, &ICM_SPI_BUS);
    Adafruit_LSM6DSO32     lsm  = Adafruit_LSM6DSO32();

    // Component statuses, note: We only care about components that need to be initialized! 
    Status BMPStatus  = UNINITIALIZED;
    Status ICMStatus  = UNINITIALIZED;
    Status ADXLStatus = UNINITIALIZED;
    Status LSMStatus  = UNINITIALIZED;

    uint32_t chipTimeOffset;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // PRIVATE EXPORT MEMBERS
    // initializers
    void initRadio();
    void initSD();

    // data functions, take byte arrays as arguments
    void saveData();
    void transmitData();

    SdFs sd;
    FsFile file;
    RingBuf<FsFile, RING_BUF_CAPACITY> rb;
    
    //File data_file; // The data file on the SD card
    uint16_t sd_num_connection_attempts = 0;
    uint8_t bigassbuffer[1024]; // buffer for radio TX

    Status SDStatus = UNINITIALIZED;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // flag to tell us when to send gps data
    bool gps_ready = false;

    // Persistent packet objects used to store and transmit data
    sensor_p  sensor_packet;
    gps_p     gps_packet;
    command_p command_packet;

    // The current and previous times as recorded by a 'micros()' call
    uint32_t current_time = 0;
    uint32_t sensor_packet_counter = 0;
    uint8_t sd_file_opened = 0;

    // other initializers
    void awaitStart();
    void initPins();
    void initSerial();
    
};

#endif