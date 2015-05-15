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

}

static void main_window_unload(Window *window) {
  
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
