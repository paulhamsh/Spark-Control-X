#include <Arduino.h>
#include "SparkControlX.h"

int current_profile = 1;
int last_profile = 1;

bool looper_on;

#define NUM_BUTTONS 8
int spark_control_map[NUM_BUTTONS]  {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00};

uint8_t profiles[11][8]={
  {0xff, 0xff, 0xfd, 0xff, 0xff, 0xfe, 0xff, 0xff},
  {0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},
  {0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75},
  {0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},
  {0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75},
  {0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},
  {0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75},
  {0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75},                    
  {0x10, 0x11, 0x14, 0x12, 0x13, 0x15, 0x72, 0x75},
  {0x31, 0x42, 0x08, 0x43, 0x44, 0x45, 0xff, 0xff},  
//  {0xff, 0x42, 0x0C, 0x44, 0x48, 0x08, 0xff, 0xff},                    
  {0xfc, 0xfb, 0x0c, 0xfa, 0xf9, 0x08, 0xff, 0xff}
};

char profile_names[11][PROFILE_NAME_LENGTH]={
  "Global",
  "Profile #1",
  "Profile #2",
  "Profile #3",
  "Profile #4",
  "Profile #5",
  "Profile #6",
  "Profile #7",
  "Profile #8",
  "Looper #1",
  "Looper #2"
};

uint8_t extra_buttons[11][3]={
  {0xfd, 0xfe, 0xf8},
  {0xfd, 0xfe, 0xf8},
  {0xfd, 0xfe, 0xf8},
  {0xfd, 0xfe, 0xf8},
  {0xfd, 0xfe, 0xf8},
  {0xfd, 0xfe, 0xf8},
  {0xfd, 0xfe, 0xf8},
  {0xfd, 0xfe, 0xf8},
  {0xfd, 0xfe, 0xf8},
  {0x46, 0x47, 0xf8},
  {0x46, 0x47, 0xf8}
};

char *get_button_text(int num) {
  switch (num) {
    case 0x00:
      return "Preset\n1";
    case 0x01:
      return "Preset\n2";   
    case 0x02:
      return "Preset\n3";
    case 0x03:
      return "Preset\n4";
    case 0x08:
      return "Next\nPreset";
    case 0x0c:
      return "Change\nBank";  
    case 0x10:
      return "Toggle\nGate";
    case 0x11:
      return "Toggle\nComp/\nWah";   
    case 0x12:
      return "Toggle\nDrive";
    case 0x13:
      return "Toggle\nMod/EQ";
    case 0x14:
      return "Toggle\nDelay";
    case 0x15:
      return "Toggle\nReverb";    
    case 0x31:
      return "Tap\nTempo";    
    case 0x42:
      return "Drum\nMute/\nUnmute";   
    case 0x43:
      return "Record\nDub";
    case 0x44:
      return "Play\nStop";
    case 0x45:
      return "Undo\nRedo";
    case 0x46:
      return "Auto\nMode";    
    case 0x47:
      return "Clear";   
    case 0xfd:
      return "Profile\nUp";    
    case 0xfe:
      return "Profile\nDown";    
    default:
      return "";  
  }
}

// triggered by the UI when a slider is moved
void send_slider_info(uint16_t slider1_val, uint16_t slider2_val)
{
  uint8_t slider_dat[] = {0x0c, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x02, 0xff, 0xff};

  slider_dat[0] = 0x0c;
  slider_dat[4] = 0x01;
  slider_dat[7] = 0x02;
  slider_dat[5] = slider1_val & 0xff;
  slider_dat[6] = slider1_val >> 8;
  slider_dat[8] = slider2_val & 0xff;
  slider_dat[9] = slider2_val >> 8;
  send_spark_x_data(slider_dat, sizeof(slider_dat));
}

void set_main_buttons_text(int profile) {
  for (int i = 0; i <= 5; i++)
    set_button_text(i, get_button_text(profiles[profile][i]));
}


void set_extra_buttons_text(int profile) {
  for (int i = 0; i <= 1; i++)
    set_button_text(i + 6, get_button_text(extra_buttons[profile][i]));
  set_button_text(8, "Looper");  
}



