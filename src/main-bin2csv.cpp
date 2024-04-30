#include <SD.h>

#define NUM_DATAPOINTS 18
#define CHUNK_SIZE 100 // number of lines to read at a time
#define WRITE_FILE "data.csv"
#define READ_FILE "data.poop"

File data_file;
int pos = 0;
int chunks = 0;

void write_col_names() {
  if (!(data_file = SD.open(WRITE_FILE, FILE_WRITE))) {
    Serial.println("failed to open data.csv");
    return;
  }
  data_file.print("Time (s),,,,Gyroscope X (deg/s),Gyroscope Y (deg/s),Gyroscope Z (deg/s),Accelerometer X (g),Accelerometer Y (g),Accelerometer Z (g),Magnetometer X (uT),Magnetometer Y (uT),Magnetometer Z (uT),Temperature (c),Pressure (Pa),,,,");
  data_file.close();
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.print("SD initialization failed");
    return;
  }

  union {
      float flt[NUM_DATAPOINTS * CHUNK_SIZE];
      char bin[NUM_DATAPOINTS * CHUNK_SIZE * sizeof(float)];
  } bin2flt;

  SD.remove(WRITE_FILE);
  write_col_names();
  if (!(data_file = SD.open(READ_FILE, FILE_READ))) {
    return;
  }

  if (data_file) {
    Serial.println("Reading data from file:");

    while (data_file.seek(pos)) {
      data_file.readBytes(bin2flt.bin, NUM_DATAPOINTS * CHUNK_SIZE * sizeof(float));
      data_file.close();
      
      pos += NUM_DATAPOINTS * CHUNK_SIZE * sizeof(float);
      if (!(data_file = SD.open(WRITE_FILE, FILE_WRITE))) {
        Serial.println("failed to open data.csv");
        return;
      }
      
      for (int i = 0; i < NUM_DATAPOINTS * CHUNK_SIZE; i++) {
        if (i % NUM_DATAPOINTS == 0) data_file.println();
        data_file.print(bin2flt.flt[i]);
        data_file.print(',');
      }

      data_file.close();
      chunks++;
      Serial.print(chunks);
      Serial.print(" chunks written\n");
      if (!(data_file = SD.open(READ_FILE, FILE_READ))) {
        Serial.println("failed to open data.poop");
        return;
      }

    }
    Serial.println("Done");

    data_file.close();
 
  } else {
    Serial.println("Error opening file!");
  }
}

void loop() {

}
