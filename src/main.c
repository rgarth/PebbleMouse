#include <pebble.h>

static Window *s_main_window;
static BitmapLayer *s_dial_layer;
static RotBitmapLayer *s_hour_layer, *s_minute_layer;
  
GBitmap *dial, *h_hand, *m_hand;

static void show_time() {
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {

}

static void main_window_load(Window *window) {
  dial = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIAL);
  h_hand = gbitmap_create_with_resource(RESOURCE_ID_IMG_HOUR);
  m_hand = gbitmap_create_with_resource(RESOURCE_ID_IMG_MINUTE);
  s_dial_layer = rot_bitmap_layer_create(GRect(0, 0, 144, 168));
  s_hour_layer = rot_bitmap_layer_create(GRect(0, 0, 144, 168));
  s_minute_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_dial_layer, dial);
  rot_bitmap_layer_set_bitmap(s_hour_layer, h_hand);
  rot_bitmap_layer_set_bitmap(s_minute_layer, m_hand);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_dial_layer));
  layer_add_child(window_get_root_layer(window), rot_bitmap_layer_get_layer(s_hour_layer));
  layer_add_child(window_get_root_layer(window), rot_bitmap_layer_get_layer(s_minute_layer));

  
}

static void main_window_unload(Window *window) {
  gbitmap_destroy(dial);
  gbitmap_destroy(h_hand);
  gbitmap_destroy(m_hand);
  bitmap_layer_destroy(s_dial_layer);
  rot_bitmap_layer_destroy(s_hour_layer);
  rot_bitmap_layer_destroy(s_minute_layer);
}

static void init () {
  s_main_window = window_create();  
  window_set_background_color(s_main_window, GColorBlack); 
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  show_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

}

static void deinit () {
  // Destroy Window
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
