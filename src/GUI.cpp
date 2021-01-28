#include <lvgl.h>

#include <stdio.h>

#define DEFAULT_TXT_COLOR LV_COLOR_SILVER

#define NUM_TILES   4
#define NUM_DOWS    7
#define NUM_ALARMS  2

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
static lv_style_t style_pressed;

static lv_obj_t* alarm_dow_status_buttons[NUM_DOWS][NUM_ALARMS];
static lv_obj_t* alarm_time_labels[NUM_ALARMS];

LV_IMG_DECLARE(arrow_yellow_img);
LV_IMG_DECLARE(arrow_blue_img);
LV_IMG_DECLARE(sun_img);
LV_IMG_DECLARE(stars_img);

// Status View widgets:
static lv_obj_t * date_label = NULL;
static lv_obj_t * temperature_label = NULL;

// Date Time View widgets:
lv_obj_t *year_roller = NULL;
lv_obj_t *month_roller =  NULL;
lv_obj_t *day_roller = NULL;


static void slider_event_cb(lv_obj_t * slider, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        if(lv_slider_get_type(slider) == LV_SLIDER_TYPE_NORMAL) {
            static char buf[16];
            lv_snprintf(buf, sizeof(buf), "%d", lv_slider_get_value(slider));
            lv_obj_set_style_local_value_str(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, buf);
        }
    }
}

static void cpicker_event_cb(lv_obj_t * cpicker, lv_event_t event)
{
    if ((event == LV_EVENT_VALUE_CHANGED) || (event == LV_EVENT_PRESSING)) {
        lv_cpicker_color_mode_t mode = lv_cpicker_get_color_mode(cpicker);
        if (event == LV_EVENT_PRESSING) {
            static lv_cpicker_color_mode_t old_mode = -1;
            if (mode == old_mode) {
                return;
            } else {
                old_mode = mode;
            }
        }

        uint16_t value = 0;
        switch (mode) {
        case LV_CPICKER_COLOR_MODE_HUE:
            value = lv_cpicker_get_hue(cpicker);
            break;
        case LV_CPICKER_COLOR_MODE_SATURATION:
            value = lv_cpicker_get_saturation(cpicker);
            break;
        case LV_CPICKER_COLOR_MODE_VALUE:
            value = lv_cpicker_get_value(cpicker);
            break;
        default:
            break;
        }
        static char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d", value);
        lv_cpicker_ext_t * ext = (lv_cpicker_ext_t*)lv_obj_get_ext_attr(cpicker);
        uint32_t part_type = (ext->type == LV_CPICKER_TYPE_DISC)? LV_CPICKER_PART_MAIN : LV_CPICKER_PART_KNOB;
        lv_obj_set_style_local_value_str(cpicker, part_type, LV_STATE_DEFAULT, buf);
    }
}

