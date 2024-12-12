#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP3XX.h>

#define BMP_CS 0

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BMP3XX bmp;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Adafruit BMP388 / BMP390 test");

  if (! bmp.begin_SPI(BMP_CS, &SPI1)) { 
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    while (1);
  }

  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_200_HZ);
}

void loop() {
  if (! bmp.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  Serial.print("Temperature = ");
  Serial.print(bmp.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bmp.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println();
  delay(2000);
}