#include <lvgl.h>
#include <Arduino.h>

// LVGL display elements and callbacks

#define NUM_BUTTONS 8
#define NUM_SLIDERS 2
lv_obj_t *btns[NUM_BUTTONS];
lv_obj_t *labels[NUM_BUTTONS];
lv_obj_t *leds[NUM_BUTTONS];
lv_obj_t *sliders[NUM_SLIDERS];
lv_obj_t *tone_bank_label;

int numbers[NUM_BUTTONS]{0, 1, 2, 6, 3, 4, 5, 7};
char *texts[NUM_BUTTONS]{"I","II","A","BANK\n  UP","III","IV","B"," BANK\nDOWN"};
int slider_numbers[NUM_SLIDERS]{0, 1};

void send_button_info(int my_btn_num);
void send_slider_info(uint16_t slider1_val, uint16_t slider2_val);

void change_colour(int led_num, int red, int green, int blue) {
  lv_obj_set_style_bg_color(leds[led_num], lv_color_make(red, green, blue), LV_PART_MAIN);
}

void show_tone_bank(int bank) {
  lv_label_set_text_fmt(tone_bank_label, "Tone bank: %d", bank);
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
    Serial.println(texts[my_btn_num]);

    send_button_info(my_btn_num);
  }
}

void screen_setup() 
{
  lv_color_t back_col;
  
  back_col = lv_obj_get_style_bg_color(lv_screen_active(), LV_PART_MAIN);
  lv_obj_t *label1 = lv_label_create(lv_screen_active());
  lv_label_set_text(label1, "Spark Control Xish");
  lv_obj_set_pos(label1, 100, 20);
  lv_obj_set_style_text_font(label1, &lv_font_montserrat_28, LV_PART_MAIN);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    int x = 60 + 140 * (i & 3);
    int y = 100 + (i > 3 ? 170 : 0);
    btns[i] = lv_button_create(lv_screen_active());   
    lv_obj_set_pos(btns[i], x, y); 
    lv_obj_set_size(btns[i], 90, 90); 
    lv_obj_add_event_cb(btns[i], btn_event_cb, LV_EVENT_ALL, &numbers[i]);

    leds[i] = lv_obj_create(lv_screen_active());
    lv_obj_set_pos(leds[i] , x + 10, y + 110);
    lv_obj_set_size(leds[i], 70, 50);
    if (i == 3 || i == 7) {
      lv_obj_set_style_bg_color(leds[i], back_col, LV_PART_MAIN);
      lv_obj_set_style_border_color(leds[i], back_col, LV_PART_MAIN);
    }
    else
      lv_obj_set_style_bg_color(leds[i], lv_color_make(0, 0, 0), LV_PART_MAIN);
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    labels[i] = lv_label_create(btns[i]);   
    //lv_label_set_text_fmt(labels[i], "%i", i);
    lv_label_set_text(labels[i], texts[i]); 
    lv_obj_center(labels[i]);
  }

  tone_bank_label = lv_label_create(lv_screen_active());
  lv_label_set_text(tone_bank_label, "Tone bank: 1");
  lv_obj_set_pos(tone_bank_label, 600, 20);
  lv_obj_set_style_text_font(tone_bank_label, &lv_font_montserrat_28, LV_PART_MAIN);

  for (int i = 0; i < NUM_SLIDERS; i++) {
    sliders[i] = lv_slider_create(lv_screen_active());
    lv_slider_set_value(sliders[i], 0, LV_ANIM_ON);
    lv_obj_set_pos(sliders[i], 650 + 60 * i , 100); 
    lv_obj_set_size(sliders[i], 20, 300);       
    lv_slider_set_range(sliders[i], 0 , 0xffff);
    lv_obj_add_event_cb(sliders[i], slider_event_cb, LV_EVENT_VALUE_CHANGED, &slider_numbers[i]);
  }
} 
