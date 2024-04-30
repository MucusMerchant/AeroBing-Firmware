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
    int datapoints = 0; \
    int rate = 0; \
    int current, last;
  #define PRINT_DATARATE() \
    datapoints++; \
    rate++; \
    current = millis(); \
    if (current-last >= 1000) { \
        last = current; \
        Serial.print("[RATE] "); \
        Serial.print(rate); \
        Serial.print("Hz -----> "); \
        Serial.print(datapoints); \
        Serial.print(" datapoints\n"); \
        rate = 0; \
    } 
#else
  #define DATARATE_VARS()
  #define PRINT_DATARATE()
#endif
 
/*for (int i = 1; i < n; i++) { \
      sprintf(buf, "  %+12.5f", data[i]); \
      Serial.print(buf); \
    } \*/
// Prints data (human-readable) to Serial
#ifdef DEBUG_MODE_SENSOR_DATA
  #define PRINT_DATA(sen, gp) \
    Serial.print("[SENSOR] "); \
    char buf[25]; \
    int time = sen.data.ms; \
    int16_t ms = time%1000; time/=1000; \
    int16_t s  = time%60;   time/=60; \
    int16_t m  = time%60;   time/=60; \
    sprintf(buf, "%02d:%02d:%02d:%03d", time, m, s, ms); Serial.print(buf); \
    sprintf(buf, "  %+12d", sen.data.acc_x); Serial.print(buf); \
    sprintf(buf, "  %+12d", sen.data.acc_y); Serial.print(buf); \
    sprintf(buf, "  %+12d", sen.data.acc_z); Serial.print(buf); \
    sprintf(buf, "  %+12.5f", sen.data.gyr_x); Serial.print(buf); \
    sprintf(buf, "  %+12.5f", sen.data.gyr_y); Serial.print(buf); \
    sprintf(buf, "  %+12.5f", sen.data.gyr_z); Serial.print(buf); \
    sprintf(buf, "  %+12.5f", sen.data.mag_x); Serial.print(buf); \
    sprintf(buf, "  %+12.5f", sen.data.mag_y); Serial.print(buf); \
    sprintf(buf, "  %+12.5f", sen.data.mag_z); Serial.print(buf); \
    sprintf(buf, "  %+12.5f", sen.data.temp); Serial.print(buf); \
    sprintf(buf, "  %+12.5f", sen.data.pres); Serial.print(buf); \
    Serial.print("\n");
#elif defined(DEBUG_MODE_GPS_DATA)
  #define PRINT_DATA(sen, gp) \
    Serial.print("[GPS] "); \
    char buf[25]; \
    int time = gp.data.ms; \
    int16_t ms = time%1000; time/=1000; \
    int16_t s  = time%60;   time/=60; \
    int16_t m  = time%60;   time/=60; \
    sprintf(buf, "%02d:%02d:%02d:%03d", time, m, s, ms); Serial.print(buf); \
    sprintf(buf, "  %+12d", gp.data.lat); Serial.print(buf); \
    sprintf(buf, "  %+12d", gp.data.lon); Serial.print(buf); \
    sprintf(buf, "  %+12d", gp.data.alt); Serial.print(buf); \
    sprintf(buf, "  %+12d", gp.data.veln); Serial.print(buf); \
    sprintf(buf, "  %+12d", gp.data.vele); Serial.print(buf); \
    sprintf(buf, "  %+12d", gp.data.veld); Serial.print(buf); \
    sprintf(buf, "  %+12d", gp.data.fix_type); Serial.print(buf); \
    sprintf(buf, "  %+12d", gp.data.nsats); Serial.print(buf); \
    Serial.print("\n");
#else
  #define PRINT_DATA(sen, gp)
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