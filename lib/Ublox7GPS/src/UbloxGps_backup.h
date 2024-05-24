#ifndef UBXGPS_H_INCLUDED
#define UBXGPS_H_INCLUDED

#include <Arduino.h>
#include "Packets.h"

const unsigned char UBXGPS_HEADER[] = {0xB5, 0x62};

template <typename Packet>
class UbloxGps {

public:

  UbloxGps(HardwareSerial &serial) : serial(serial) {
    carriagePosition = 0;
    size = sizeof(packet);
  }

  void begin(unsigned long baudrate) {
    serial.begin(baudrate);
  }
  // failing either checksum sends you to the code after the switch; all other cases skip it, this is the weird break/continue bullshit
// when the packet is full and valid, we execute the default condition, so fucking clean
/*
bool packetValid;                                                                       \
    if (p < 4) {                                                                            \
        sum[p-2] = c;                                                                       \
    } else {                                                                                \
        if (p < (sizeof(packet))) ((unsigned char *) &packet)[p - 2] = c;                         \ 
        p++;                                                                                       \
        switch (p) {                                                                                  \
            case (sizeof(packet)): CHECKSUM(packet, sizeof(packet));                        continue; \
            case (sizeof(packet) + 1): if (c != packet.c_a) { packetValid = false; break; } continue; \
            case (sizeof(packet) + 2): if (c != packet.c_b) { packetValid = false; break; } continue; \
            default: packetValid = true;                                                    \
        }                                                                                   \
        p = 0;                                                                              \
        break;                                                                              \
    }*/
  void update() {
    unsigned char p = carriagePosition;
    while (serial.available()) { // we read bytes an order of magnitude faster than gps sends, so this while is okay for time-sensitive applications
      unsigned char c = serial.read();

      // Carriage is at the first or the second sync byte, should be equals.
      if (p < 2) {
        if (c == UBXGPS_HEADER[p]) {
          p++;
        }
        // Reset if not.
        else {
          p = 0;
        }
      }

      // Sync with header after success.
      else {
        // Put the byte read to a particular address of this object which depends on the carriage position.
        if (p < (size + 2)) {
          // THIS IS WRONG, WE ARE OVERSTEPING BOUNDS OF THE ARRAY, writing memory we shouldnt be writng to
          data[p - 2] = c;
        }

        // Move the carriage forward.
        p++;

        // Carriage is at the first checksum byte, we can calculate our checksum, but not compare, because this byte is not read.
        if (p == (size + 2)) {
          calculateChecksum();
        }
        // Carriage is at the second checksum byte, but only the first byte of checksum read, check if it equals to ours.
        else if (p == (size + 3)) {
          // Reset if not.
          if (c != checksum[0]) {
            p = 0;
          }
        }
        // Carriage is after the second checksum byte, which has been read, check if it equals to ours.
        else if (p == (size + 4)) {
          // Reset the carriage.
          p = 0;

          // The readings are correct and filled the object, return true.
          if (c == checksum[1]) {
            //carriagePosition = p;
            memcpy((unsigned char *) &packet, data, sizeof(packet));
            ready = true;
            break;
            //return;
          }
        }
        // Reset the carriage if it is out of a packet.
        else if (p > (size + 4)) {
          p = 0;
        }
      }
    }

    carriagePosition = p;
    return;
  }

  bool isReady() {
    return ready;
  }

  const Packet &getPacket() {
    ready = false;
    return packet;
  };

private:
  Packet packet;
  HardwareSerial &serial;
  bool ready = false;
  
  // why the fuck does putting data first mess everything up? it overwrites other variables?????
  unsigned char size;
  unsigned char carriagePosition;
  unsigned char checksum[2];
  unsigned char data[sizeof(packet)];

  int available() {
    return serial.available();
  }

  byte read() {
    return serial.read();
  }

  void calculateChecksum() {
    memset(checksum, 0, 2);

    for (int i = 0; i < size; i++) {
      checksum[0] += data[i];
      checksum[1] += checksum[0];
    }
  }

};

#endif
