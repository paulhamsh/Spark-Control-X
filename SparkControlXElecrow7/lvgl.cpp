#include <lvgl.h>
//#include <demos/lv_demos.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>

#include <Wire.h>
#include <TAMC_GT911.h>

// touch
#define TOUCH_GT911_SCL      20
#define TOUCH_GT911_SDA      19
#define TOUCH_GT911_INT      -1
#define TOUCH_GT911_RST      -1
#define TOUCH_GT911_ROTATION ROTATION_NORMAL
#define TOUCH_MAP_X1         800
#define TOUCH_MAP_X2         0
#define TOUCH_MAP_Y1         480
#define TOUCH_MAP_Y2         0

// lvgl
#define SCREEN_WIDTH         800
#define SCREEN_HEIGHT        480

//
// TOUCH
//

TAMC_GT911 ts = TAMC_GT911(TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST, max(TOUCH_MAP_X1, TOUCH_MAP_X2), max(TOUCH_MAP_Y1, TOUCH_MAP_Y2));

int touch_last_x, touch_last_y;

void touch_init()
{
  touch_last_x = 0;
  touch_last_y = 0;

  Wire.begin(TOUCH_GT911_SDA, TOUCH_GT911_SCL);
  ts.begin();
  ts.setRotation(TOUCH_GT911_ROTATION);
}

bool touch_touched()
{
  ts.read();
  if (ts.isTouched)
  {
#if defined(TOUCH_SWAP_XY)
    touch_last_x = map(ts.points[0].y, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, SCREEN_WIDTH - 1);
    touch_last_y = map(ts.points[0].x, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, SCREEN_HEIGHT - 1);
#else
    touch_last_x = map(ts.points[0].x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, SCREEN_WIDTH - 1);
    touch_last_y = map(ts.points[0].y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, SCREEN_HEIGHT - 1);
#endif
    return true;
  }
  else
  {
    return false;
  }
}

//
// LOVYAN
//

static uint32_t my_tick_function(void) {
  return millis();
}

class LGFX : public lgfx::LGFX_Device {
public:
  // Instances for the RGB bus and panel.
  lgfx::Bus_RGB     _bus_instance;
  lgfx::Panel_RGB   _panel_instance;

  // Constructor for the LGFX class.
  LGFX(void) {
    // Configure the RGB bus.
    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;

      // Configure data pins.
      cfg.pin_d0  = GPIO_NUM_15; // B0
      cfg.pin_d1  = GPIO_NUM_7;  // B1
      cfg.pin_d2  = GPIO_NUM_6;  // B2
      cfg.pin_d3  = GPIO_NUM_5;  // B3
      cfg.pin_d4  = GPIO_NUM_4;  // B4
      
      cfg.pin_d5  = GPIO_NUM_9;  // G0
      cfg.pin_d6  = GPIO_NUM_46; // G1
      cfg.pin_d7  = GPIO_NUM_3;  // G2
      cfg.pin_d8  = GPIO_NUM_8;  // G3
      cfg.pin_d9  = GPIO_NUM_16; // G4
      cfg.pin_d10 = GPIO_NUM_1;  // G5
      
      cfg.pin_d11 = GPIO_NUM_14; // R0
      cfg.pin_d12 = GPIO_NUM_21; // R1
      cfg.pin_d13 = GPIO_NUM_47; // R2
      cfg.pin_d14 = GPIO_NUM_48; // R3
      cfg.pin_d15 = GPIO_NUM_45; // R4

      // Configure sync and clock pins.
      cfg.pin_henable = GPIO_NUM_41;
      cfg.pin_vsync   = GPIO_NUM_40;
      cfg.pin_hsync   = GPIO_NUM_39;
      cfg.pin_pclk    = GPIO_NUM_0;
      cfg.freq_write  = 15000000;

      // Configure timing parameters for horizontal and vertical sync.
      cfg.hsync_polarity    = 0;
      cfg.hsync_front_porch = 40;
      cfg.hsync_pulse_width = 48;
      cfg.hsync_back_porch  = 40;
      
      cfg.vsync_polarity    = 0;
      cfg.vsync_front_porch = 1;
      cfg.vsync_pulse_width = 31;
      cfg.vsync_back_porch  = 13;

      // Configure polarity for clock and data transmission.
      cfg.pclk_active_neg   = 1;
      cfg.de_idle_high      = 0;
      cfg.pclk_idle_high    = 0;

      // Apply configuration to the RGB bus instance.
      _bus_instance.config(cfg);
    }

    // Configure the panel.
    {
      auto cfg = _panel_instance.config();
      cfg.memory_width  = SCREEN_WIDTH;
      cfg.memory_height = SCREEN_HEIGHT;
      cfg.panel_width   = SCREEN_WIDTH;
      cfg.panel_height  = SCREEN_HEIGHT;
      cfg.offset_x      = 0;
      cfg.offset_y      = 0;

      // Apply configuration to the panel instance.
      _panel_instance.config(cfg);
    }

    // Set the RGB bus and panel instances.
    _panel_instance.setBus(&_bus_instance);
    setPanel(&_panel_instance);
  }
};

LGFX lcd;

//
// LVGL
//

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lv_draw_sw_rgb565_swap(px_map, w * h);
  lcd.pushImageDMA(area->x1, area->y1, w, h, (uint16_t *)px_map);

  lv_disp_flush_ready(disp);
}

void my_touchpad_read( lv_indev_t * indev_driver, lv_indev_data_t * data )
{
  if (touch_touched())
  {
    data->state = LV_INDEV_STATE_PR;
    /*Set the coordinates*/
    data->point.x = touch_last_x;
    data->point.y = touch_last_y;
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

static const int buf_size_in_bytes = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(lv_color_t) / 10;
static uint16_t buf[buf_size_in_bytes / 2];

lv_display_t *disp;
lv_indev_t   *indev;

#define TFT_BL 2
#define BLACK 0

void lvgl_setup() {
  lcd.begin();
  lcd.fillScreen(BLACK);
  lcd.setTextSize(2);
  delay(200);

  lv_init();
  touch_init();

  lv_tick_set_cb(my_tick_function);

  disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, buf, NULL, buf_size_in_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);

  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif
}

void lvgl_loop() {
  lv_timer_handler();
  //lv_task_handler();
  delay(5);
}
