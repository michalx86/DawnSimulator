#ifndef GUI_h
#define GUI_h

void setup_gui();
void loop_gui();

void gui_set_temperature(float temperature);

void gui_set_date(uint16_t year, uint16_t month, uint16_t day);

uint16_t gui_get_year();
uint16_t gui_get_month();
uint16_t gui_get_day();
void gui_set_time(uint16_t hour, uint16_t minute);
uint16_t gui_get_hour();
uint16_t gui_get_minute();

void gui_show_datetime_view();

#endif // GUI_h