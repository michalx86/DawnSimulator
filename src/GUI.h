#ifndef GUI_h
#define GUI_h

void setup_gui();
void loop_gui();

void gui_set_temperature(float temperature);

void gui_set_date(uint16_t year, uint16_t month, uint16_t day);

#endif // GUI_h