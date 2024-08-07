#include <lvgl.h>
#include <Arduino.h>

// LVGL display elements and callbacks

#define NUM_BUTTONS 8
lv_obj_t *btns[NUM_BUTTONS];
lv_obj_t *labels[NUM_BUTTONS];
int numbers[NUM_BUTTONS]{0,1,2,3,4,5,6,7};
char *texts[NUM_BUTTONS]{"I","II","A","MODE","III","IV","B","MODE"};

void send_to_amp(int my_btn_num);

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
  }
}

void screen_setup() 
{
  lv_obj_t *label1 = lv_label_create(lv_screen_active());
  lv_label_set_text(label1, "Spark Control X");
  lv_obj_set_pos(label1, 100, 20);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    
    int x = 60 + 140 * (i & 3);
    int y = 120 + (i > 3 ? 140 : 0);
    btns[i] = lv_button_create(lv_screen_active());   
    lv_obj_set_pos(btns[i], x, y); 
    lv_obj_set_size(btns[i], 90, 90); 
    lv_obj_add_event_cb(btns[i], btn_event_cb, LV_EVENT_ALL, &numbers[i]);
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    labels[i] = lv_label_create(btns[i]);   
    //lv_label_set_text_fmt(labels[i], "%i", i);
    lv_label_set_text(labels[i], texts[i]); 
    lv_obj_center(labels[i]);
  }
}
