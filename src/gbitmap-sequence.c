#include <pebble.h>

static Window *s_main_window;

static GBitmap *s_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static TextLayer *s_text_layer, *s_top_left;
static GBitmapSequence *s_sequence = NULL;

static void load_sequence();

char time_buffer[8];

static void timer_handler(void *context) {
  uint32_t next_delay;

  // Advance to the next APNG frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
    bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));

    // Timer for that delay
    app_timer_register(next_delay, timer_handler, NULL);
  } else {
    // Start again
    load_sequence();
  }
}

static void load_sequence() {
  // Free old data
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }

  // Create sequence
  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_TOAD_OPT);

  // Create GBitmap
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);

  // Begin animation
  app_timer_register(1, timer_handler, NULL);
}

void minute_tick(struct tm *tick_time, TimeUnits units){
	if(clock_is_24h_style()) strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
	else strftime(time_buffer, sizeof(time_buffer), "%I:%M", tick_time);
	text_layer_set_text(s_text_layer, time_buffer);
	text_layer_set_text(s_top_left, time_buffer);
}

static void init_text_layer(TextLayer *text_layer){
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text(text_layer, time_buffer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  s_text_layer = text_layer_create(GRect(0,0,144,42));
  init_text_layer(s_text_layer);
  text_layer_set_text_color(s_text_layer, GColorSpringBud);	

  s_top_left = text_layer_create(GRect(-2,-2,144,42));
  init_text_layer(s_top_left);
  text_layer_set_text_color(s_top_left, GColorKellyGreen);
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	minute_tick(t, MINUTE_UNIT);	
	
  layer_add_child(window_layer, text_layer_get_layer(s_top_left));
	
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
	
  s_bitmap_layer = bitmap_layer_create(GRect(0,42,144,126));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

  load_sequence();
	
	tick_timer_service_subscribe(MINUTE_UNIT, minute_tick);
}

static void main_window_unload(Window *window) {
  bitmap_layer_destroy(s_bitmap_layer);
	gbitmap_sequence_destroy(s_sequence);
	text_layer_destroy(s_text_layer);
	text_layer_destroy(s_top_left);
	tick_timer_service_unsubscribe();
}

static void init() {
	
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  //window_set_fullscreen(s_main_window, true);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
