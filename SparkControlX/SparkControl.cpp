#include <Arduino.h>
#include "SparkControlX.h"

uint8_t *resp;
int resp_size;

int led_num, red, green, blue;
bool got_lights_message = false;

int tone_bank = 1;

uint8_t resp1[]= {0x0b, 0x00, 0x00, 0x00, 0x00, 0x46, 0x34, 0x2e, 0x31, 0x2e, 0x31, 0x39, 0x00};  // firmware F4.1.19
uint8_t resp2[]= {0x0d, 0x00, 0x00, 0x00, 0x03}; // both expression pedals active
uint8_t resp3[]= {0x08, 0x00, 0x00, 0x00, 0x01}; // start with bank 1 active

uint8_t banks[8][13]={{0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x02, 0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x04, 0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x05, 0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x06, 0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75},
                      {0x14, 0x00, 0x00, 0x00, 0x07, 0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},                    
                      {0x14, 0x00, 0x00, 0x00, 0x08, 0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75}
                     };


int sparkx_switch = -1, last_sparkx_switch = -1;
int sparkcontrol_switch = 0, last_sparkcontrol_switch = -1;
uint8_t swx_dat[]   =  {0x03, 0x00, 0x00, 0x00, 0x00};
uint8_t sw_dat[]    =  {0x00};
uint8_t slider_dat[] = {0x0c, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x02, 0xff, 0xff};

#define NUM_BUTTONS 8
int spark_control_map[NUM_BUTTONS]  {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00};


// triggered by the UI when a slider is moved
void send_slider_info(uint16_t slider1_val, uint16_t slider2_val)
{
  slider_dat[5] = slider1_val & 0xff;
  slider_dat[6] = slider1_val >> 8;
  slider_dat[8] = slider2_val & 0xff;
  slider_dat[9] = slider2_val >> 8;
  send_spark_x_data(slider_dat, sizeof(slider_dat));
}

// triggered by the UI when a button is pressed
void send_button_info(int my_btn_num)
{
  // handle Spark X
  int val;
  
  if (my_btn_num == 6)
    val = 0xfd;
  else if (my_btn_num == 7)
    val = 0xfe;
  else
    val = banks[tone_bank - 1][my_btn_num + 5];

  swx_dat[4] = val;
  send_spark_x_data(swx_dat, sizeof(swx_dat));

  Serial.print("Spark Conrtol X: ");
  Serial.println(val, HEX);
  // swap banks if needed
  if (val == 0xfd) {
    if (tone_bank < 8) tone_bank++;
    Serial.print("Bank changed to: ");
    Serial.println(tone_bank);
    show_tone_bank(tone_bank);
  }
  else if (val == 0xfe) {
    if (tone_bank >1 ) tone_bank--;
    Serial.print("Bank changed to: ");
    Serial.println(tone_bank);
    show_tone_bank(tone_bank);
  };

  // handle Spark Control
  sw_dat[0] = spark_control_map[my_btn_num];
  send_spark_control_data(sw_dat, 1);

  Serial.print("Spark Conrtol  : ");
  Serial.println(sw_dat[0], HEX);
};

void clear_message(uint8_t *buf, int buf_len) 
{
  for (int i = 0; i < buf_len; i++)
    buf[i] = 0;

}

void process_message(uint8_t *message, int len) {
  uint8_t response[MESSAGE_SIZE];
  uint8_t cmd = message[0];

  if (cmd == 0x0b) {
    // 0x0b, 0x00, 0x00, 0x00, 0x00, 0x46, 0x34, 0x2e, 0x31, 0x2e, 0x31, 0x39, 0x00   firmware F4.1.19
    /*
    clear_message(response, MESSAGE_SIZE);
    response[0] = 0x0b;
    memcpy(&response[5], (uint8_t *) FIRMWARE_NAME, strlen(FIRMWARE_NAME));
    send_spark_x_data(response, 6 + strlen(FIRMWARE_NAME)); 
    */

    resp = resp1;
    resp_size = sizeof(resp1);
    send_spark_x_data(resp, resp_size);  
  }
    
  if (cmd == 0x0d) {
    resp = resp2;
    resp_size = sizeof(resp2);
    send_spark_x_data(resp, resp_size);  
  }
  if (cmd == 0x08) {
    resp = resp3;
    resp_size = sizeof(resp3);
    resp[4] = tone_bank;
    send_spark_x_data(resp, resp_size);  
  }
  if (cmd == 0x14) {
    resp = banks[tone_bank - 1];
    resp_size = sizeof(banks[tone_bank - 1]);
    send_spark_x_data(resp, resp_size);  
  }
  if (cmd == 0x01) {
    led_num = message[5];
    blue = message[9];
    green = message[10];
    red = message[11];
    if (led_num <= 6)
      change_colour(led_num, red, green, blue);
  }
}

void SparkControlLoop() 
{
  struct packet_data qe;

  while (uxQueueMessagesWaiting(qFromAmp) > 0) {
    xQueueReceive(qFromAmp, &qe, (TickType_t) 0);

    Serial.print("MESSAGE IN QUEUE: ");
    Serial.println(qe.ptr[0], HEX);
 
    process_message(qe.ptr, qe.size);

    free(qe.ptr);
  }
} 


void  SparkControlStart() 
{
  spark_comms_start();
  Serial.println("Spark Control started");
}