#ifndef SparkControlX_h
#define SparkControlX_h

extern QueueHandle_t qFromAmp;

struct packet_data {
  uint8_t *ptr;
  int size;
};


#define FIRMWARE_NAME "F4.1.19"
#define MESSAGE_SIZE 50
#define PROFILE_NAME_LENGTH 21

// From SparkControl.cpp
void SparkControlStart();
void SparkControlLoop();
void send_button_info(int my_btn_num);
void send_slider_info(uint16_t slider1_val, uint16_t slider2_val);

// from lvgl.cpp
void lvgl_setup();
void lvgl_loop();

// from SparkControlScreen.cpp
void screen_setup();
void change_colour(int led_num, int red, int green, int blue);
void show_tone_bank(int bank);
void show_connected();
void show_disconnected();

// SparkComms
void send_spark_x_data(uint8_t *buf, int len);
void send_spark_control_data(uint8_t *buf, int len);
void spark_comms_start();
int get_message(uint8_t *buf);

#endif