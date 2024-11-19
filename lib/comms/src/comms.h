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

// macro to update a packet object with the checksum of its payload, braces to avoid redeclaration of temporary variables
#define CHECKSUM(p) \
    {                                                                  \
        unsigned char *payload = (unsigned char *) &p;                 \
        memset(payload + 2, 0, 2);                                     \
        for (unsigned int i = HEADER_LENGTH; i < sizeof(p); i++) {     \
            p.c_a += payload[i];                                       \
            p.c_b += p.c_a;                                            \
        }                                                              \
    }

typedef unsigned char packet_t;

// Header common to all packet types
// only one sync byte (0xAA), then a type, defined by the packet_t enum, then two checksum bytes
// note here that packet types are hard baked into the packets themselves, this makes a few things simpler later on
struct packet_base {

    const unsigned char sync = SYNC;
    const packet_t      type;
    unsigned char       c_a; // single-byte addition is actually slower bc of sign-extending, msybe should cast before calculating, may be insignificant though
    unsigned char       c_b;

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
template<typename PacketType, typename SerialType>
bool receivePacketType(PacketType *p, SerialType *serial, bool acknowledge) {
    
    int packet_size = sizeof(PacketType);

    // read header, potentially problematic if unfortunate payload choice, potentially endless blocking
    // gotta make sure last byte of data isnt sync byte
    if (serial->available() < packet_size ||
        serial->read() != SYNC ||
        serial->read() != p->type) return false;

    p->c_a = serial->read();
    p->c_b = serial->read();

    // buffer for the packet's data content (packet minus header)
    unsigned char *buffer = static_cast<unsigned char*>(malloc(packet_size - HEADER_LENGTH));
    unsigned char sum[2] = {0};

    for (int i = 0; i < packet_size - HEADER_LENGTH; i++) {

        unsigned char b = serial->read();
        buffer[i] = b;
        sum[0] += b;
        sum[1] += sum[0];

    }

    if (sum[0] == p->c_a && sum[1] == p->c_b) {

        memcpy(&(p->data), buffer, packet_size - HEADER_LENGTH);
        // send ack signal back to sender
        if (acknowledge) serial->write(reinterpret_cast<unsigned char *>(p), packet_size);
        free(buffer);
        return true;

    } 

    free(buffer);
    return false;

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