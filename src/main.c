#include <pebble.h>
#define KEY_COLOR 0

static Window *s_main_window;
static BitmapLayer *s_dial_layer, *s_mickey_layer;
static RotBitmapLayer *s_hour_layer, *s_minute_layer;
static Layer *s_date_layer;
GBitmap *dial, *mickey, *h_hand, *m_hand;
int color = 0xFFFFAA;
int mday;

static void show_time() {
  time_t temp = time(NULL);
  int minutes = localtime(&temp)->tm_min;
  mday = localtime(&temp)->tm_mday;
  int32_t m_angle = TRIG_MAX_ANGLE * minutes / 60;

  int hour = localtime(&temp)->tm_hour;
  int32_t h_angle = TRIG_MAX_ANGLE * ((hour % 12)*30 + minutes/2) / 360;

  rot_bitmap_layer_set_angle(s_hour_layer, h_angle);
  rot_bitmap_layer_set_angle(s_minute_layer, m_angle);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  show_time();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received!");

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_COLOR:
      // color returned as a hex string
      if (t->value->int32 > 0) {
        color = t->value->int32;
        APP_LOG(APP_LOG_LEVEL_INFO, "Saving color: %x", color);
        window_set_background_color(s_main_window, GColorFromHEX(color)); 
        persist_write_int(KEY_COLOR, color);
      }
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }
    // Look for next item
    t = dict_read_next(iterator);
  }
}


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void draw_date(Layer *this_layer, GContext *ctx) {
  char str_mday[] = "32";
  snprintf(str_mday, sizeof(str_mday), "%i", mday);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(this_layer), 0, GCornerNone);
  graphics_draw_rect(ctx, layer_get_bounds(this_layer));
  graphics_draw_text(ctx,
                     str_mday,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     layer_get_bounds(this_layer), 
                     GTextOverflowModeWordWrap,
                     GTextAlignmentCenter, 
                     NULL);
}


static void main_window_load(Window *window) {
  // Dial bitmap layer
  dial = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIAL);
  s_dial_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_dial_layer, dial);
  bitmap_layer_set_compositing_mode(s_dial_layer, GCompOpSet);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_dial_layer));

  // Armless Mickey imaage
  mickey = gbitmap_create_with_resource(RESOURCE_ID_IMG_MICKEY);
  s_mickey_layer = bitmap_layer_create(GRect(0, 12, 144, 144));
  bitmap_layer_set_bitmap(s_mickey_layer, mickey);
  bitmap_layer_set_compositing_mode(s_mickey_layer, GCompOpSet);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_mickey_layer));

  // Date layer
  s_date_layer = layer_create(GRect(100, 74, 21, 19));
  layer_set_update_proc(s_date_layer, draw_date);
  layer_add_child(window_get_root_layer(window), s_date_layer);

  // Arms
  m_hand = gbitmap_create_with_resource(RESOURCE_ID_IMG_MINUTE);
  h_hand = gbitmap_create_with_resource(RESOURCE_ID_IMG_HOUR);
  s_minute_layer = rot_bitmap_layer_create(m_hand);
  s_hour_layer = rot_bitmap_layer_create(h_hand);
  rot_bitmap_layer_set_corner_clip_color(s_minute_layer, GColorClear);
  rot_bitmap_layer_set_corner_clip_color(s_hour_layer, GColorClear);
  rot_bitmap_set_compositing_mode(s_minute_layer, GCompOpSet);
  rot_bitmap_set_compositing_mode(s_hour_layer, GCompOpSet);
  layer_add_child(window_get_root_layer(window), (Layer*)s_minute_layer);
  layer_add_child(window_get_root_layer(window), (Layer*)s_hour_layer);
  layer_set_frame((Layer*)s_minute_layer, GRect(0, 12, 144, 144));
  layer_set_frame((Layer*)s_hour_layer, GRect(0, 12, 144, 144));

}

static void main_window_unload(Window *window) {
  gbitmap_destroy(dial);
  gbitmap_destroy(mickey);
  gbitmap_destroy(h_hand);
  gbitmap_destroy(m_hand);
  bitmap_layer_destroy(s_dial_layer);
  bitmap_layer_destroy(s_mickey_layer);
  layer_destroy(s_date_layer);
  rot_bitmap_layer_destroy(s_hour_layer);
  rot_bitmap_layer_destroy(s_minute_layer);
}

static void init () {
  
  if (persist_exists(KEY_COLOR)) {
    color = persist_read_int(KEY_COLOR);
    APP_LOG(APP_LOG_LEVEL_INFO, "Reading color: %x", color);
  }
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());


  s_main_window = window_create();  
  window_set_background_color(s_main_window, GColorFromHEX(color)); 
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
