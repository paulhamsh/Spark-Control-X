#include <lvgl.h>
#include <Arduino.h>

// LVGL display elements and callbacks

#define NUM_BUTTONS 8
lv_obj_t *btns[NUM_BUTTONS];
lv_obj_t *labels[NUM_BUTTONS];
lv_obj_t *leds[NUM_BUTTONS];

int numbers[NUM_BUTTONS]{0, 1, 2, 6, 3, 4, 5, 7};
char *texts[NUM_BUTTONS]{"I","II","A","BANK\n  UP","III","IV","B"," BANK\nDOWN"};

void send_to_amp(int my_btn_num);

void change_colour(int led_num, int red, int green, int blue) {
  lv_obj_set_style_bg_color(leds[led_num], lv_color_make(red, green, blue), LV_PART_MAIN);
}


static void btn_event_cb(lv_event_t *e)
{
  int my_btn_num;

  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = (lv_obj_t *) lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    static uint8_t cnt = 0;
    cnt++;
    // Get the first child of the button which is the label and change its text*/
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    //lv_label_set_text_fmt(label, "Button: %d", cnt);
    Serial.print("Button pressed ");
    Serial.println(lv_label_get_text(label));

    my_btn_num = *(int *) lv_event_get_user_data(e);
    send_to_amp(my_btn_num);
    //lv_obj_set_style_bg_color(leds[my_btn_num], lv_color_hex(0x885511), LV_PART_MAIN);
  }
}

void screen_setup() 
{
  lv_color_t back_col;
  
  back_col = lv_obj_get_style_bg_color(lv_screen_active(), LV_PART_MAIN);
  lv_obj_t *label1 = lv_label_create(lv_screen_active());
  lv_label_set_text(label1, "Spark Control X");
  lv_obj_set_pos(label1, 100, 20);

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
} 
