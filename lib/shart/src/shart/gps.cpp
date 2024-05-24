#include "shart.h"

void Shart::initGTU7() {
  // add configuration code here if it ever works
  gps.begin(GPS_BAUD_RATE);
  while (!Serial2);

}
/*
struct gps_message {
	uint64_t time_usec{0};
	int32_t lat;		///< Latitude in 1E-7 degrees
	int32_t lon;		///< Longitude in 1E-7 degrees
	int32_t alt;		///< Altitude in 1E-3 meters (millimeters) above MSL
	float yaw;		///< yaw angle. NaN if not set (used for dual antenna GPS), (rad, [-PI, PI])
	float yaw_offset;	///< Heading/Yaw offset for dual antenna GPS - refer to description for GPS_YAW_OFFSET
	uint8_t fix_type;	///< 0-1: no fix, 2: 2D fix, 3: 3D fix, 4: RTCM code differential, 5: Real-Time Kinematic
	float eph;		///< GPS horizontal position accuracy in m
	float epv;		///< GPS vertical position accuracy in m
	float sacc;		///< GPS speed accuracy in m/s
	float vel_m_s;		///< GPS ground speed (m/sec)
	Vector3f vel_ned;	///< GPS ground speed NED
	bool vel_ned_valid;	///< GPS ground speed is valid
	uint8_t nsats;		///< number of satellites used
	float pdop;
*/
// Collect data from the GTU7
void Shart::collectDataGTU7() {

  // UBX protocol for GPS data
  gps.update();
  if (gps.isReady()) {
    const NavPvtPacket &packet = gps.getPacket();
    gps_packet.data.lat = packet.lat;
    gps_packet.data.lon = packet.lon;
    gps_packet.data.alt = packet.hMSL;
    gps_packet.data.veln = packet.velN;
    gps_packet.data.vele = packet.velE;
    gps_packet.data.veld = packet.velD;
    gps_packet.data.eph = packet.hAcc;
    gps_packet.data.epv = packet.vAcc;
    gps_packet.data.sacc = packet.sAcc;
    gps_packet.data.gspeed = packet.gSpeed;
    gps_packet.data.pdop = ((float) packet.pDOP) / 100;
    gps_packet.data.nsats = packet.numSV;
    gps_packet.data.fix_type = packet.fixType;
    gps_packet.data.valid = packet.valid;
    gps_packet.data.flags = packet.flags;
    gps_ready = true;
  }

}