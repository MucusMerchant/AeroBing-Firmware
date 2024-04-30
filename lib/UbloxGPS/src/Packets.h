#ifndef PACKETS_H
#define PACKETS_H

struct commonHeader {
  unsigned char headerClass;
  unsigned char headerId;
  unsigned short headerLength;
};

struct NavPvtPacket {
  // note that 'long' is typically 4 bytes. on a machine where it is 8, this code breaks
  commonHeader    header;
  unsigned long   iTOW;       //  ms    GPS time of week of the navigation epoch. See the description of iTOW for details.
  unsigned short  year;       //  y     Year (UTC)
  unsigned char   month;      //  month Month, range 1..12 (UTC)
  unsigned char   day;        //  d     Day of month, range 1..31 (UTC)
  unsigned char   hour;       //  h     Hour of day, range 0..23 (UTC)
  unsigned char   min;        //  min   Minute of hour, range 0..59 (UTC)
  unsigned char   sec;        //  s     Seconds of minute, range 0..60 (UTC)
  char            valid;      //        Validity Flags (see graphic below)
  unsigned long   tAcc;       //  ns    Time accuracy estimate (UTC)
  long            nano;       //  ns    Fraction of second, range -1e9 .. 1e9 (UTC)
  unsigned char   fixType;    //        GNSSfix Type, range 0..5, 0x00 = No Fix, 0x01 = Dead Reckoning only, 0x02 = 2D-Fix, 0x03 = 3D-Fix, 0x04 = GNSS + dead reckoning combined, 0x05 = Time only fix, 0x06..0xff: reserved,
  char            flags;      //        Fix Status Flags (see graphic below)
  unsigned char   reserved1;  //        Reserved
  unsigned char   numSV;      //        Number of satellites used in Nav Solution
  long            lon;        //  deg   Longitude (1e-7)
  long            lat;        //  deg   Latitude (1e-7)
  long            alt;        //  mm    Height above Ellipsoid
  long            hMSL;       //  mm    Height above mean sea level
  unsigned long   hAcc;       //  mm    Horizontal Accuracy Estimate
  unsigned long   vAcc;       //  mm    Vertical Accuracy Estimate
  long            velN;       //  mm/s  NED north velocity
  long            velE;       //  mm/s  NED east velocity
  long            velD;       //  mm/s  NED down velocity
  long            gSpeed;     //  mm/s  Ground Speed (2-D)
  long            heading;    //  deg   Heading of motion 2-D (1e-5)
  unsigned long   sAcc;       //  mm/s  Speed Accuracy Estimate
  unsigned long   headingAcc; //  deg   Heading Accuracy Estimate (1e-5)
  unsigned short  pDOP;       //        Position DOP (0.01)
  short           reserved2;  //        Reserved
  unsigned long   reserved3;  //        Reserved

};
// write Packet unions like ^ to enable other UBX message types.
/*
  // Type       Name        Unit  Description (Scaling)
  unsigned long iTOW;   //  ms    GPS time of week of the navigation epoch. See the description of iTOW for details.
  long          ecefX;  //  cm    ECEF X coordinate
  long          ecefY;  //  cm    ECEF Y coordinate
  long          ecefZ;  //  cm    ECEF Z coordinate
  unsigned long pAcc;   //  cm    Position Accuracy Estimate

  UbxGpsNavPosecef(T &serial) : UbxGps<T>(serial)
  {
    this->setLength(20);
  }
};

*/
/*
  // Type       Name        Unit  Description (Scaling)
  unsigned long iTOW;   //  ms    GPS time of week of the navigation epoch. See the description of iTOW for details.
  long          lon;    //  deg   Longitude (1e-7)
  long          lat;    //  deg   Latitude (1e-7)
  long          height; //  mm    Height above ellipsoid
  long          hMSL;   //  mm    Height above mean sea level
  unsigned long hAcc;   //  mm    Horizontal accuracy estimate
  unsigned long vAcc;   //  mm    Vertical accuracy estimate

  UbxGpsNavPosllh(T &serial) : UbxGps<T>(serial)
  {
    this->setLength(28);
  }
};
*/
/*
  // Type         Name            Unit  Description (Scaling)
  unsigned long   iTOW;       //  ms    GPS time of week of the navigation epoch. See the description of iTOW for
                              //        details.
  unsigned short  year;       //  y     Year (UTC)
  unsigned char   month;      //  month Month, range 1..12 (UTC)
  unsigned char   day;        //  d     Day of month, range 1..31 (UTC)
  unsigned char   hour;       //  h     Hour of day, range 0..23 (UTC)
  unsigned char   min;        //  min   Minute of hour, range 0..59 (UTC)
  unsigned char   sec;        //  s     Seconds of minute, range 0..60 (UTC)
  char            valid;      //        Validity flags (see graphic below)
  unsigned long   tAcc;       //  ns    Time accuracy estimate (UTC)
  long            nano;       //  ns    Fraction of second, range -1e9 .. 1e9 (UTC)
  unsigned char   fixType;    //        GNSSfix Type:
                              //        0: no fix
                              //        1: dead reckoning only
                              //        2: 2D-fix
                              //        3: 3D-fix
                              //        4: GNSS + dead reckoning combined
                              //        5: time only fix
  char            flags;      //        Fix status flags (see graphic below)
  char            flags2;     //        Additional flags (see graphic below)
  unsigned char   numSV;      //        Number of satellites used in Nav Solution
  long            lon;        //  deg   Longitude (1e-7)
  long            lat;        //  deg   Latitude (1e-7)
  long            height;     //  mm    Height above ellipsoid
  long            hMSL;       //  mm    Height above mean sea level
  unsigned long   hAcc;       //  mm    Horizontal accuracy estimate
  unsigned long   vAcc;       //  mm    Vertical accuracy estimate
  long            velN;       //  mm/s  NED north velocity
  long            velE;       //  mm/s  NED east velocity
  long            velD;       //  mm/s  NED down velocity
  long            gSpeed;     //  mm/s  Ground Speed (2-D)
  long            headMot;    //  deg   Heading of motion (2-D) (1e-5)
  unsigned long   sAcc;       //  mm/s  Speed Accuracy Estimate
  unsigned long   headAcc;    //  deg   Heading accuracy estimate (both motion and vehicle) (1e-5)
  unsigned short  pDOP;       //        Position DOP (0.01)
  short           flags3;     //        Additional flags (see graphic below)
  unsigned char   reserved1;  //        Reserved
  long            headVeh;    //  deg   Heading of vehicle (2-D), this is only valid when headVehValid is set,
                              //        otherwise the output is set to the heading of motion (1e-5)
  short           magDec;     //  deg   Magnetic declination. Only supported in ADR 4.10 and later. (1e-2)
  unsigned short  magAcc;     //  deg   Magnetic declination accuracy. Only supported in ADR 4.10 and later. (1e-2)

  UbxGpsNavPvt8(T &serial) : UbxGps<T>(serial)
  {
    this->setLength(92);
  }
};
*/
/*
  // Type         Name            Unit  Description (Scaling)
  unsigned long   iTOW;       //  ms    GPS time of week of the navigation epoch. See the description of iTOW for
                              //        details.
  long            fTOW;       //  ns    Fractional part of iTOW (range: +/-500000). The precise GPS time of week in
                              //        seconds is: (iTOW * 1e-3) + (fTOW * 1e-9)
  short           week;       //  weeks GPS week number of the navigation epoch
  unsigned char   gpsFix;     //        GPSfix Type, range 0..5
                              //        0x00 = No Fix
                              //        0x01 = Dead Reckoning only
                              //        0x02 = 2D-Fix
                              //        0x03 = 3D-Fix
                              //        0x04 = GNSS + dead reckoning combined
                              //        0x05 = Time only fix
                              //        0x06..0xff: reserved
  char            flags;      //        Fix Status Flags (see graphic below)
  long            ecefX;      //  cm    ECEF X coordinate
  long            ecefY;      //  cm    ECEF Y coordinate
  long            ecefZ;      //  cm    ECEF Z coordinate
  unsigned long   pAcc;       //  cm    3D Position Accuracy Estimate
  long            ecefVX;     //  cm/s  ECEF X velocity
  long            ecefVY;     //  cm/s  ECEF Y velocity
  long            ecefVZ;     //  cm/s  ECEF Z velocity
  unsigned long   sAcc;       //  cm/s  Speed Accuracy Estimate
  unsigned short  pDOP;       //        Position DOP (0.01)
  unsigned char   reserved1;  //        Reserved
  unsigned char   numSV;      //        Number of SVs used in Nav Solution
  unsigned long   reserved2;  //        Reserved

  UbxGpsNavSol(T &serial) : UbxGps<T>(serial)
  {
    this->setLength(52);
  }
};
*/


#endif