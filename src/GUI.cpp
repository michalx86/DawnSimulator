#include <lvgl.h>

#include <stdio.h>
#include <assert.h>
#include "GUI.h"


#define DEFAULT_TXT_COLOR LV_COLOR_SILVER

#define NUM_TILES   4
#define NUM_DOWS    7
#define NUM_ALARMS  2

static const int DATE_ROLLER_X = 0;
static const int TIME_ROLLER_X = 212;
static const int ROLLER_DISTANCE_X = 2;
static const int ROLLER_Y = 70;
static const int ALARM_TIME_LABEL_OFFSET = -5;

static const size_t NUM_ROLLER_YEARS = 40;
static const size_t START_ROLLER_YEAR = 2020;
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

static lv_style_t style_obj_black;
static lv_style_t style_obj_title;
static lv_style_t style_pressed;
static lv_style_t style_disabled;


LV_IMG_DECLARE(arrow_yellow_img);
LV_IMG_DECLARE(arrow_blue_img);
LV_IMG_DECLARE(sun_img);
LV_IMG_DECLARE(stars_img);

// Color control view
static lv_obj_t * brightness_slider = NULL;
static lv_obj_t * hsv_cpicker = NULL;
static lv_obj_t * light_temp_cpicker = NULL;
static lv_obj_t * popup_label = NULL;

// Date Time and Alarm roller widgets:
static lv_obj_t *hour_rollers[LAST_ROLLER_IDX];
static lv_obj_t *minute_rollers[LAST_ROLLER_IDX];

// Status View widgets:
static lv_obj_t * date_label = NULL;
static lv_obj_t * time_label = NULL;
static lv_obj_t * led_intensity_label = NULL;
static lv_obj_t * arrow_up_widget = NULL;
static lv_obj_t * arrow_down_widget = NULL;

static lv_obj_t * temperature_label = NULL;
static lv_obj_t* dow_labels[NUM_DOWS];
static lv_obj_t* alarm_dow_status_buttons[NUM_DOWS][NUM_ALARMS];
static lv_obj_t* alarm_time_labels[NUM_ALARMS];


// Date Time View widgets:
static lv_obj_t *year_roller = NULL;
static lv_obj_t *month_roller =  NULL;
static lv_obj_t *day_roller = NULL;

// Bottom Tile View
static const int TILE_DATETIME = 0;
static const int TILE_STATUS = 1;
static const lv_point_t valid_pos[NUM_TILES] = { { 0, 0 }, { 1, 0 }, { 2, 0 }, {3, 0 } };
static lv_obj_t *tileview = NULL;

// Alarm Set View
static lv_obj_t *alarm_en_switch[NUM_ALARMS];
static lv_obj_t* alarm_dow_switches[NUM_DOWS][NUM_ALARMS];


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
                // if ((cpicker == hsv_cpicker) && (mode == LV_CPICKER_COLOR_MODE_VALUE)) {
                //     mode = LV_CPICKER_COLOR_MODE_HUE;
                //     lv_cpicker_set_color_mode(cpicker, mode);
                // }
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
        lv_cpicker_ext_t * ext = (lv_cpicker_ext_t*)lv_obj_get_ext_attr(cpicker);
        if (ext->type == LV_CPICKER_TYPE_DISC) {
          static char buf[8];
          lv_snprintf(buf, sizeof(buf), "%d", value);
          lv_obj_set_style_local_value_str(cpicker, LV_CPICKER_PART_MAIN, LV_STATE_DEFAULT, buf);
        } else {
          static char buf[8];
          lv_snprintf(buf, sizeof(buf), "%d", value);
          lv_obj_set_style_local_value_str(cpicker, LV_CPICKER_PART_KNOB, LV_STATE_DEFAULT, buf);
        }
    }
}

void set_style_black(lv_obj_t *obj) {
    lv_obj_reset_style_list(obj, LV_OBJ_PART_MAIN);
    lv_obj_add_style(obj, LV_OBJ_PART_MAIN, &style_obj_black);
}

