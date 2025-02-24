// this file contains all of the necessary typedefs and packets for ALL flight computer communication. this includes both outgoing and incoming packets
#ifndef COMMS_H
#define COMMS_H

#define HEADER_LENGTH    4
#define SYNC             0xAA

// packet type bytes
#define TYPE_SENSOR      0x0B
#define TYPE_GPS         0xCA
#define TYPE_COMMAND     0xA5
#define TYPE_POOP        0x33

// commands for command_p
#define START_COMMAND    0x6D656F77 // DANGER, DO NOT CONVERT THIS TO ASCII!!! YOU WILL REGRET
#define STOP_COMMAND     0x6D696175 // or this one!!!

const uint16_t crc16_lookup_table[256] = { 
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
        0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
        0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
        0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
        0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
        0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
        0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
        0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
        0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

//switch to crc 16 ccitt false
#define CHECKSUM(p) \
    { \
        unsigned char *payload = (unsigned char *) &p;                 \
        memset(payload + 2, 0xFF, 2); \
        uint16_t crc = 0xFFFF;\
        for (unsigned int i = HEADER_LENGTH; i < sizeof(p); i++) {  \
            crc = (crc << 8) ^ crc16_lookup_table[(crc >> 8) ^ payload[i]];\
            crc &= 0xFFFF;\
        }\
        p.crc_16_ccitt_false = crc;\
    }

typedef unsigned char packet_t;

// Header common to all packet types
// only one sync byte (0xAA), then a type, defined by the packet_t enum, then two checksum bytes
// note here that packet types are hard baked into the packets themselves, this makes a few things simpler later on
struct packet_base {

    const unsigned char sync = SYNC;
    const packet_t      type;
    uint16_t            crc_16_ccitt_false;

    packet_base(packet_t t) : type(t) {}

};

// Sensor packet includes IMU data, altimeter data
// if you make changes to this struct, they should respect packed alignment
// if packing is impossible, make sure to explicitly specify the padding in the struct to its straightfoward to interpret on the Python end
struct sensor_p : public packet_base {

    struct {
        uint32_t      us;   
        int16_t       acc_x;
        int16_t       acc_y;
        int16_t       acc_z;
        int16_t       gyr_x;
        int16_t       gyr_y;
        int16_t       gyr_z;
        float         mag_x;
        float         mag_y;
        float         mag_z;
        float         temp;
        float         pres;
        int16_t       adxl_acc_x;
        int16_t       adxl_acc_y;
        int16_t       adxl_acc_z;
        unsigned char status;
        unsigned char reserved;
    } data;

    sensor_p() : packet_base(TYPE_SENSOR), data{} {}

};

struct gps_p : public packet_base {

    struct {
        uint32_t      us;
        int32_t       lat;
        int32_t       lon;
        int32_t       alt;
        int32_t       veln;
        int32_t       vele;
        int32_t       veld;
        uint32_t      eph;
        uint32_t      epv;
        uint32_t      sacc;
        int32_t       gspeed;
        float         pdop;
        unsigned char nsats;
        unsigned char fix_type;
        unsigned char valid;
        unsigned char flags;
        // blah blah
    } data;

    gps_p() : packet_base(TYPE_GPS), data{} {}

};

struct command_p : public packet_base {
    
    struct {
        int32_t command;
    } data;

    command_p() : packet_base(TYPE_COMMAND), data{} {}
};

// this assumes the packet passed in is initialized with correct type, i.e. correct size
// templated to use any packet type an either usb or harware serial
// TURN THIS INTO MACRO, ALSO CONSIDER PACKET POINTER TYPE OOPSIE

#define RECEIVE_PACKET(p, serial, result) \
{\
    int packet_size = sizeof(p); \
    if (serial.available() < packet_size || \
        serial.read() != SYNC || \
        serial.read() != p.type) result = false; \
    else { \
        uint16_t received_crc = (serial.read() & 0xFF) | (serial.read() << 8); \
        unsigned char *buffer = reinterpret_cast<unsigned char *>(&p); \
        for (int i = HEADER_LENGTH; i < packet_size; i++) buffer[i] = serial.read(); \
        CHECKSUM(p) \
        if (p.crc_16_ccitt_false == received_crc) { \
            serial.write(reinterpret_cast<unsigned char *>(&p), packet_size); \
            result = true; \
        } else { \
            result = false; \
        } \
    } \
}

// struct gps_message {
// 	uint64_t time_usec{0};
// 	int32_t lat;		///< Latitude in 1E-7 degrees
// 	int32_t lon;		///< Longitude in 1E-7 degrees
// 	int32_t alt;		///< Altitude in 1E-3 meters (millimeters) above MSL
// 	float yaw;		///< yaw angle. NaN if not set (used for dual antenna GPS), (rad, [-PI, PI])
// 	float yaw_offset;	///< Heading/Yaw offset for dual antenna GPS - refer to description for GPS_YAW_OFFSET
// 	uint8_t fix_type;	///< 0-1: no fix, 2: 2D fix, 3: 3D fix, 4: RTCM code differential, 5: Real-Time Kinematic
// 	float eph;		///< GPS horizontal position accuracy in m
// 	float epv;		///< GPS vertical position accuracy in m
// 	float sacc;		///< GPS speed accuracy in m/s
// 	float vel_m_s;		///< GPS ground speed (m/sec)
// 	Vector3f vel_ned;	///< GPS ground speed NED
// 	bool vel_ned_valid;	///< GPS ground speed is valid
// 	uint8_t nsats;		///< number of satellites used
// 	float pdop;		///< position dilution of precision
// };


#endif