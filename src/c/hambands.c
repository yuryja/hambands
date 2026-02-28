#include <pebble.h>

#define STORAGE_KEY_LOCALE 100

static Window *s_window;
static TextLayer *s_title_layer, *s_info_layer, *s_date_layer;
static TextLayer *s_head_b, *s_head_d, *s_head_n;
static TextLayer *s_band_layers[4], *s_day_layers[4], *s_night_layers[4];

static char s_info_buf[64], s_date_buf[64];
static char s_day_status_bufs[4][16], s_night_status_bufs[4][16];
static char s_current_locale[8];

static void set_layer_style(TextLayer *layer, GColor bg, GColor text,
                            GFont font, GTextAlignment align) {
  text_layer_set_background_color(layer, bg);
  text_layer_set_text_color(layer, text);
  text_layer_set_font(layer, font);
  text_layer_set_text_alignment(layer, align);
}

typedef struct {
  char *title;
  char *loading;
  char *h_band, *h_day, *h_night;
  char *upd_prefix;
  char *good;
  char *fair;
  char *poor;
} LocStrings;

static LocStrings get_strings() {
  if (strncmp(s_current_locale, "es", 2) == 0) {
    return (LocStrings){
        "CONDICIONES DX", "Cargando...", "Banda", "Dia", "Noche",
        "Act:",           "Buena",       "Reg.",  "Mala"};
  } else if (strncmp(s_current_locale, "fr", 2) == 0) {
    return (LocStrings){
        "CONDITIONS DX", "Chargement...", "Bande", "Jour", "Nuit",
        "Mis à jour:",   "Bonne",         "Moy.",  "Mauv."};
  } else if (strncmp(s_current_locale, "it", 2) == 0) {
    return (LocStrings){
        "CONDIZIONI DX", "Caricamento...", "Banda", "Giorno", "Notte",
        "Agg:",          "Buona",          "Media", "Scarsa"};
  } else if (strncmp(s_current_locale, "pt", 2) == 0) {
    return (LocStrings){
        "CONDIÇÕES DX", "Carregando...", "Banda", "Dia", "Noite",
        "Atu:",         "Boa",           "Reg.",  "Ruim"};
  } else { // Default English
    return (LocStrings){"DX CONDITIONS", "Loading...", "Band", "Day", "Night",
                        "Upd:",          "Good",       "Fair", "Poor"};
  }
}

static void update_fixed_texts() {
  LocStrings s = get_strings();
  if (s_title_layer)
    text_layer_set_text(s_title_layer, s.title);
  if (s_head_b)
    text_layer_set_text(s_head_b, s.h_band);
  if (s_head_d)
    text_layer_set_text(s_head_d, s.h_day);
  if (s_head_n)
    text_layer_set_text(s_head_n, s.h_night);

  if (s_info_layer) {
    const char *current_info = text_layer_get_text(s_info_layer);
    if (!current_info || strlen(current_info) == 0 ||
        strstr(current_info, ":") == NULL) {
      text_layer_set_text(s_info_layer, s.loading);
    }
  }
}

static const char *translate_status(const char *status, LocStrings s) {
  if (!status)
    return "?";
  if (strcmp(status, "Good") == 0)
    return s.good;
  if (strcmp(status, "Fair") == 0)
    return s.fair;
  if (strcmp(status, "Poor") == 0)
    return s.poor;
  return status;
}

