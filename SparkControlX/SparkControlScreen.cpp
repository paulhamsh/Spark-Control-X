#include <lvgl.h>
#include <Arduino.h>
#include "SparkControlX.h"

// Set up these fonts in lvgl_conf.h
//
// #define LV_FONT_MONTSERRAT_14 1
// #define LV_FONT_MONTSERRAT_16 1
// #define LV_FONT_MONTSERRAT_18 1
// #define LV_FONT_MONTSERRAT_20 1
// #define LV_FONT_MONTSERRAT_28 1

// LVGL display elements and callbacks


#define NUM_MAIN_BUTTONS 6
#define NUM_EXTRA_BUTTONS 3
#define NUM_BUTTONS (NUM_MAIN_BUTTONS + NUM_EXTRA_BUTTONS)
#define MAIN_BUTTONS_IN_ONE_ROW (NUM_MAIN_BUTTONS / 2)
#define NUM_SLIDERS 2
#define NUM_LEDS 6
#define LEDS_IN_ONE_ROW (NUM_LEDS / 2)

lv_obj_t *btns[NUM_BUTTONS];
lv_obj_t *labels[NUM_BUTTONS];
lv_obj_t *leds[NUM_LEDS];
lv_obj_t *name_labels[NUM_LEDS];
lv_obj_t *sliders[NUM_SLIDERS];
lv_obj_t *profile_label;
lv_obj_t *amp_conn_status;

int numbers[NUM_BUTTONS]{0, 1, 2, 3, 4, 5, 6, 7, 8};
char *name_texts[NUM_LEDS]{"I", "II", "A", "III", "IV", "B"};
int slider_numbers[NUM_SLIDERS]{0, 1};


void change_colour(int led_num, int red, int green, int blue) 
{
  lv_obj_set_style_bg_color(leds[led_num], lv_color_make(red, green, blue), LV_PART_MAIN);
}

void show_tone_bank(int bank, char *name) 
{
  lv_label_set_text_fmt(profile_label, "%d   %s", bank, name);
}

void show_connected() 
{
  lv_obj_set_style_bg_color(amp_conn_status, lv_color_make(0, 0, 192), LV_PART_MAIN);
}

void show_disconnected() 
{
  lv_obj_set_style_bg_color(amp_conn_status, lv_color_make(192, 0, 0), LV_PART_MAIN);
}

void set_button_text(int button_number, char *text) {
  lv_label_set_text(labels[button_number], text);  
}

static void slider_event_cb(lv_event_t *e)
{
  int my_slider_num;
  uint16_t val1, val2, my_val;

  my_slider_num = *(int *) lv_event_get_user_data(e);
  val1 = lv_slider_get_value(sliders[0]);
  val2 = lv_slider_get_value(sliders[1]);
  my_val = my_slider_num == 0 ? val1 : val2;
  Serial.print("Slider ");
  Serial.print(my_slider_num);
  Serial.print(" ");
  Serial.println(my_val, HEX);

  send_slider_info(val1, val2);
}

static void btn_event_cb(lv_event_t *e)
{
  int my_btn_num;

  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = (lv_obj_t *) lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    my_btn_num = *(int *) lv_event_get_user_data(e);

    Serial.print("Button pressed ");
    Serial.println(my_btn_num);

    send_button_info(my_btn_num);
  }
}

