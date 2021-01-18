#include <lvgl.h>

#include <stdio.h>

#define NUM_TILES 4
static const int NUM_DOWS = 7;
static const int DATE_ROLLER_X = 0;
static const int TIME_ROLLER_X = 212;
static const int ROLLER_DISTANCE_X = 2;
static const int ROLLER_Y = 70;

static const size_t NUM_ROLLER_YEARS = 40;
static const size_t START_ROLLER_YEAR = 2021;
static const size_t ROLLER_YEAR_DIGITS = 4;

static const size_t NUM_ROLLER_MONTHS = 12;
static const size_t NUM_ROLLER_DAYS = 31;
static const size_t NUM_ROLLER_HOURS = 24;
static const size_t NUM_ROLLER_MINUTES = 60;
static const size_t ROLLER_MONTHS_LETTERS = 3;
static const size_t TWO_DIGITS = 2;

static const char* month_name_arr[] = {"Sty", "Lut", "Mar", "Kwi", "Maj", "Cze", "Lip", "Sie", "Wrz", "Paz", "Lis", "Gru"};
static const char* dow_name_arr[] = {"Pn", "Wt", "Sr", "Cz", "Pt", "So", "Nd" };

static const lv_coord_t SCREEN_WIDTH = 320;
static const lv_coord_t SCREEN_HEIGHT = 240;
static const lv_coord_t SCREEN_MARGIN = 20;
static const lv_coord_t SLIDER_LENGTH = 200;
static const lv_coord_t SLIDER_WIDTH = 20;
static const lv_coord_t HSV_CPICKER_SIZE = 160;

static char* roller_years_arr = NULL;
static char* roller_months_arr = NULL;
static char* roller_days_arr = NULL;
static char* roller_hours_arr = NULL;
static char* roller_minutes_arr = NULL;

static lv_obj_t * label;
static lv_style_t style_obj_black;
static lv_style_t style_obj_title;

static void slider_event_cb(lv_obj_t * slider, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        /*Refresh the text*/
        lv_label_set_text_fmt(label, "%d", lv_slider_get_value(slider));
    }
}

static void roller_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_roller_get_selected_str(obj, buf, sizeof(buf));
        lv_label_set_text_fmt(label, "%s\n", buf);
    }
}

static void switch_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
      lv_label_set_text_fmt(label, "%s\n", lv_switch_get_state(obj) ? "On" : "Off");
    }
}


void set_style_black(lv_obj_t *obj) {
    lv_obj_reset_style_list(obj, LV_OBJ_PART_MAIN);
    lv_obj_add_style(obj, LV_OBJ_PART_MAIN, &style_obj_black);
}

void create_top_light_color_control_view(void) {
    lv_obj_t * brightness_slider = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_set_pos(brightness_slider, SCREEN_MARGIN + 10, SCREEN_MARGIN);
    lv_obj_set_size(brightness_slider, SLIDER_WIDTH, SLIDER_LENGTH);
    lv_obj_set_event_cb(brightness_slider, slider_event_cb);         /*Assign an event function*/

    lv_obj_t * light_temp_cpicker = lv_cpicker_create(lv_scr_act(), NULL);
    lv_cpicker_set_type(light_temp_cpicker, LV_CPICKER_TYPE_RECT);
    lv_obj_set_pos(light_temp_cpicker, SCREEN_WIDTH - SLIDER_LENGTH - SCREEN_MARGIN, SCREEN_MARGIN);
    lv_obj_set_size(light_temp_cpicker, SLIDER_LENGTH, SLIDER_WIDTH);

    lv_obj_t * cpicker = lv_cpicker_create(lv_scr_act(), NULL);
    lv_obj_set_pos(cpicker, SCREEN_WIDTH - (SLIDER_LENGTH + HSV_CPICKER_SIZE)/2 - SCREEN_MARGIN, SCREEN_HEIGHT - HSV_CPICKER_SIZE- SCREEN_MARGIN);
    lv_obj_set_size(cpicker, 160, 160);

    label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "0");
    lv_obj_set_pos(label, 30, 30);
}

