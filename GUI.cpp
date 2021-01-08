#include <lvgl.h>

void setup_gui() {
  // e.g.: lv_demo_sth();  
}

void loop_gui() {
    lv_task_handler(); /* let the GUI do its work */
}
