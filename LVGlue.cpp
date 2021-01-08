#include <lvgl.h>
#include <TFT_eSPI.h>

//Ticker tick; /* timer for interrupt handler */
#define LVGL_TICK_PERIOD 30

const int DISPLAY_COUNT = 2;
const int DISPLAY_HEIGHT = 240;
const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = DISPLAY_HEIGHT * DISPLAY_COUNT;
const int SCREEN_ROTATION = 1;
const int BUFFER_SIZE = SCREEN_WIDTH * 30;

static int DISPLAY_PINS[DISPLAY_COUNT] = {12, 14};
static int TOUCH_PINS[DISPLAY_COUNT] = {13, 4};

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
static lv_disp_buf_t disp_buf;
static lv_color_t buf[BUFFER_SIZE];

#define USE_LV_LOG 0
#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * func_name, const char * dsc)
{

  Serial.printf("[%" PRIi8 "][%s:%d] %s: %s\r\n", level, file, line, func_name, dsc);
  //Serial.flush();
}
#endif

void displayCsEnable(uint32_t display_idx, bool enable) {
  digitalWrite(DISPLAY_PINS[display_idx], enable? LOW : HIGH);
}

void touchCsEnable(uint32_t display_idx, bool enable) {
  digitalWrite(TOUCH_PINS[display_idx], enable? LOW : HIGH);
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  //log_d("Display: %p, y1: %u, y2:, %u", disp, area->y1, area->y2);

  uint32_t start_y = area->y1;
  uint32_t data_offset = 0 ;

  while (start_y < area->y2) {
    int disp_idx = start_y / DISPLAY_HEIGHT;
    if (disp_idx >= DISPLAY_COUNT) {
      log_e("Tried to paint over the display boundary!");
      break;
    }

    displayCsEnable(disp_idx, true);

    uint32_t display_y_offset = disp_idx * DISPLAY_HEIGHT;
    uint32_t yp1 = start_y - display_y_offset;
    uint32_t yp2 = area->y2 - display_y_offset;
    if (yp2 >= DISPLAY_HEIGHT) {
      yp2 = DISPLAY_HEIGHT - 1;
    }

    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (yp2 - yp1 + 1);
    //log_d("yp1: %u, yp2:, %u, h: %u", yp1, yp2, h);

    tft.startWrite();
    tft.setAddrWindow(area->x1, yp1, w, h);
    tft.pushColors((&color_p->full) + data_offset, w * h, true);
    tft.endWrite();

    displayCsEnable(disp_idx, false);

    start_y += h;
    data_offset += w * h;
  }


  lv_disp_flush_ready(disp);
}

bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    uint16_t touchX, touchY;
    bool touched = false;

    for (int disp_idx = 0; disp_idx < DISPLAY_COUNT; disp_idx++) {
      touchCsEnable(disp_idx, true);
      touched = tft.getTouch(&touchX, &touchY, 400);
      touchCsEnable(disp_idx, false);

      if (touched) {
        touchY += disp_idx * DISPLAY_HEIGHT;
        break;
      }
    }

    if(!touched) {
      data->state = LV_INDEV_STATE_REL;
    } else {
      //log_d("Touch at: %d,%d", touchX, touchY);
      if(touchX>SCREEN_WIDTH || touchY > SCREEN_HEIGHT) {
        log_w("Touch outside screen area!");
      }

      data->state = LV_INDEV_STATE_PR;
	    
      //Set the coordinates
      data->point.x = touchX;
      data->point.y = touchY;
  
    }

    return false; /*Return `false` because we are not buffering and no more data to read*/
}

void setup_lvglue() {

  lv_init();

  #if USE_LV_LOG != 0
    lv_log_register_print_cb(my_print); /* register print function for debugging */
  #endif


  /*Initialize TFT Displays (HW)*/
  for (int i = 0; i < DISPLAY_COUNT; i++) {
    pinMode(DISPLAY_PINS[i], OUTPUT);
    displayCsEnable(i, true);
    pinMode(TOUCH_PINS[i], OUTPUT);
    touchCsEnable(i, false);
  }

  tft.begin(); /* TFT init */
  tft.setRotation(SCREEN_ROTATION);

  for (int i = 0; i < DISPLAY_COUNT; i++) {
    displayCsEnable(i, false);
  }

  //uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
  uint16_t calData[5] = { 451, 3213, 525, 3090, 5 };
  tft.setTouch(calData);

  lv_disp_buf_init(&disp_buf, buf, NULL, BUFFER_SIZE);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = SCREEN_WIDTH;
  disp_drv.ver_res = SCREEN_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the input device driver*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
  indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
  indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
  lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

}