void screen_setup() 
{
  // Set the colour palette
  lv_color_t gold                 = lv_color_make(187, 165, 61);
  lv_color_t black                = lv_color_make(5, 5, 5);
  lv_color_t white                = lv_color_make(192, 192, 192);
  lv_color_t grey                 = lv_color_make(70, 70, 70);

  lv_color_t back_col             = black;
  lv_color_t text_col             = white;
  lv_color_t button_back_col      = gold;
  lv_color_t button_border_col    = gold;
  lv_color_t led_back_col         = grey;
  lv_color_t led_border_col       = gold;
  lv_color_t slider_indicator_col = grey;
  lv_color_t slider_border_col    = gold;
  lv_color_t slider_knob_col      = gold;
  lv_color_t label_text_col       = black;
  lv_color_t name_text_col         = gold;  

  // Background colour
  lv_obj_set_style_bg_color(lv_screen_active(), back_col, LV_PART_MAIN);

  // Name of application
  lv_obj_t *name_label = lv_label_create(lv_screen_active());
  lv_obj_set_pos(name_label, 20, 20);
  lv_obj_set_size(name_label, 260, 40);   
  //lv_obj_remove_flag(name_label, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_text_font(name_label, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_style_text_color(name_label, text_col, LV_PART_MAIN);

  lv_label_set_text(name_label, "Spark Control XYZ"); 
  
  // Tone bank display
  lv_obj_t *profile_background = lv_obj_create(lv_screen_active());
  lv_obj_set_pos(profile_background, 340, 15);  
  lv_obj_set_size(profile_background, 260, 40);   
  lv_obj_remove_flag(profile_background, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_bg_color(profile_background, back_col, LV_PART_MAIN);
  lv_obj_set_style_border_color(profile_background, led_border_col, LV_PART_MAIN);

  profile_label = lv_label_create(profile_background);  
  lv_obj_set_align(profile_label, LV_ALIGN_LEFT_MID);

  lv_obj_set_style_text_font(profile_label, &lv_font_montserrat_18, LV_PART_MAIN); 
  lv_obj_set_style_text_color(profile_label, text_col, LV_PART_MAIN);
  lv_obj_set_style_text_align(profile_label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);


  // Amp connected display
  amp_conn_status = lv_obj_create(lv_screen_active());
  lv_obj_set_pos(amp_conn_status, 675, 15);  
  lv_obj_set_size(amp_conn_status, 90, 40);   
  lv_obj_remove_flag(amp_conn_status, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_set_style_bg_color(amp_conn_status, led_back_col, LV_PART_MAIN);
  lv_obj_set_style_border_color(amp_conn_status, led_border_col, LV_PART_MAIN);

  lv_obj_t *amp_conn_text = lv_label_create(amp_conn_status);
  lv_obj_set_align(amp_conn_text, LV_ALIGN_LEFT_MID);

  lv_obj_set_style_text_font(amp_conn_text, &lv_font_montserrat_18, LV_PART_MAIN); 
  lv_obj_set_style_text_color(amp_conn_text, text_col, LV_PART_MAIN);
  lv_obj_set_style_text_align(profile_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

  lv_label_set_text(amp_conn_text, "AMP");

  // Define button names
  for (int i = 0; i < NUM_MAIN_BUTTONS; i++) {
    int x = 135 + 140 * (i % MAIN_BUTTONS_IN_ONE_ROW);
    int y = 110 + (i >= MAIN_BUTTONS_IN_ONE_ROW ? 200 : 0);

    name_labels[i] = lv_label_create(lv_screen_active()); 
    lv_obj_set_pos(name_labels[i], x, y);
    lv_obj_set_size(name_labels[i], 70, 60);   
    //lv_obj_remove_flag(name_labels[i], LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_text_font(name_labels[i], &lv_font_montserrat_18, LV_PART_MAIN); 
    lv_obj_set_style_text_color(name_labels[i], name_text_col, LV_PART_MAIN);
    lv_obj_set_style_text_align(name_labels[i], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        
    lv_label_set_text(name_labels[i], name_texts[i]); 
  }

  // Define buttons and their labels
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int x, y;
    
    if (i < NUM_MAIN_BUTTONS) {
      x = 60 + 140 * (i % MAIN_BUTTONS_IN_ONE_ROW);
      y = 80 + (i >= MAIN_BUTTONS_IN_ONE_ROW ? 200 : 0);
    } 
    else {
      x = 510;
      y = 80 + (i - NUM_MAIN_BUTTONS) * 140;
    }

    btns[i] = lv_button_create(lv_screen_active());   
    lv_obj_add_event_cb(btns[i], btn_event_cb, LV_EVENT_ALL, &numbers[i]);
    
    lv_obj_set_pos(btns[i], x, y); 
    lv_obj_set_size(btns[i], 90, 90); 
    lv_obj_remove_flag(btns[i], LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(btns[i], button_back_col, LV_PART_MAIN);
    lv_obj_set_style_border_color(btns[i], button_border_col, LV_PART_MAIN);

    labels[i] = lv_label_create(btns[i]); 
    lv_obj_set_align(labels[i], LV_ALIGN_CENTER);  

    lv_obj_set_style_text_color(labels[i], label_text_col, LV_PART_MAIN);
    lv_obj_set_style_text_font(labels[i], &lv_font_montserrat_18, LV_PART_MAIN); 
    lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  }

  // Define LEDS
  for (int i = 0; i < NUM_LEDS; i++) {
    int x = 70 + 140 * (i % LEDS_IN_ONE_ROW);
    int y = 190 + (i >= LEDS_IN_ONE_ROW ? 200 : 0);
    leds[i] = lv_obj_create(lv_screen_active());
    lv_obj_set_pos(leds[i], x, y);
    lv_obj_set_size(leds[i], 70, 40);
    lv_obj_remove_flag(leds[i], LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(leds[i], led_back_col, LV_PART_MAIN);
    lv_obj_set_style_border_color(leds[i], led_border_col, LV_PART_MAIN);
  }

  // Define sliders
  for (int i = 0; i < NUM_SLIDERS; i++) {
    sliders[i] = lv_slider_create(lv_screen_active());
    lv_obj_add_event_cb(sliders[i], slider_event_cb, LV_EVENT_VALUE_CHANGED, &slider_numbers[i]);
    
    lv_slider_set_value(sliders[i], 0, LV_ANIM_ON);
    lv_obj_set_pos(sliders[i], 680 + 60 * i, 80); 
    lv_obj_set_size(sliders[i], 20, 360);       
    lv_obj_set_style_border_color(sliders[i], slider_border_col, LV_PART_MAIN);
    lv_obj_set_style_border_width(sliders[i], 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sliders[i], back_col, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sliders[i], slider_indicator_col, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(sliders[i], slider_knob_col, LV_PART_KNOB);
    lv_obj_set_style_pad_all(sliders[i], 4, LV_PART_MAIN);
    lv_slider_set_range(sliders[i], 0 , 0xffff);
  }

  Serial.println("Screen started");
} 
