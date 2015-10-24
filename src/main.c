#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer;
static GFont s_time_font, s_date_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_time () {
	//update time
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	static char buffer[] = "00:00";
	
	if (clock_is_24h_style() == true){
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	}else{
		strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
	}
	
	text_layer_set_text(s_time_layer, buffer);
	
	//update date
	static char date_buffer[16];
	strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
	text_layer_set_text(s_date_layer, date_buffer);
}

static void tick_handler (struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

static void main_window_load (Window *window) {
	//background image
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_IMAGE);
	s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
	
	//time text
	s_time_layer = text_layer_create(GRect(2, 0, 144, 50));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorBlack);
	text_layer_set_text(s_time_layer, "00:00");
	
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_DOS_48));
	text_layer_set_font(s_time_layer, s_time_font);
	
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	
	//date text
	s_date_layer = text_layer_create(GRect(2, 120, 144, 30));
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorWhite);
	
	s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_DOS_24));
	text_layer_set_font(s_date_layer, s_date_font);
	
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
}

static void main_window_unload (Window *window) {
	text_layer_destroy(s_time_layer);
	fonts_unload_custom_font(s_time_font);
	text_layer_destroy(s_date_layer);
	fonts_unload_custom_font(s_date_font);
	gbitmap_destroy(s_background_bitmap);
	bitmap_layer_destroy(s_background_layer);
}

static void init () {
	s_main_window = window_create();
	
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	window_stack_push(s_main_window, true);
	
	update_time();
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit () {
	window_destroy(s_main_window);
}

int main (void) {
	init();
	app_event_loop();
	deinit();
}