// triggered by the UI when a button is pressed
void send_button_info(int my_btn_num)
{
  uint8_t swx_dat[]   =  {0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t sw_dat[]    =  {0x00};

  // handle Spark X
  int val;
  
  /*
  if (my_btn_num == 6) {
    if (current_profile < 8 && current_profile >= 1) {
      current_profile++;
      Serial.print("Bank changed to: ");
      Serial.println(current_profile);
      show_tone_bank(current_profile, profile_names[current_profile]);
      set_main_buttons_text(current_profile);
      set_extra_buttons_text(current_profile);

      swx_dat[4] = 0xfd;
      send_spark_x_data(swx_dat, 5);
    }
  }
  else if (my_btn_num == 7) {
    if (current_profile > 1 && current_profile <= 8) {
      current_profile--;
      Serial.print("Bank changed to: ");
      Serial.println(current_profile);
      show_tone_bank(current_profile, profile_names[current_profile]);
      set_main_buttons_text(current_profile);
      set_extra_buttons_text(current_profile);

      swx_dat[4] = 0xfe;
      send_spark_x_data(swx_dat, 5);
    }
  }
  else if (my_btn_num == 8) {
    // toggle looper
    if (looper_on) {
      current_profile = last_profile;
      looper_on = false;
      show_tone_bank(current_profile, profile_names[current_profile]);
      set_main_buttons_text(current_profile);
      set_extra_buttons_text(current_profile);
    }
    else {
      last_profile = current_profile;
      current_profile = 9; // special looper profile
      looper_on = true;
      show_tone_bank(current_profile, "Looper");
      set_main_buttons_text(current_profile);
      set_extra_buttons_text(current_profile);
    }
    swx_dat[4] = 0xf8;
    swx_dat[5] = 0x09;
    send_spark_x_data(swx_dat, 6);

    Serial.println("Toggle Looper mode");
  }
  else {   // is a normal message
    val = profiles[current_profile][my_btn_num];

    // update Spark Control X
    swx_dat[4] = val;
    send_spark_x_data(swx_dat, 5);

    Serial.print("Spark Cotrol X: ");
    print_hex(val);
    Serial.println();

    // update Spark Control
    sw_dat[0] = spark_control_map[my_btn_num];
    send_spark_control_data(sw_dat, 1);

    Serial.print("Spark Conrtol  : ");
    print_hex(sw_dat[0]);
    Serial.println();
  }
  */

  if (my_btn_num >= 6)
    val = extra_buttons[current_profile][my_btn_num - 6];
  else 
    val = profiles[current_profile][my_btn_num];  

  // update in Spark Control X format
  if (val == 0xf8) {

    swx_dat[4] = 0xf8;
    swx_dat[5] = 0x09;    
    send_spark_x_data(swx_dat, 6);

    Serial.println("Spark Cotrol X: f8 09");
  }
  else {
    swx_dat[4] = val;
    send_spark_x_data(swx_dat, 5);

    Serial.print("Spark Cotrol X: ");
    print_hex(val);
    Serial.println();
  }

  // update in Spark Control format
  if (my_btn_num <= 5) {
    sw_dat[0] = spark_control_map[my_btn_num];
    send_spark_control_data(sw_dat, 1);

    Serial.print("Spark Conrtol  : ");
    print_hex(sw_dat[0]);
    Serial.println();
  }


  // handle special cases
  if (val == 0xfd) {
    // profile up
    if (current_profile < 8 && current_profile >= 1) {
      current_profile++;
      Serial.print("Bank changed to: ");
      Serial.println(current_profile);
      show_tone_bank(current_profile, profile_names[current_profile]);
      set_main_buttons_text(current_profile);
      set_extra_buttons_text(current_profile);
    }
  }
  else if (val == 0xfe) {
    // profile down
    if (current_profile > 1 && current_profile <= 8) {
      current_profile--;
      Serial.print("Bank changed to: ");
      Serial.println(current_profile);
      show_tone_bank(current_profile, profile_names[current_profile]);
      set_main_buttons_text(current_profile);
      set_extra_buttons_text(current_profile);
    }
  }
  else if (val == 0xf8) {
    // toggle looper
    if (looper_on) {
      current_profile = last_profile;
      looper_on = false;
      show_tone_bank(current_profile, profile_names[current_profile]);
      set_main_buttons_text(current_profile);
      set_extra_buttons_text(current_profile);
    }
    else {
      last_profile = current_profile;
      current_profile = 9; // special looper profile
      looper_on = true;
      show_tone_bank(current_profile, "Looper");
      set_main_buttons_text(current_profile);
      set_extra_buttons_text(current_profile);
    }
    Serial.println("Toggle Looper mode");
  }
}

void clear_message(uint8_t *buf, int buf_len) 
{
  for (int i = 0; i < buf_len; i++)
    buf[i] = 0;
}

void process_message(uint8_t *message, int len) {
  uint8_t response[MESSAGE_SIZE];
  uint8_t cmd = message[0];

  if (cmd == 0x0b) {
    // get firmware string
    // 0x0b, 0x00, 0x00, 0x00, 0x00, 0x46, 0x34, 0x2e, 0x31, 0x2e, 0x31, 0x39, 0x00   firmware F4.1.19

    clear_message(response, MESSAGE_SIZE);
    response[0] = 0x0b;
    memcpy(&response[5], (uint8_t *) FIRMWARE_NAME, strlen(FIRMWARE_NAME));
    send_spark_x_data(response, 6 + strlen(FIRMWARE_NAME)); 
  }
  else if (cmd == 0x0d) {   
    // get expression pedals active
    // 0x0d, 0x00, 0x00, 0x00, 0x03 espression pedals active
    clear_message(response, MESSAGE_SIZE);
    response[0] = 0x0d;
    response[4] = 0x03;
    send_spark_x_data(response, 5); 
  }
  else if (cmd == 0x08) {
    // get current profile
    clear_message(response, MESSAGE_SIZE);
    response[0] = 0x08;
    response[4] = current_profile;
    send_spark_x_data(response, 5); 
  }
  else if (cmd == 0x07) {
    // get current profile
    current_profile = message[4];

    Serial.print("Changed current profile ");
    Serial.println(current_profile);
  }
  else if (cmd == 0x14) {
    // get profile button map
    // 0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x0C, 0x02, 0x03, 0x08, 0x72, 0x75
    int profile_number;
    profile_number = message[4];

    clear_message(response, MESSAGE_SIZE);
    response[0] = 0x14;
    response[4] = profile_number;
    memcpy(&response[5], profiles[current_profile], 8);
    send_spark_x_data(response, 13); 
  }
  else if (cmd == 0x12) {
    // get profile name
    int profile_number, name_len;

    profile_number = message[4];

    clear_message(response, MESSAGE_SIZE);
    response[0] = 0x12;
    response[4] = profile_number;
    name_len = strlen(profile_names[profile_number]);
    memcpy(&response[5], profile_names[profile_number], name_len + 1);
    send_spark_x_data(response, 25); 

    Serial.println(profile_names[profile_number]);
  }
  else if (cmd == 0x01) {
    // set button lamp colours
    int led_num, red, green, blue;

    led_num = message[5];
    blue = message[9];
    green = message[10];
    red = message[11];
    if (led_num <= 6)
      change_colour(led_num, red, green, blue);
  }
  else if (cmd == 0x13) {
    // set profile name
    int profile_number;

    profile_number = message[4];
    strncpy(profile_names[profile_number], (const char *) &message[5], PROFILE_NAME_LENGTH - 1);  // leaves space for the terminating null
    Serial.println(profile_names[profile_number]);
  }
  else if (cmd == 0x03) {
    // set button message
    int profile_number, button_number, message_to_send;

    profile_number = message[4];
    button_number = message[5] - 1;  // use 0 - 6 for indexing the array
    message_to_send = message[10];
    if (button_number <= 5) {
      profiles[profile_number][button_number] = message_to_send;
      set_button_text(button_number, get_button_text(message_to_send));
    }
  }
  else if (cmd == 0x0f) {
    // get battery level
    clear_message(response, MESSAGE_SIZE);
    response[0] = 0x0f;
    response[4] = 0x60;
    send_spark_x_data(response, 5); 
  }
  else if (cmd == 0x15) {
    // unkown 15 00 00 00 01 01
    Serial.print("Got 0x15 - unknown - ");
    print_hex(message[4]);
    print_hex(message[5]);
    Serial.println();
  }
  else if (cmd == 0x11) {
    // unkown 11 00 00 00 
    Serial.println("Got 0x11 - unknown");
    clear_message(response, MESSAGE_SIZE);
    response[0] = 0x11;
    response[4] = 0x00;
    send_spark_x_data(response, 5); 
  }
  else if (cmd == 0x10) {
    // unkown 10 00 00 00 
    Serial.println("Got 0x10 - unknown");
    clear_message(response, MESSAGE_SIZE);
    response[0] = 0x10;
    response[4] = 0x00;
    send_spark_x_data(response, 5); 
  }
  else if (cmd == 0x04) {
    // unkown
    Serial.print("Got 0x04 - unknown - ");
    print_hex(message[4]);
    Serial.println();
  }
  else {
    Serial.print("ERROR: ");
    print_hex(cmd);
    Serial.println("request is unprocessed");
  }
}

void SparkControlLoop() 
{
  struct packet_data qe;

  while (uxQueueMessagesWaiting(qFromAmp) > 0) {
    xQueueReceive(qFromAmp, &qe, (TickType_t) 0);
    process_message(qe.ptr, qe.size);
    free(qe.ptr);   // release the memory block
  }
} 


void  SparkControlStart() 
{
  spark_comms_start();
  Serial.println("Spark Control started");

  for (int i = 0; i <= 8; i++) {
    for (int j = 0; j < PROFILE_NAME_LENGTH; j++ )
      Serial.print(profile_names[i][j]);
    Serial.println();
  }

  looper_on = false;
  
  show_tone_bank(current_profile, profile_names[current_profile]);
  set_main_buttons_text(current_profile);
  set_extra_buttons_text(current_profile);  
  
}