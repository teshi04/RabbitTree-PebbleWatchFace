#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static uint32_t s_current_rabbit_tree_resource_id;

static void update_rabbit_face(int current_hour) {
  static uint32_t s_rabbit_tree_resource_id;
  
  if (current_hour <= 9) {
    // 0 - 9 sleep
    s_rabbit_tree_resource_id = RESOURCE_ID_RABBIT_TREE_SLEEP;
   } else if (current_hour <= 12){
    // 10 - 12 smile
    s_rabbit_tree_resource_id = RESOURCE_ID_RABBIT_TREE_SMILE;
  } else if (current_hour <= 18){
    // 13 - 18 default
    s_rabbit_tree_resource_id = RESOURCE_ID_RABBIT_TREE_DEFAULT;
  } else {
    // 19 - 23 unlimited
    s_rabbit_tree_resource_id = RESOURCE_ID_RABBIT_TREE_UNLIMITED;
  } 
  
  if (s_rabbit_tree_resource_id != s_current_rabbit_tree_resource_id) {  
    s_current_rabbit_tree_resource_id = s_rabbit_tree_resource_id;
    s_background_bitmap = gbitmap_create_with_resource(s_rabbit_tree_resource_id);
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  }
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char s_time_text[10];
  static char s_date_text[8];

  if (clock_is_24h_style()) {
    strftime(s_time_text, sizeof(s_time_text), "%H:%M:%S", tick_time);
  } else {
    strftime(s_time_text, sizeof(s_time_text), "%I:%M:%S", tick_time);
  }
  text_layer_set_text(s_time_layer, s_time_text);
  
  strftime(s_date_text, sizeof(s_date_text), "%b %d", tick_time);
  text_layer_set_text(s_date_layer, s_date_text);
  
  update_rabbit_face(tick_time->tm_hour);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // text, image
  s_time_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(30, 15), PBL_IF_ROUND_ELSE(40, 30), bounds.size.w, 40));
  s_date_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(30, 15), PBL_IF_ROUND_ELSE(70, 60), bounds.size.w, 40));
  
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);

  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);

  s_background_layer = bitmap_layer_create(GRect(30, 0, bounds.size.w, bounds.size.h));

  // time
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);

  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

static void main_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorWhite);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