static void inbox_received_callback(DictionaryIterator *iterator,
                                    void *context) {
  LocStrings s = get_strings();

  Tuple *lang_t = dict_find(iterator, MESSAGE_KEY_LANG);
  if (lang_t && lang_t->type == TUPLE_CSTRING) {
    const char *lang_val = lang_t->value->cstring;
    if (strcmp(lang_val, "system") == 0) {
      snprintf(s_current_locale, sizeof(s_current_locale), "%s",
               i18n_get_system_locale());
    } else {
      snprintf(s_current_locale, sizeof(s_current_locale), "%s", lang_val);
    }
    persist_write_string(STORAGE_KEY_LOCALE, s_current_locale);
    update_fixed_texts();
    s = get_strings();
  }

  Tuple *sfi_t = dict_find(iterator, MESSAGE_KEY_SFI);
  Tuple *k_t = dict_find(iterator, MESSAGE_KEY_K_INDEX);
  if (sfi_t && k_t) {
    snprintf(s_info_buf, sizeof(s_info_buf), "SFI:%d  K-IDX:%d",
             (int)sfi_t->value->int32, (int)k_t->value->int32);
    text_layer_set_text(s_info_layer, s_info_buf);
  }

  Tuple *up_t = dict_find(iterator, MESSAGE_KEY_UPDATED);
  if (up_t && up_t->type == TUPLE_CSTRING) {
    char date_raw[32];
    strncpy(date_raw, up_t->value->cstring, sizeof(date_raw) - 1);
    date_raw[sizeof(date_raw) - 1] = '\0';

    char *day = date_raw;
    char *month = strchr(day, '|');
    if (month) {
      *month = '\0';
      month++;
      char *year = strchr(month, '|');
      if (year) {
        *year = '\0';
        year++;
        if (strncmp(s_current_locale, "en", 2) == 0) {
          snprintf(s_date_buf, sizeof(s_date_buf), "%s %s %s, %s", s.upd_prefix,
                   month, day, year);
        } else {
          snprintf(s_date_buf, sizeof(s_date_buf), "%s %s %s %s", s.upd_prefix,
                   day, month, year);
        }
      } else {
        snprintf(s_date_buf, sizeof(s_date_buf), "%s %s", s.upd_prefix,
                 up_t->value->cstring);
      }
    } else {
      snprintf(s_date_buf, sizeof(s_date_buf), "%s %s", s.upd_prefix,
               up_t->value->cstring);
    }
    text_layer_set_text(s_date_layer, s_date_buf);
  }

  uint32_t keys_d[4] = {MESSAGE_KEY_BAND_80_40_D, MESSAGE_KEY_BAND_30_20_D,
                        MESSAGE_KEY_BAND_17_15_D, MESSAGE_KEY_BAND_12_10_D};
  uint32_t keys_n[4] = {MESSAGE_KEY_BAND_80_40_N, MESSAGE_KEY_BAND_30_20_N,
                        MESSAGE_KEY_BAND_17_15_N, MESSAGE_KEY_BAND_12_10_N};

  for (int i = 0; i < 4; i++) {
    Tuple *td = dict_find(iterator, keys_d[i]);
    if (td && td->type == TUPLE_CSTRING) {
      snprintf(s_day_status_bufs[i], sizeof(s_day_status_bufs[i]), "%s",
               translate_status(td->value->cstring, s));
      text_layer_set_text(s_day_layers[i], s_day_status_bufs[i]);
    }
    Tuple *tn = dict_find(iterator, keys_n[i]);
    if (tn && tn->type == TUPLE_CSTRING) {
      snprintf(s_night_status_bufs[i], sizeof(s_night_status_bufs[i]), "%s",
               translate_status(tn->value->cstring, s));
      text_layer_set_text(s_night_layers[i], s_night_status_bufs[i]);
    }
  }
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_title_layer = text_layer_create(GRect(0, 0, bounds.size.w, 28));
  set_layer_style(s_title_layer, GColorBlack, GColorWhite,
                  fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                  GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  s_info_layer = text_layer_create(GRect(0, 28, bounds.size.w, 22));
  set_layer_style(s_info_layer, GColorDarkGray, GColorWhite,
                  fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                  GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_info_layer));

  int col1_x = 5, col1_w = 48, col2_x = 55, col2_w = 42, col3_x = 98,
      col3_w = 42, head_y = 52;
  s_head_b = text_layer_create(GRect(col1_x, head_y, col1_w, 18));
  set_layer_style(s_head_b, GColorClear, GColorBlack,
                  fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                  GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_head_b));
  s_head_d = text_layer_create(GRect(col2_x, head_y, col2_w, 18));
  set_layer_style(s_head_d, GColorClear, GColorBlack,
                  fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                  GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_head_d));
  s_head_n = text_layer_create(GRect(col3_x, head_y, col3_w, 18));
  set_layer_style(s_head_n, GColorClear, GColorBlack,
                  fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                  GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_head_n));

  char *band_names[] = {"80-40m", "30-20m", "17-15m", "12-10m"};
  GFont row_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  int start_y = 70;
  for (int i = 0; i < 4; i++) {
    int y = start_y + (i * 18);
    s_band_layers[i] = text_layer_create(GRect(col1_x, y, col1_w, 18));
    text_layer_set_text(s_band_layers[i], band_names[i]);
    set_layer_style(s_band_layers[i], GColorClear, GColorBlack, row_font,
                    GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_band_layers[i]));
    s_day_layers[i] = text_layer_create(GRect(col2_x, y, col2_w, 18));
    set_layer_style(s_day_layers[i], GColorClear, GColorBlack, row_font,
                    GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_day_layers[i]));
    s_night_layers[i] = text_layer_create(GRect(col3_x, y, col3_w, 18));
    set_layer_style(s_night_layers[i], GColorClear, GColorBlack, row_font,
                    GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_night_layers[i]));
  }

  s_date_layer =
      text_layer_create(GRect(0, bounds.size.h - 18, bounds.size.w, 18));
  set_layer_style(s_date_layer, GColorBlack, GColorWhite,
                  fonts_get_system_font(FONT_KEY_GOTHIC_14),
                  GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  update_fixed_texts();
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_info_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_head_b);
  text_layer_destroy(s_head_d);
  text_layer_destroy(s_head_n);
  for (int i = 0; i < 4; i++) {
    text_layer_destroy(s_band_layers[i]);
    text_layer_destroy(s_day_layers[i]);
    text_layer_destroy(s_night_layers[i]);
  }
}

static void prv_init(void) {
  if (persist_exists(STORAGE_KEY_LOCALE)) {
    persist_read_string(STORAGE_KEY_LOCALE, s_current_locale,
                        sizeof(s_current_locale));
  } else {
    snprintf(s_current_locale, sizeof(s_current_locale), "%s",
             i18n_get_system_locale());
  }
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
                                           .load = prv_window_load,
                                           .unload = prv_window_unload,
                                       });
  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(512, 512);
  window_stack_push(s_window, true);
}

static void prv_deinit(void) { window_destroy(s_window); }

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
