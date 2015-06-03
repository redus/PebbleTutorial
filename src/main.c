#include <pebble.h>
#include <ctype.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
  
static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer, *s_weather_layer;
static GFont s_time_font, s_date_font, s_weather_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

const char* WEATHER_DEFAULT_TEXT = "LOADING...";

static void update_time(){
  // get tm struct
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // create long-lived buffer
  static char* buffer = "00:00";
  static char* date_buffer = "01/01/99";
  
  // wrtie hours and min into buffer
  if (clock_is_24h_style() == true){
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  strftime(date_buffer, sizeof("01/01/99"), "%Ex", tick_time);
//   APP_LOG(APP_LOG_LEVEL_ERROR, "check date %s", date_buffer);
  
  // display to text_layer
  text_layer_set_text(s_time_layer, buffer);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time(); 
}



static void main_window_load(Window *window){
  // create GBitmap, and set it to bitmap_layer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  
  // add to window's root layer
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // create time_layer
  s_time_layer = text_layer_create(GRect(5, 52, 139, 50));
  
  // add style to time_layer
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // add to window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // create date_layer
  s_date_layer = text_layer_create(GRect(0, 33, 144, 14));
  
  // add style to date_layer, and
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_14));
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // add to window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  // create weather_layer
  s_weather_layer = text_layer_create(GRect(0, 130, 144, 20));
  
  // add style to weather_layer
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_14));
  text_layer_set_font(s_weather_layer, s_weather_font);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, WEATHER_DEFAULT_TEXT);
  
  // add to window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
}
static void main_window_unload(Window *window){
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_weather_layer);
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  fonts_unload_custom_font(s_time_font);
}



static void inbox_received_callback(DictionaryIterator *iterator, void *context){
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];

  Tuple *t = dict_read_first(iterator);
  
  while(t != NULL){
    switch(t->key){
      case KEY_TEMPERATURE:
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int) t->value->int32);
      break;
      case KEY_CONDITIONS:
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized", (int) t->key);
    }
    t = dict_read_next(iterator);
  }

  int i = 0;
  char c;
  while(conditions_buffer[i]){
    c = conditions_buffer[i];
    conditions_buffer[i] = toupper((unsigned char) c);
    ++i;
  }
//   APP_LOG(APP_LOG_LEVEL_ERROR, "check uppercase: %s", conditions_buffer);
  
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void* context){
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context){
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context){
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send success");
}
static void init(){
  // register w tick_timer_service
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // create window
  s_main_window = window_create();
  // set handlers to manage elts inside the window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  // show window on watch, animated = true
  window_stack_push(s_main_window, true);
  
  // display time
  update_time();
  
  // register inbox callback
  app_message_register_inbox_received(inbox_received_callback);
  // open appmessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  // register other callbacks
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
}

static void deinit(){
  window_destroy(s_main_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
  
}