lv_obj_t* create_custom_label(lv_obj_t* par_obj, lv_font_t* font, lv_color_t color) {
    lv_obj_t* labl = lv_label_create(par_obj, NULL);
    lv_obj_reset_style_list(labl, LV_OBJ_PART_MAIN);
    lv_obj_set_style_local_text_font(labl, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font);
    lv_obj_set_style_local_text_color(labl, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color);
    return labl;
}

void create_top_light_color_control_view(void) {
    brightness_slider = lv_slider_create(lv_scr_act(), NULL);
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

    hsv_cpicker = lv_cpicker_create(lv_scr_act(), NULL);
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

    light_temp_cpicker = lv_cpicker_create(lv_scr_act(), NULL);
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

    // popup_label = lv_label_create(lv_scr_act(), NULL);
    // lv_obj_set_style_local_text_font(popup_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
    // lv_obj_set_style_local_text_color(popup_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xCF, 0x40, 0x00));

    popup_label = create_custom_label(lv_scr_act(), LV_THEME_DEFAULT_FONT_TITLE, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_opa(popup_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(popup_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, DEFAULT_TXT_COLOR);
    lv_obj_set_style_local_border_color(popup_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xCF, 0x40, 0x00));
    lv_obj_set_style_local_border_width(popup_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
    lv_obj_set_style_local_border_opa(popup_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_radius(popup_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_hidden(popup_label, true);
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
    lv_obj_add_style(img_button, LV_IMGBTN_PART_MAIN, &style_disabled);
    lv_obj_align(img_button, NULL, LV_ALIGN_CENTER, 0, -40);
    return img_button;
}

void create_status_view(lv_obj_t* par_obj) {
    date_label = create_custom_label(par_obj, LV_THEME_DEFAULT_FONT_SUBTITLE, DEFAULT_TXT_COLOR);
    lv_obj_align(date_label, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 0);

    led_intensity_label = create_custom_label(par_obj, LV_THEME_DEFAULT_FONT_SUBTITLE, DEFAULT_TXT_COLOR);
    lv_obj_set_width(led_intensity_label, 60);
    lv_obj_align(led_intensity_label, NULL, LV_ALIGN_IN_TOP_RIGHT, -15, 0);

    arrow_up_widget = create_arrow_img(par_obj, led_intensity_label, &arrow_yellow_img);
    arrow_down_widget = create_arrow_img(par_obj, led_intensity_label, &arrow_blue_img);
    lv_obj_align(arrow_up_widget, led_intensity_label, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    lv_obj_align(arrow_down_widget, led_intensity_label, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    lv_obj_set_hidden(arrow_up_widget, true);
    lv_obj_set_hidden(arrow_down_widget, true);

    // LV_COLOR_MAKE(0xFF, 0x80, 0x00) - light orange - almost yellow
    // LV_COLOR_MAKE(0xDF, 0x50, 0x00) - orange
    // LV_COLOR_MAKE(0xCF, 0x40, 0x00) - dark orange - almost red
    time_label = create_custom_label(par_obj, LV_THEME_DEFAULT_FONT_TITLE, LV_COLOR_MAKE(0xCF, 0x40, 0x00));
    lv_obj_set_pos(time_label, 20, 60);

    temperature_label = create_custom_label(par_obj, LV_THEME_DEFAULT_FONT_TITLE, LV_COLOR_TEAL);
    lv_obj_set_pos(temperature_label, 170, 60);

    // Alarm status table
    for (int j = 0; j < NUM_ALARMS; j++) {
        alarm_time_labels[j] = create_custom_label(par_obj, LV_THEME_DEFAULT_FONT_NORMAL, DEFAULT_TXT_COLOR);
        lv_obj_set_width(alarm_time_labels[j], 50);
        for (int i = 0; i < NUM_DOWS; i++) {
            alarm_dow_status_buttons[i][j] = create_checkable_button(par_obj);
        }
    }
    lv_obj_set_pos(alarm_dow_status_buttons[0][1], 60, 0);
    lv_obj_align_y(alarm_dow_status_buttons[0][1], par_obj, LV_ALIGN_IN_BOTTOM_MID, -16);
    lv_obj_align(alarm_dow_status_buttons[0][0], alarm_dow_status_buttons[0][1], LV_ALIGN_OUT_TOP_MID, 0, -3);

    for (int j = 0; j < NUM_ALARMS; j++) {
        lv_obj_align(alarm_time_labels[j], alarm_dow_status_buttons[0][j], LV_ALIGN_OUT_LEFT_MID, ALARM_TIME_LABEL_OFFSET, 0);
        for (int i = 1; i < NUM_DOWS; i++) {
            lv_obj_align(alarm_dow_status_buttons[i][j], alarm_dow_status_buttons[i-1][j], LV_ALIGN_OUT_RIGHT_MID, 3, 0);
        }
    }

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
void create_time_rollers(lv_obj_t* par_obj, int roller_idx, size_t x, size_t y) {
    hour_rollers[roller_idx] = create_roller(par_obj, roller_hours_arr);
    lv_obj_set_pos(hour_rollers[roller_idx], x, y);

    lv_obj_t * colon_label = create_label(par_obj, ":", hour_rollers[roller_idx]);

    minute_rollers[roller_idx] = create_roller(par_obj, roller_minutes_arr);
    lv_obj_align(minute_rollers[roller_idx], colon_label, LV_ALIGN_OUT_RIGHT_MID, ROLLER_DISTANCE_X, 0);
}

void create_date_time_set_view(lv_obj_t* par_obj) {
    lv_obj_t * title = create_title(par_obj);
    lv_label_set_text_fmt(title, "Data i Czas");
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    // Date
  create_date_rollers(par_obj, DATE_ROLLER_X, ROLLER_Y);

    // Time
  create_time_rollers(par_obj, TIME_ROLLER_IDX, TIME_ROLLER_X, ROLLER_Y);
}

lv_obj_t* create_switch(lv_obj_t* par_obj) {
    lv_obj_t *sw = lv_switch_create(par_obj, NULL);
  return sw;
}

void create_alarm_set_view(lv_obj_t* par_obj, int alarm_no) {
  lv_obj_t * title = create_title(par_obj);
  lv_label_set_text_fmt(title, "Alarm %d", alarm_no + 1);
  lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  alarm_en_switch[alarm_no] = create_switch(par_obj);
  lv_obj_align(alarm_en_switch[alarm_no], title, LV_ALIGN_OUT_RIGHT_MID, 40, 0);

  // Day-of-Week switches
  lv_obj_t* dow_labels[NUM_DOWS];
  for (int i = 0; i < NUM_DOWS; i++) {
    alarm_dow_switches[i][alarm_no] = create_switch(par_obj);
  }
  lv_obj_set_pos(alarm_dow_switches[0][alarm_no], 40, 55);
  for (int i = 1; i < 5; i++) {
    lv_obj_align(alarm_dow_switches[i][alarm_no], alarm_dow_switches[i-1][alarm_no], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  }
  lv_obj_align(alarm_dow_switches[5][alarm_no], alarm_dow_switches[0][alarm_no], LV_ALIGN_OUT_RIGHT_MID, 40, 0);
  lv_obj_align(alarm_dow_switches[6][alarm_no], alarm_dow_switches[5][alarm_no], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  for (int i = 0; i < NUM_DOWS; i++) {
    dow_labels[i] = lv_label_create(par_obj, NULL);
    lv_label_set_text_fmt(dow_labels[i], "%s", dow_name_arr[i]);
    lv_obj_align(dow_labels[i], alarm_dow_switches[i][alarm_no], LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }

  // Time
  create_time_rollers(par_obj, ALARM_0_ROLLER_IDX + alarm_no, TIME_ROLLER_X, ROLLER_Y);
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

    lv_tileview_set_tile_act(tileview, valid_pos[TILE_STATUS].x, valid_pos[TILE_STATUS].y, false);
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

  /*Darken the button when disabled*/
  lv_style_init(&style_disabled);
  lv_style_set_image_recolor_opa(&style_disabled, LV_STATE_DISABLED, LV_OPA_70);
  lv_style_set_image_recolor(&style_disabled, LV_STATE_DISABLED, LV_COLOR_BLACK);

  set_style_black(lv_scr_act());

  create_top_light_color_control_view();
  create_bottom_tileview();
}


void gui_set_temperature(float temperature) {
  static char buf[5];
  snprintf(buf, sizeof(buf), "%.1f", temperature);
    lv_label_set_text_fmt(temperature_label, "%s%cC", buf, 127);
}

void gui_set_date(uint16_t year, uint16_t month, uint16_t day, uint16_t dow) {
  lv_label_set_text_fmt(date_label, "%04u-%02u-%02u", year, month, day);
  if ((year >= START_ROLLER_YEAR) && (year < START_ROLLER_YEAR + NUM_ROLLER_YEARS)) {
    lv_roller_set_selected(year_roller, year - START_ROLLER_YEAR, false);
    lv_roller_set_selected(month_roller, month - 1, false);
    lv_roller_set_selected(day_roller, day - 1, false);
  }
  for (int i = 0; i < NUM_DOWS; i++) {
    uint16_t dow_idx = (i + 2 > 7)? 1 : i + 2;

    lv_color_t bg_color = (dow_idx == dow)? LV_COLOR_GRAY : LV_COLOR_BLACK;
    lv_color_t txt_color = (dow_idx == dow)? LV_COLOR_WHITE : DEFAULT_TXT_COLOR;
    lv_obj_set_style_local_bg_opa(dow_labels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(dow_labels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, bg_color);
    lv_obj_set_style_local_text_color(dow_labels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, txt_color);
  }
}

uint16_t gui_get_year() {
  return lv_roller_get_selected(year_roller) + START_ROLLER_YEAR;
}

uint16_t gui_get_month() {
  return lv_roller_get_selected(month_roller) + 1;
}

uint16_t gui_get_day() {
  return lv_roller_get_selected(day_roller) + 1;
}

void gui_set_time(RollerIndexes_t idx, uint16_t hour, uint16_t minute) {
  if ((hour <= 23) && (minute <= 59)) {
    if (idx == TIME_ROLLER_IDX) {
      lv_label_set_text_fmt(time_label, "%02u:%02u", hour, minute);
    } else {
      int alarm_label_idx = idx - ALARM_0_ROLLER_IDX;
      lv_label_set_text_fmt(alarm_time_labels[alarm_label_idx], "%02u:%02u", hour, minute);
      lv_obj_align(alarm_time_labels[alarm_label_idx], alarm_dow_status_buttons[0][alarm_label_idx], LV_ALIGN_OUT_LEFT_MID, ALARM_TIME_LABEL_OFFSET, 0);
    }
    lv_roller_set_selected(hour_rollers[idx], hour, false);
    lv_roller_set_selected(minute_rollers[idx], minute, false);
  }
}

uint16_t gui_get_hour(RollerIndexes_t idx) {
  return lv_roller_get_selected(hour_rollers[idx]);
}
uint16_t gui_get_minute(RollerIndexes_t idx) {
  return lv_roller_get_selected(minute_rollers[idx]);
}

void gui_set_alarm_enabled_dows(RollerIndexes_t idx, bool enabled, uint8_t enabledDows) {
  assert((idx == ALARM_0_ROLLER_IDX) || (idx == ALARM_1_ROLLER_IDX));
  int alarm_idx = idx - ALARM_0_ROLLER_IDX;

  if (enabled) {
    lv_switch_on(alarm_en_switch[alarm_idx], false);
  } else {
    lv_switch_off(alarm_en_switch[alarm_idx], false);
  }

  lv_btn_state_t btn_state_dow_disabled = enabled? LV_BTN_STATE_RELEASED : LV_BTN_STATE_DISABLED;
  lv_btn_state_t btn_state_dow_enabled  = enabled? LV_BTN_STATE_CHECKED_RELEASED : LV_BTN_STATE_CHECKED_DISABLED;
  for (uint8_t i = 2; i <=8; i++) {
    uint8_t dow = (i > 7)? 1 : i;

    uint8_t alarmEnabledOnDow = ((enabledDows >> dow) & 1);
    lv_imgbtn_set_state(alarm_dow_status_buttons[i-2][alarm_idx], alarmEnabledOnDow? btn_state_dow_enabled : btn_state_dow_disabled);

    if (alarmEnabledOnDow) {
      lv_switch_on(alarm_dow_switches[i-2][alarm_idx], false);
    } else {
      lv_switch_off(alarm_dow_switches[i-2][alarm_idx], false);
    }
  }
}

bool gui_get_alarm_enabled(RollerIndexes_t idx) {
  assert((idx == ALARM_0_ROLLER_IDX) || (idx == ALARM_1_ROLLER_IDX));
  int alarm_idx = idx - ALARM_0_ROLLER_IDX;
  return lv_switch_get_state(alarm_en_switch[alarm_idx]);
}

uint8_t gui_get_alarm_enabled_dows(RollerIndexes_t idx, bool from_status_view) {
  assert((idx == ALARM_0_ROLLER_IDX) || (idx == ALARM_1_ROLLER_IDX));
  int alarm_idx = idx - ALARM_0_ROLLER_IDX;
  uint8_t enabledDows = 0;

  for (uint8_t i = 2; i <=8; i++) {
    uint8_t dow = (i > 7)? 1 : i;
    uint8_t alarmEnabledOnDow = false;
    if (from_status_view) {
      lv_btn_state_t state = lv_imgbtn_get_state(alarm_dow_status_buttons[i-2][alarm_idx]);
      alarmEnabledOnDow = ((state == LV_BTN_STATE_CHECKED_RELEASED) || (state == LV_BTN_STATE_CHECKED_PRESSED) || (state == LV_BTN_STATE_CHECKED_DISABLED))? 1 : 0;
    } else {
      alarmEnabledOnDow = lv_switch_get_state(alarm_dow_switches[i-2][alarm_idx]);
    }
    enabledDows += alarmEnabledOnDow << dow;
  }
  return enabledDows;
}

void gui_set_led_dir(int dir) {
  bool show_up_arrow = false;
  bool show_down_arrow = false;
  if ((dir == 1)) {
    show_up_arrow = true;
  } else if (dir == -1) {
    show_down_arrow = true;
  }
  lv_obj_set_hidden(arrow_up_widget, !show_up_arrow);
  lv_obj_set_hidden(arrow_down_widget, !show_down_arrow);
}

void gui_set_led_percent(int percent) {
  lv_label_set_text_fmt(led_intensity_label, "%d%%", percent);
  lv_obj_align(led_intensity_label, NULL, LV_ALIGN_IN_TOP_RIGHT, -5, 0);
}


void gui_show_datetime_view() {
  lv_tileview_set_tile_act(tileview, valid_pos[TILE_DATETIME].x, valid_pos[TILE_DATETIME].y, false);
}


#define log_e(...)
#define COLOR_NORMAL_VALUE 255
void gui_set_color(Color_t color) {
  lv_color_hsv_t hsv;
  hsv.h = 0;
  hsv.s = 0;
  hsv.v = 0;
  uint16_t brightness = 0;
  uint16_t white_temp = 50;

  for (int i = 0; i < LED_LAST; i++) {
    if (color[i] > brightness) {
      brightness = color[i];
    }
  }
  log_e("brightnes: %u", brightness);

  uint32_t col_normalized[LED_LAST];
  if (brightness > 0) {
    for (int i = 0; i < LED_LAST; i++) {
      col_normalized[i] = color[i] * COLOR_NORMAL_VALUE / brightness;
    }
    log_e("col_norm: [%u, %u, %u, %u, %u]",col_normalized[LED_R], col_normalized[LED_G], col_normalized[LED_B], col_normalized[LED_WW], col_normalized[LED_CW]);

    hsv = lv_color_rgb_to_hsv(col_normalized[LED_R], col_normalized[LED_G], col_normalized[LED_B]);
    log_e("hsv: [%u, %u, %u]",hsv.h, hsv.s, hsv.v);

    uint32_t white_max = (col_normalized[LED_WW] > col_normalized[LED_CW])? col_normalized[LED_WW] : col_normalized[LED_CW];
    if (white_max > 0) {
      if (col_normalized[LED_CW] < col_normalized[LED_WW]) {
        white_temp = 99 - col_normalized[LED_CW] * 50 / col_normalized[LED_WW];
      } else {
        white_temp = col_normalized[LED_WW] * 50 / col_normalized[LED_CW];
      }
    }
    log_e("white_max: %u, white_temp: %u",white_max, white_temp);

    if (white_max == COLOR_NORMAL_VALUE) {
      hsv.s /= 2;
    } else {
      hsv.s =  (COLOR_NORMAL_VALUE - white_max) * 50 / COLOR_NORMAL_VALUE + 49;
    }
  }

  brightness = brightness * 100 / DUTY_MAX;
  lv_slider_set_value(brightness_slider, brightness, false);
  slider_event_cb(brightness_slider, LV_EVENT_VALUE_CHANGED);
  lv_cpicker_set_hsv(hsv_cpicker, hsv);
  cpicker_event_cb(hsv_cpicker, LV_EVENT_VALUE_CHANGED);
  lv_cpicker_set_saturation(light_temp_cpicker, white_temp);
  cpicker_event_cb(light_temp_cpicker, LV_EVENT_VALUE_CHANGED);
}

/**
 * Convert a HSV color to RGB
 * @param h hue [0..359]
 * @param s saturation [0..100]
 * @param v value [0..100]
 * @return the given RGB color
 */
Color_t hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v)
{
    Color_t retColor;
    h = (uint32_t)((uint32_t)h * 255) / 360;
    s = (uint16_t)((uint16_t)s * 255) / 100;
    v = (uint16_t)((uint16_t)v * 255) / 100;

    uint8_t r, g, b;

    uint8_t region, remainder, p, q, t;

    if(s == 0) {
        retColor[LED_R] = retColor[LED_G] = retColor[LED_B] = v;
        return retColor;
    }

    region    = h / 43;
    remainder = (h - (region * 43)) * 6;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch(region) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        default:
            r = v;
            g = p;
            b = q;
            break;
    }

    retColor[LED_R] = r;
    retColor[LED_G] = g;
    retColor[LED_B] = b;
    return retColor;
  }


Color_t gui_get_color() {
  Color_t ret_color;
  uint16_t brightness = lv_slider_get_value(brightness_slider);
  lv_color_hsv_t hsv =lv_cpicker_get_hsv(hsv_cpicker);
  uint16_t light_temp = lv_cpicker_get_saturation(light_temp_cpicker);

  if (light_temp > 99) light_temp = 99;
  if (hsv.s > 99) hsv.s = 99;

  uint16_t white_value = 0;
  if (hsv.s < 50) {
    //white_value = (50 - hsv.s) * 255 / 50;
    //hsv.s = 0;
    white_value = DUTY_MAX;
    hsv.s *= 2;
  } else {
    //hsv.s = (hsv.s - 49) * 2 - 1;
    white_value = (99 - hsv.s) * DUTY_MAX / 50;
    hsv.s = 99;
  }
  //log_e("%d, %d",light_temp, white_value);

  ret_color = hsv_to_rgb(hsv.h, hsv.s, hsv.v);
  ret_color[LED_R] = ret_color[LED_R] * DUTY_MAX / 255;
  ret_color[LED_G] = ret_color[LED_G] * DUTY_MAX / 255;
  ret_color[LED_B] = ret_color[LED_B] * DUTY_MAX / 255;

  uint16_t ww = white_value;
  uint16_t cw = white_value;
  if (light_temp < 50) {
    ww = ww * light_temp / 50;
  } else {
    cw = cw * (99 - light_temp) / 50;
  }

  ret_color[LED_WW] = ww;
  ret_color[LED_CW] = cw;

  if (brightness < 100) {
    ret_color[LED_R] = ret_color[LED_R] * brightness / 100;
    ret_color[LED_G] = ret_color[LED_G] * brightness / 100;
    ret_color[LED_B] = ret_color[LED_B] * brightness / 100;
    ret_color[LED_WW] = ret_color[LED_WW] * brightness / 100;
    ret_color[LED_CW] = ret_color[LED_CW] * brightness / 100;
  }

  return ret_color;
}

void gui_show_popup(const char* txt) {
  lv_label_set_text_fmt(popup_label, " %s ", txt);
  lv_obj_align(popup_label, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_y(popup_label, SCREEN_HEIGHT / 2);
  lv_obj_set_hidden(popup_label, false);
}

void gui_hide_popup() {
  lv_obj_set_hidden(popup_label, true);
}

void setup_gui() {
  GUI();
}

void loop_gui() {
    lv_task_handler(); /* let the GUI do its work */
}
