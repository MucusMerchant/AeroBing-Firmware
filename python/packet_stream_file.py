# simple python class for reading binary shart packets from SD card file
# this can be a base for other functionality, i.e. storing sensor packets in a csv, graphing, etc.
# to use: put binary shart file in this folder. thats it

import struct # this library is very useful, handles structs for us
import os

# this will work if u got the file in the 'python' folder and your working directory is Aerobing-Firmware
os.chdir(os.getcwd()+"/python")

FILE_NAME = "out.poop"
SYNC_BYTE   = b'\xaa'
TYPE_SENSOR = b'\x0b'
TYPE_GPS    = b'\xca'

# struct specifications following documentation at https://docs.python.org/3/library/struct.html
# note that endian-ness matters
# these structs are defined in comms.h in the Aerobing firmware folder
PACKET_SPEC = {
    TYPE_SENSOR : (56, '<I11f3h2B'), 
    TYPE_GPS    : (52, '<I6i3Iif4B'),
}

class PacketStream:
    def __init__(self, filename):
        self.filename = filename
        self.file = None
        self.error_state = 0

    def begin(self):
        self.file = open(self.filename, mode='rb')

    # Function to calculate the checksum
    def calculate_checksum(self, data):
        checksum_a = 0
        checksum_b = 0
        for byte in data:
            checksum_a += byte
            checksum_b += checksum_a
        return checksum_a & 0xFF, checksum_b & 0xFF

    # Function to process packets, reading bytes from file one at a time
    def read_packet(self, packet_types):
        if (sync_byte := self.file.read(1)):
            if sync_byte == SYNC_BYTE:
                # Found sync byte, read packet type
                packet_type_byte = self.file.read(1)
                if packet_type_byte in packet_types:
                    received_checksum_a, received_checksum_b = struct.unpack('<BB', self.file.read(2))
                    packet_info = packet_types[packet_type_byte]
                    packet_size = packet_info[0]
                    packet_data = self.file.read(packet_size)
                        
                    calculated_checksum_a, calculated_checksum_b = self.calculate_checksum(packet_data)

                    if (received_checksum_a, received_checksum_b) == (calculated_checksum_a, calculated_checksum_b):
                        packet_format = packet_info[1]
                        return packet_type_byte, struct.unpack(packet_format, packet_data)
                    else:
                        print("Checksum failed!")
                        self.error_state = 1
                else:
                    print("Invalid packet type byte:", packet_type_byte)
                    self.error_state = 2
        else:
            self.error_state = 3 # error state 2 if we are at eof
        return None, None
    
# note to Julie: barometer data is stored in the last 2 spots of the sensor tuple (temp in C and pressure in Pa)
if __name__ == "__main__":
    packet_reader = PacketStream(FILE_NAME)
    packet_reader.begin()
    packets = 0
    # here you can filter by error state. right now, we stop only when we reach eof
    while packet_reader.error_state != 3:
        packet_type, packet = packet_reader.read_packet(PACKET_SPEC)
        if packet_type == TYPE_SENSOR:
            print("[SENSOR] " + str(packet))
        elif packet_type == TYPE_GPS:
            print("[GPS] " + str(packet))
        else:
            continue
    
        packets += 1
    print("Done! " + str(packets) + " packets read. meow")

