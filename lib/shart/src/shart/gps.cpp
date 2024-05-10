#include "shart.h"

void Shart::initGTU7() {
  // add configuration code here if it ever works
  gps.begin(GPS_BAUD_RATE);
  while (!Serial2);

}

// Collect data from the GTU7
void Shart::collectDataGTU7() {

  // UBX protocol for GPS data
  gps.update();
  if (gps.isReady()) {
    const NavPvtPacket &packet = gps.getPacket();
    gps_packet.data.lat = packet.lat;
    gps_packet.data.lon = packet.lon;
    gps_packet.data.alt = packet.alt;
    gps_packet.data.veln = packet.velN;
    gps_packet.data.vele = packet.velE;
    gps_packet.data.veln = packet.velD;
    gps_ready = true;
  }

}