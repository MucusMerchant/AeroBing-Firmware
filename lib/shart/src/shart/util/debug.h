// Macros on macros on macros
// preprocessor handles everything, Teensy doesn't have to worry about it
// each #ifdef block corresponds to a single debug mode, specified in debug.conf
// TODO: nice print timestamp function

#ifndef SHART_DEBUG_H
#define SHART_DEBUG_H

// maybe try adding build flags -Isrc/util to platformio.ini, but this makes the library non-portable
#include "../../../debug.config" // this looks ugly, but it lets me keep debug.conf in the main directory

// General error logger, TODO: add all errors to code
#ifdef DEBUG_MODE_ERROR
  #define ERROR(message) \
    Serial.print("[ERROR] In function '"); \
    Serial.print(__func__); \
    Serial.print("' on line "); \
    Serial.print(__LINE__); \
    Serial.print(": '"); \
    Serial.print(message); \
    Serial.println("'");
#else
  #define ERROR(message)
#endif

// change variables later, redundancy with time vars
#ifdef DEBUG_MODE_DATARATE
  #define DATARATE_VARS() \
    int current, prev, num_datapoints;
  #define PRINT_DATARATE() \
    current = micros(); \
    float rate = 1000000.0 / (current - prev); \
    prev = current; \
    num_datapoints++; \
    if (num_datapoints % 1000 == 0) { \
      Serial.print("[RATE] "); \
      Serial.print(rate); \
      Serial.print("Hz -----> "); \
      Serial.print(num_datapoints); \
      Serial.print(" datapoints\n"); }
 
#else
  #define DATARATE_VARS()
  #define PRINT_DATARATE()
#endif

// Print to serial when the status of a sensor changes
#ifdef DEBUG_MODE_STATUS
  #define UPDATE_STATUS(sensor, status) \
    if (sensor != status) { \
        Serial.print("[STATUS] "); \
        Serial.print(#sensor); \
        Serial.print(": "); \
        Serial.print(statusToString(sensor)); \
        Serial.print(" -----> "); \
        Serial.print(statusToString(status)); \
        Serial.print("\n"); \
        sensor = status; \
    }
#else
  #define UPDATE_STATUS(sensor, status) \
    sensor = status;
#endif

#endif