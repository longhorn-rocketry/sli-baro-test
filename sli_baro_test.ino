// #define HEADLESS_TEST
#define TELEMETRY_FNAME "tlm.dat" // Be careful changing this
#define TELEMETRY_DURATION 2 // Seconds
#define TELEMETRY_FREQUENCY 20 // Hz

#include <SD.h>

#include "Adafruit_BMP085.h"

Adafruit_BMP085 g_baro;
unsigned long g_epoch_ms;
float g_lastrec_t;
bool g_setup_success = false;

union FloatBytes {
  float f;
  int i;
};

/**
 * @brief Gets time since system epoch in seconds.
 */
float time() {
  return (millis() - g_epoch_ms) / 1000.0;
}

void setup() {
  // Connect to serial port
  Serial.begin(115200);
  while (!Serial);

  // Connect to BMP085
  Serial.println("Initializing barometer...");
  bool status = g_baro.begin(BMP085_ULTRALOWPOWER);

#ifndef HEADLESS_TEST
  if (!status) {
    Serial.println("FATAL: FAILED TO CONTACT BMP085; ABORTING STARTUP");
    return;
  }
#endif

  // Initialize SD IO
  Serial.println("Initializing SD card...");
  status = SD.begin(BUILTIN_SDCARD);

  if (!status) {
    Serial.println("FATAL: FAILED TO CONTACT SD CARD; ABORTING STARTUP");
    return;
  }

  g_epoch_ms = millis();
  g_lastrec_t = 0;
  g_setup_success = true;

  Serial.println("Startup successful");
}

void loop() {
  if (!g_setup_success)
    return;

  float now = time();

  // Only record for the allotted duration
  if (now > TELEMETRY_DURATION)
    return;

  // Enforce telemetry frequency
  if (now - g_lastrec_t < 1.0 / TELEMETRY_FREQUENCY)
    return;

  // Read telemetry
  float pressure = 9.81; // g_baro.readPressure();
  float temperature = 3.14159; // g_baro.readTemperature();

  // Pack telemetry into buffer. Each telemetry vector <time, pressure, temp>
  // is recorded in 12 contiguous Big Endian bytes.
  static const int BUFFER_LEN = 12;
  char buffer[BUFFER_LEN];
  FloatBytes bytes;

  for (unsigned int i = 0; i < BUFFER_LEN; i += 4) {
    // Unionize the correct value
    switch (i) {
      case 0:
        bytes.f = now;
        break;

      case 4:
        bytes.f = pressure;
        break;

      case 8:
        bytes.f = temperature;
        break;
    }

    // Insert bytes into buffer
    buffer[i]     = (bytes.i >> 24) & 0xFF;
    buffer[i + 1] = (bytes.i >> 16) & 0xFF;
    buffer[i + 2] = (bytes.i >>  8) & 0xFF;
    buffer[i + 3] =  bytes.i        & 0xFF;
  }

  // Dump buffer to file
  File out = SD.open(TELEMETRY_FNAME, FILE_WRITE);
  out.write(buffer, BUFFER_LEN);
  out.close();
  Serial.println("wrote");

  g_lastrec_t = now;
}
