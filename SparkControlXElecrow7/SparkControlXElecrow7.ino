//#define USE_GPIOS
//#define ACTIVE_HIGH

// Spark functions
void SparkControlStart();
void InitialiseGPIO();
void SparkControlLoop();

// lvgl functions
void lvgl_setup();
void lvgl_loop();

// screen for lvgl
void screen_setup();

void setup() {
  Serial.begin(115200);

  // LVGL
  lvgl_setup();
  screen_setup();

  // Spark Control
  SparkControlStart();

  Serial.println("=========================================");
  Serial.println("Started");
}

void loop() {
  lvgl_loop();
  SparkControlLoop();
}
