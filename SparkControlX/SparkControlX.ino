// Don't forget Arudino IDE settings:
//   Board:              ESP32 S3 Dev Module
//   PSRAM:              OPI PSRAM
//   Partition Scheme:   Huge App (3MB No OTA / 1MB SPIFFS)

#include "SparkControlX.h"

void setup() {
  Serial.begin(115200);

  // LVGL
  lvgl_setup();
  screen_setup();

  // Spark Control
  SparkControlStart();

  Serial.println("Started");
}

void loop() {
  lvgl_loop();
  SparkControlLoop();
}
