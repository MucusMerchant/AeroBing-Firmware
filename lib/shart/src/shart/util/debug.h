// Macros on macros on macros
// preprocessor handles everything, Teensy doesn't have to worry about it
// each #ifdef block corresponds to a single debug mode, specified in debug.conf
// TODO: nice print timestamp function

#ifndef SHART_DEBUG_H
#define SHART_DEBUG_H

// maybe try adding build flags -Isrc/util to platformio.ini, but this makes the library non-portable
#include "../../../shart.config" // this looks ugly, but it lets me keep debug.conf in the main directory

// General error logger, TODO: add all errors to code
#ifdef DEBUG_MODE_ERROR
  #define ERROR(message, serial_port) \
    serial_port.print("[ERROR] In function '"); \
    serial_port.print(__func__); \
    serial_port.print("' on line "); \
    serial_port.print(__LINE__); \
    serial_port.print(": '"); \
    serial_port.print(message); \
    serial_port.println("'");
#else
  #define ERROR(message, serial_port)
#endif

/* Keeping this here in case I want nicely formatted time in the future
    char buf[25]; \
    int time = sen.data.ms; \
    int16_t ms = time%1000; time/=1000; \
    int16_t s  = time%60;   time/=60; \
    int16_t m  = time%60;   time/=60; \
    sprintf(buf, "%02d:%02d:%02d:%03d", time, m, s, ms); Serial.print(buf); \
*/

// Print to serial when the status of a sensor changes
#ifdef DEBUG_MODE_STATUS
  #define UPDATE_STATUS(sensor, status, serial_port) \
    if (sensor != status) { \
        serial_port.print("[STATUS] "); \
        serial_port.print(#sensor); \
        serial_port.print(": "); \
        serial_port.print(statusToString(sensor)); \
        serial_port.print(" -----> "); \
        serial_port.print(statusToString(status)); \
        serial_port.print("\n"); \
        sensor = status; \
    }
#else
  #define UPDATE_STATUS(sensor, status, serial_port) \
    sensor = status;
#endif

#endif