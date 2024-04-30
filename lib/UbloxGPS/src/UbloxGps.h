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
 
  void update() {

    unsigned char pos = carriagePosition;
    while (serial.available()) // we read bytes an order of magnitude faster than gps sends, so we won't really ever get stuck in this while loop
    { 
      unsigned char curr_byte = serial.read();
      // Check for sync bytes, this block gets executed the most, must be fast
      if (pos < sizeof(UBXGPS_HEADER)) 
      {
        if (curr_byte != UBXGPS_HEADER[pos]) 
        {
          pos = 0;
          continue;
        }
      } 
      else if (pos < (size + sizeof(UBXGPS_HEADER))) 
      {
        data[pos - sizeof(UBXGPS_HEADER)] = curr_byte;
        checksum[0] += curr_byte;
        checksum[1] += checksum[0];
      } 
      else if (pos == (size + sizeof(UBXGPS_HEADER)) && curr_byte == checksum[0]) 
      { 
      // just carry on!
      } 
      else if (pos == (size + sizeof(UBXGPS_HEADER)+1) && curr_byte == checksum[1]) 
      {
        memcpy((unsigned char *) &packet, data, sizeof(packet));
        memset(checksum, 0, 2);
        ready = true;
        pos = 0;
        break;
      } 
      else 
      {
        pos = 0;
        memset(checksum, 0, 2);
        continue; // skip the increment
      }
      // Move the carriage forward.
      pos++;
    }

    carriagePosition = pos;
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
  
  unsigned char size;
  unsigned char carriagePosition;
  unsigned char checksum[2];
  unsigned char data[sizeof(packet)];

};

#endif