static void roller_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        static char buf[32];
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
    lv_obj_set_pos(brightness_slider, SCREEN_MARGIN + 20, SCREEN_MARGIN);
    lv_obj_set_size(brightness_slider, SLIDER_WIDTH, SLIDER_LENGTH);
    lv_obj_set_event_cb(brightness_slider, slider_event_cb);
    lv_obj_set_style_local_bg_opa(brightness_slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(brightness_slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_border_width(brightness_slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 2);
    lv_obj_set_style_local_border_opa(brightness_slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_border_color(brightness_slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_value_color(brightness_slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, DEFAULT_TXT_COLOR);  // text color
    lv_obj_set_style_local_value_ofs_x(brightness_slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -30);
    lv_obj_set_style_local_value_opa(brightness_slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(brightness_slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(brightness_slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(brightness_slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_X);
    lv_obj_set_style_local_transition_prop_6(brightness_slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);

    lv_obj_t * hsv_cpicker = lv_cpicker_create(lv_scr_act(), NULL);
    lv_obj_set_pos(hsv_cpicker, SCREEN_WIDTH - (SLIDER_LENGTH + HSV_CPICKER_SIZE)/2 - SCREEN_MARGIN, SCREEN_HEIGHT - HSV_CPICKER_SIZE- SCREEN_MARGIN);
    lv_obj_set_size(hsv_cpicker, 160, 160);
    lv_obj_set_event_cb(hsv_cpicker, cpicker_event_cb);
    lv_obj_set_style_local_value_color(hsv_cpicker, LV_CPICKER_PART_MAIN, LV_STATE_DEFAULT, DEFAULT_TXT_COLOR);  // text color
    lv_obj_set_style_local_value_ofs_x(hsv_cpicker, LV_CPICKER_PART_MAIN, LV_STATE_DEFAULT, -70);
    lv_obj_set_style_local_value_ofs_y(hsv_cpicker, LV_CPICKER_PART_MAIN, LV_STATE_DEFAULT, 70);
    lv_obj_set_style_local_value_opa(hsv_cpicker, LV_CPICKER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(hsv_cpicker, LV_CPICKER_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(hsv_cpicker, LV_CPICKER_PART_MAIN, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_6(hsv_cpicker, LV_CPICKER_PART_MAIN, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);

    lv_obj_t * light_temp_cpicker = lv_cpicker_create(lv_scr_act(), NULL);
    lv_cpicker_set_type(light_temp_cpicker, LV_CPICKER_TYPE_RECT);
    lv_obj_set_pos(light_temp_cpicker, SCREEN_WIDTH - SLIDER_LENGTH - SCREEN_MARGIN, SCREEN_MARGIN);
    lv_obj_set_size(light_temp_cpicker, SLIDER_LENGTH, SLIDER_WIDTH);
    lv_cpicker_set_color(light_temp_cpicker, LV_COLOR_MAKE(0xFF, 0xD5, 0x00));
    lv_cpicker_set_color_mode(light_temp_cpicker, LV_CPICKER_COLOR_MODE_SATURATION);
    lv_cpicker_set_color_mode_fixed(light_temp_cpicker, true);
    lv_obj_set_event_cb(light_temp_cpicker, cpicker_event_cb);
    lv_obj_set_style_local_value_color(light_temp_cpicker, LV_CPICKER_PART_KNOB, LV_STATE_DEFAULT, DEFAULT_TXT_COLOR);  // text color
    lv_obj_set_style_local_value_ofs_x(light_temp_cpicker, LV_CPICKER_PART_KNOB, LV_STATE_FOCUSED, -19);
    lv_obj_set_style_local_value_ofs_y(light_temp_cpicker, LV_CPICKER_PART_KNOB, LV_STATE_FOCUSED, 25);
    lv_obj_set_style_local_value_opa(light_temp_cpicker, LV_CPICKER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(light_temp_cpicker, LV_CPICKER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(light_temp_cpicker, LV_CPICKER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_4(light_temp_cpicker, LV_CPICKER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_X);
    lv_obj_set_style_local_transition_prop_5(light_temp_cpicker, LV_CPICKER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(light_temp_cpicker, LV_CPICKER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);


    label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "0");
    lv_obj_set_pos(label, 30, 30);
}

lv_obj_t* create_custom_label(lv_obj_t* par_obj, lv_font_t* font, lv_color_t color) {
    lv_obj_t* labl = lv_label_create(par_obj, NULL);
    lv_obj_reset_style_list(labl, LV_OBJ_PART_MAIN);
    lv_obj_set_style_local_text_font(labl, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font);
    lv_obj_set_style_local_text_color(labl, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color);
    return labl;
}
lv_obj_t* create_arrow_img(lv_obj_t* par_obj, lv_obj_t* align_obj, const void * img_data) {
    lv_obj_t * img = lv_img_create(par_obj, NULL);
    lv_img_set_src(img, img_data);
    lv_obj_align(img, align_obj, LV_ALIGN_OUT_LEFT_MID, -3, 0);
    return img;
}

lv_obj_t* create_checkable_button(lv_obj_t* par_obj) {
    /*Create an Image button*/
    lv_obj_t * img_button = lv_imgbtn_create(par_obj, NULL);
    lv_imgbtn_set_src(img_button, LV_BTN_STATE_RELEASED, &stars_img);
    lv_imgbtn_set_src(img_button, LV_BTN_STATE_PRESSED, &stars_img);
    lv_imgbtn_set_src(img_button, LV_BTN_STATE_CHECKED_RELEASED, &sun_img);
    lv_imgbtn_set_src(img_button, LV_BTN_STATE_CHECKED_PRESSED, &sun_img);
    lv_imgbtn_set_checkable(img_button, true);
    lv_obj_add_style(img_button, LV_IMGBTN_PART_MAIN, &style_pressed);
    lv_obj_align(img_button, NULL, LV_ALIGN_CENTER, 0, -40);
    return img_button;
}

void create_status_view(lv_obj_t* par_obj) {
    date_label = create_custom_label(par_obj, LV_THEME_DEFAULT_FONT_SUBTITLE, DEFAULT_TXT_COLOR);
    lv_obj_align(date_label, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 0);

    lv_obj_t * led_intensity_label = create_custom_label(par_obj, LV_THEME_DEFAULT_FONT_SUBTITLE, DEFAULT_TXT_COLOR);
    lv_label_set_text(led_intensity_label, "100%");
    lv_obj_align(led_intensity_label, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 0);

    lv_obj_t * arrow_yellow_widget = create_arrow_img(par_obj, led_intensity_label, &arrow_yellow_img);
    lv_obj_t * arrow_blue_widget = create_arrow_img(par_obj, led_intensity_label, &arrow_blue_img);
    lv_obj_set_hidden(arrow_blue_widget, true);

    // LV_COLOR_MAKE(0xFF, 0x80, 0x00) - light orange - almost yellow
    // LV_COLOR_MAKE(0xDF, 0x50, 0x00) - orange
    // LV_COLOR_MAKE(0xCF, 0x40, 0x00) - darg orange - almost red
    lv_obj_t * time_label = create_custom_label(par_obj, LV_THEME_DEFAULT_FONT_TITLE, LV_COLOR_MAKE(0xCF, 0x40, 0x00));
    lv_label_set_text_fmt(time_label, "12:59");
    lv_obj_set_pos(time_label, 20, 60);

    temperature_label = create_custom_label(par_obj, LV_THEME_DEFAULT_FONT_TITLE, LV_COLOR_TEAL);
    lv_obj_set_pos(temperature_label, 170, 60);

    // Alarm status table
    for (int j = 0; j < NUM_ALARMS; j++) {
        alarm_time_labels[j] = create_custom_label(par_obj, LV_THEME_DEFAULT_FONT_NORMAL, DEFAULT_TXT_COLOR);
        for (int i = 0; i < NUM_DOWS; i++) {
            alarm_dow_status_buttons[i][j] = create_checkable_button(par_obj);
        }
    }
    lv_obj_set_pos(alarm_dow_status_buttons[0][1], 60, 0);
    lv_obj_align_y(alarm_dow_status_buttons[0][1], par_obj, LV_ALIGN_IN_BOTTOM_MID, -16);
    lv_obj_align(alarm_dow_status_buttons[0][0], alarm_dow_status_buttons[0][1], LV_ALIGN_OUT_TOP_MID, 0, -3);

    for (int j = 0; j < NUM_ALARMS; j++) {
        lv_label_set_text_fmt(alarm_time_labels[j], "%d:54", j*20+4);
        lv_obj_align(alarm_time_labels[j], alarm_dow_status_buttons[0][j], LV_ALIGN_OUT_LEFT_MID, -5, 0);
        for (int i = 1; i < NUM_DOWS; i++) {
            lv_obj_align(alarm_dow_status_buttons[i][j], alarm_dow_status_buttons[i-1][j], LV_ALIGN_OUT_RIGHT_MID, 3, 0);
        }
    }

    lv_obj_t* dow_labels[NUM_DOWS];

    for (int i = 0; i < NUM_DOWS; i++) {
      dow_labels[i] = lv_label_create(par_obj, NULL);
      lv_label_set_text_fmt(dow_labels[i], "%s", dow_name_arr[i]);
      lv_obj_align(dow_labels[i], alarm_dow_status_buttons[i][0], LV_ALIGN_OUT_TOP_MID, 0, 0);
    }
}

lv_obj_t* create_title(lv_obj_t* par_obj) {
    lv_obj_t * labl = lv_label_create(par_obj, NULL);
    lv_obj_reset_style_list(labl, LV_OBJ_PART_MAIN);
    lv_obj_add_style(labl, LV_OBJ_PART_MAIN, &style_obj_title);
    return labl;
}

lv_obj_t* create_roller(lv_obj_t* par_obj, char* roller_arr) {
    lv_obj_t *roller = lv_roller_create(par_obj, NULL);
    lv_roller_set_options(roller, roller_arr, LV_ROLLER_MODE_INIFINITE);

    lv_roller_set_visible_row_count(roller, 4);
    lv_obj_set_event_cb(roller, roller_event_handler);
  return roller;
}

lv_obj_t* create_label(lv_obj_t* par_obj, const char* text, const lv_obj_t* align_to_obj) {
    lv_obj_t * labl = lv_label_create(par_obj, NULL);
    lv_label_set_text(labl, text);
    lv_obj_align(labl, align_to_obj, LV_ALIGN_OUT_RIGHT_MID, ROLLER_DISTANCE_X, 0);
  return labl;
}

void create_date_rollers(lv_obj_t* par_obj, size_t x, size_t y) {
    year_roller = create_roller(par_obj, roller_years_arr);
    lv_obj_set_pos(year_roller, x, y);

    lv_obj_t* dash1_label = create_label(par_obj, "-", year_roller);

    month_roller = create_roller(par_obj, roller_months_arr);
    lv_obj_align(month_roller, dash1_label, LV_ALIGN_OUT_RIGHT_MID, 1, 0);

    lv_obj_t* dash2_label = create_label(par_obj, "-", month_roller);

    day_roller = create_roller(par_obj, roller_days_arr);
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
  const char* format = "%02u";
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

  /*Darken the button when pressed*/
  lv_style_init(&style_pressed);
  lv_style_set_image_recolor_opa(&style_pressed, LV_STATE_PRESSED, LV_OPA_30);
  lv_style_set_image_recolor(&style_pressed, LV_STATE_PRESSED, LV_COLOR_BLACK);

  set_style_black(lv_scr_act());

  create_top_light_color_control_view();
  create_bottom_tileview();
}


void gui_set_temperature(float temperature) {
  static char buf[5];
  snprintf(buf, sizeof(buf), "%.1f", temperature);
    lv_label_set_text_fmt(temperature_label, "%s%cC", buf, 127);
}

void gui_set_date(uint16_t year, uint16_t month, uint16_t day) {
  lv_label_set_text_fmt(date_label, "%04d-%02d-%02d", year, month, day);
  if ((year >= START_ROLLER_YEAR) && (year < START_ROLLER_YEAR + NUM_ROLLER_YEARS)) {
    lv_roller_set_selected(year_roller, year - START_ROLLER_YEAR, true);
  }
  // month_roller
  // day_roller
}

void setup_gui() {
  GUI();
}

void loop_gui() {
    lv_task_handler(); /* let the GUI do its work */
}
