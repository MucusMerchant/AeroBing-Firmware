### Communication protocol library for AeroBing flight computer

A very minimal communication protocol for data logging, commands, and more. Defines packet types, packet struct layouts, and some utility macros (maybe functions later). 

Every packet begins with the universal sync byte, 0xAA. The next byte specifies the packet type while also serving as a second sync byte. The third and fourth bytes are checksum A and checksum B. Checksum A is simply the sum of every byte in the payload. Checksum B is the sum of the checksum A values at each setep of the calculation. Thus, there are only 4 bytes of overhead with each packet. All of the following bytes belong to the payload (there is no footer). 

Note: the checksum method is borrowed from the Ublox protocol.