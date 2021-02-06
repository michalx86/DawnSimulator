#ifndef GUI_h
#define GUI_h

enum RollerIndexes_t {
  TIME_ROLLER_IDX,
  ALARM_0_ROLLER_IDX,
  ALARM_1_ROLLER_IDX,
  LAST_ROLLER_IDX
};


void setup_gui();
void loop_gui();

void gui_set_temperature(float temperature);

void gui_set_date(uint16_t year, uint16_t month, uint16_t day, uint16_t dow);
uint16_t gui_get_year();
uint16_t gui_get_month();
uint16_t gui_get_day();

void gui_set_time(RollerIndexes_t idx, uint16_t hour, uint16_t minute);
uint16_t gui_get_hour(RollerIndexes_t idx);
uint16_t gui_get_minute(RollerIndexes_t idx);

void gui_set_alarm_enabled_dows(RollerIndexes_t idx, bool enabled, uint8_t enabledDows);
bool gui_get_alarm_enabled(RollerIndexes_t idx);
uint8_t gui_get_alarm_enabled_dows(RollerIndexes_t idx, bool from_status_view);

void gui_set_led_dir(int dir);
void gui_set_led_percent(int percent);

void gui_show_datetime_view();

#endif // GUI_h