void create_status_view(lv_obj_t* par_obj) {

    lv_obj_t * label1 = lv_label_create(par_obj, NULL);
    lv_obj_reset_style_list(label1, LV_OBJ_PART_MAIN);
    lv_obj_set_style_local_text_font(label1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
    lv_obj_set_style_local_text_color(label1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_label_set_text_fmt(label1, "12:59 27%cC",127);
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);
}

lv_obj_t* create_title(lv_obj_t* par_obj) {
    lv_obj_t * label = lv_label_create(par_obj, NULL);
    lv_obj_reset_style_list(label, LV_OBJ_PART_MAIN);
    lv_obj_add_style(label, LV_OBJ_PART_MAIN, &style_obj_title);
    return label;
}

lv_obj_t* create_roller(lv_obj_t* par_obj, char* roller_arr) {
    lv_obj_t *roller = lv_roller_create(par_obj, NULL);
    lv_roller_set_options(roller, roller_arr, LV_ROLLER_MODE_INIFINITE);

    lv_roller_set_visible_row_count(roller, 4);
    lv_obj_set_event_cb(roller, roller_event_handler);
  return roller;
}

lv_obj_t* create_label(lv_obj_t* par_obj, char* text, lv_obj_t* align_to_obj) {
    lv_obj_t * label = lv_label_create(par_obj, NULL);
    lv_label_set_text(label, text);
    lv_obj_align(label, align_to_obj, LV_ALIGN_OUT_RIGHT_MID, ROLLER_DISTANCE_X, 0);
  return label;
}


void create_date_rollers(lv_obj_t* par_obj, size_t x, size_t y) {
    lv_obj_t *year_roller = create_roller(par_obj, roller_years_arr);
    lv_obj_set_pos(year_roller, x, y);

    lv_obj_t* dash1_label = create_label(par_obj, "-", year_roller);

    lv_obj_t *month_roller = create_roller(par_obj, roller_months_arr);
    lv_obj_align(month_roller, dash1_label, LV_ALIGN_OUT_RIGHT_MID, 1, 0);

    lv_obj_t* dash2_label = create_label(par_obj, "-", month_roller);

    lv_obj_t *day_roller = create_roller(par_obj, roller_days_arr);
    lv_obj_align(day_roller, dash2_label, LV_ALIGN_OUT_RIGHT_MID, 1, 0);
}

void create_time_rollers(lv_obj_t* par_obj, size_t x, size_t y) {
    lv_obj_t *hour_roller = create_roller(par_obj, roller_hours_arr);
    lv_obj_set_pos(hour_roller, x, y);

    lv_obj_t * colon_label = create_label(par_obj, ":", hour_roller);

    lv_obj_t *minute_roller = create_roller(par_obj, roller_minutes_arr);
    lv_obj_align(minute_roller, colon_label, LV_ALIGN_OUT_RIGHT_MID, ROLLER_DISTANCE_X, 0);
}

void create_date_time_set_view(lv_obj_t* par_obj) {
    lv_obj_t * title = create_title(par_obj);
    lv_label_set_text_fmt(title, "Data i Czas");
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    // Date
  create_date_rollers(par_obj, DATE_ROLLER_X, ROLLER_Y);

    // Time
  create_time_rollers(par_obj, TIME_ROLLER_X, ROLLER_Y);
}

lv_obj_t* create_switch(lv_obj_t* par_obj) {
    lv_obj_t *sw = lv_switch_create(par_obj, NULL);
    lv_obj_set_event_cb(sw, switch_event_handler);
  return sw;
}

void create_alarm_set_view(lv_obj_t* par_obj, int alarm_no) {
  lv_obj_t * title = create_title(par_obj);
  lv_label_set_text_fmt(title, "Alarm %d", alarm_no + 1);
  lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  lv_obj_t *sw1 = create_switch(par_obj);
  lv_obj_align(sw1, title, LV_ALIGN_OUT_RIGHT_MID, 40, 0);

  // Day-of-Week switches
  lv_obj_t* dow_switches[NUM_DOWS];
  lv_obj_t* dow_labels[NUM_DOWS];
  for (int i = 0; i < NUM_DOWS; i++) {
    dow_switches[i] = create_switch(par_obj);
  }
  lv_obj_set_pos(dow_switches[0], 40, 55);
  for (int i = 1; i < 5; i++) {
    lv_obj_align(dow_switches[i], dow_switches[i-1], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  }
  lv_obj_align(dow_switches[5], dow_switches[0], LV_ALIGN_OUT_RIGHT_MID, 40, 0);
  lv_obj_align(dow_switches[6], dow_switches[5], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  for (int i = 0; i < NUM_DOWS; i++) {
    dow_labels[i] = lv_label_create(par_obj, NULL);
    lv_label_set_text_fmt(dow_labels[i], "%s", dow_name_arr[i]);
    lv_obj_align(dow_labels[i], dow_switches[i], LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }

  // Time
  create_time_rollers(par_obj, TIME_ROLLER_X, ROLLER_Y);
}

lv_obj_t* create_bottom_tile(lv_obj_t* tileview, int tile_no) {
  lv_obj_t * tile = lv_obj_create(tileview, NULL);
  lv_obj_set_size(tile, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_pos(tile, tile_no * SCREEN_WIDTH, 0);
  set_style_black(tile);
  lv_tileview_add_element(tileview, tile);
  return tile;
}

void create_bottom_tileview(void)
{
  static lv_point_t valid_pos[NUM_TILES] = { { 0, 0 }, { 1, 0 }, { 2, 0 }, {3, 0 } };
    lv_obj_t *tileview;
    tileview = lv_tileview_create(lv_scr_act(), NULL);
    lv_obj_set_size(tileview, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(tileview, 0, SCREEN_HEIGHT);
    lv_tileview_set_valid_positions(tileview, valid_pos, NUM_TILES);
    lv_tileview_set_edge_flash(tileview, true);

    /* Populate tiles */
    lv_obj_t * tiles[NUM_TILES];
    for (int i = 0; i < NUM_TILES; i++) {
        tiles[i] = create_bottom_tile(tileview, i);
    }

    create_date_time_set_view(tiles[0]);
    create_status_view(tiles[1]);
    create_alarm_set_view(tiles[2],0);
    create_alarm_set_view(tiles[3], 1);

    lv_tileview_set_tile_act(tileview, 1, 0, false);
}

void generate_roller_text(char* text, bool is_4_digit, unsigned start, unsigned num) {
  unsigned last = start + num;
  char* format = "%02u";
  unsigned num_digits = 2;
  if (is_4_digit) {
    format = "%04u";
    num_digits = 4;
  }
  for (unsigned i = start; i < last; i++) {
    sprintf(text, format, i);
    text += num_digits;
    if (i < last - 1) {
      text[0] = '\n';
    }
    text++;
  }
  text[0] = '\0';
}

/**
 * Create a slider and write its value on a label.
 */
void GUI(void)
{
  roller_years_arr = (char*)lv_mem_alloc(NUM_ROLLER_YEARS * (ROLLER_YEAR_DIGITS + 1));
  generate_roller_text(roller_years_arr, true, START_ROLLER_YEAR, NUM_ROLLER_YEARS);

  roller_months_arr = (char*)lv_mem_alloc(NUM_ROLLER_MONTHS * (ROLLER_MONTHS_LETTERS + 1));
  char* text = roller_months_arr;
  for (unsigned i = 0; i < NUM_ROLLER_MONTHS; i++) {
    sprintf(text, "%s", month_name_arr[i]);
    text += strlen(month_name_arr[i]);
    if (i < NUM_ROLLER_MONTHS - 1) {
      text[0] = '\n';
    }
    text++;
  }
  text[0] = '\0';


  roller_days_arr = (char*)lv_mem_alloc(NUM_ROLLER_DAYS * (TWO_DIGITS + 1));
  generate_roller_text(roller_days_arr, false, 1, NUM_ROLLER_DAYS);

  roller_hours_arr = (char*)lv_mem_alloc(NUM_ROLLER_HOURS * (TWO_DIGITS + 1));
  generate_roller_text(roller_hours_arr, false, 0, NUM_ROLLER_HOURS);

  roller_minutes_arr = (char*)lv_mem_alloc(NUM_ROLLER_MINUTES * (TWO_DIGITS + 1));
  generate_roller_text(roller_minutes_arr, false, 0, NUM_ROLLER_MINUTES);

  lv_style_init(&style_obj_black);
  lv_style_set_bg_opa(&style_obj_black, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_obj_black, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_text_color(&style_obj_black, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  lv_style_init(&style_obj_title);
  // lv_style_set_bg_opa(&style_obj_title, LV_STATE_DEFAULT, LV_OPA_COVER); // TODO: check performance
  // lv_style_set_bg_color(&style_obj_title, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_text_font(&style_obj_title, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
  lv_style_set_text_color(&style_obj_title, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  set_style_black(lv_scr_act());

  create_top_light_color_control_view();
  create_bottom_tileview();
}


void setup_gui() {
  GUI();
}

void loop_gui() {
    lv_task_handler(); /* let the GUI do its work */
}
