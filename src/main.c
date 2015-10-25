#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer;
static GFont s_time_font, s_date_font;
static BitmapLayer *s_background_layer, *s_bt_disconnected_layer, *s_bt_connected_layer, *s_charging_layer;
static GBitmap *s_background_bitmap, *s_bt_disconnected_bitmap, *s_bt_connected_bitmap, *s_charging_bitmap;
static int s_battery_level;
static Layer *s_battery_layer;

static void battery_callback (BatteryChargeState state) {
	bool charging = state.is_charging;
	s_battery_level = state.charge_percent;
	layer_mark_dirty(s_battery_layer);
	layer_set_hidden(bitmap_layer_get_layer(s_charging_layer), !charging);
}

static void bluetooth_callback (bool connected) {
	layer_set_hidden(bitmap_layer_get_layer(s_bt_disconnected_layer), connected);
	layer_set_hidden(bitmap_layer_get_layer(s_bt_connected_layer), !connected);
	
	if (!connected){
		vibes_double_pulse();
	}
}

static void battery_update_proc (Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);
	int width = (int)(float)(((float)s_battery_level / 100.0F) * 144.0F);
	
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

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
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
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
	
	//battery meter
	s_battery_layer = layer_create(GRect(0, 75, 144, 2));
	layer_set_update_proc(s_battery_layer, battery_update_proc);
	layer_add_child(window_get_root_layer(window), s_battery_layer);
	
	//charging status
	s_charging_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING);
	s_charging_layer = bitmap_layer_create(GRect(100, 90, 30, 30));
	
	bitmap_layer_set_bitmap(s_charging_layer, s_charging_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_charging_layer));
	
	//bluetooth status
	s_bt_disconnected_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_DISCONNECTED);
	s_bt_connected_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_CONNECTED);
	
	s_bt_disconnected_layer = bitmap_layer_create(GRect(59, 90, 30, 30));
	s_bt_connected_layer = bitmap_layer_create(GRect(59, 90, 30, 30));
	
	bitmap_layer_set_bitmap(s_bt_disconnected_layer, s_bt_disconnected_bitmap);
	bitmap_layer_set_bitmap(s_bt_connected_layer, s_bt_connected_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_disconnected_layer));
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_connected_layer));
}

static void main_window_unload (Window *window) {
	text_layer_destroy(s_time_layer);
	fonts_unload_custom_font(s_time_font);
	text_layer_destroy(s_date_layer);
	fonts_unload_custom_font(s_date_font);
	layer_destroy(s_battery_layer);
	gbitmap_destroy(s_background_bitmap);
	bitmap_layer_destroy(s_background_layer);
	gbitmap_destroy(s_bt_connected_bitmap);
	bitmap_layer_destroy(s_bt_connected_layer);
	gbitmap_destroy(s_bt_disconnected_bitmap);
	bitmap_layer_destroy(s_bt_disconnected_layer);
	gbitmap_destroy(s_charging_bitmap);
	bitmap_layer_destroy(s_charging_layer);
}

static void init () {
	s_main_window = window_create();
	
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	window_stack_push(s_main_window, true);
	
	update_time();
	battery_callback(battery_state_service_peek());
	bluetooth_callback(bluetooth_connection_service_peek());
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	battery_state_service_subscribe(battery_callback);
	bluetooth_connection_service_subscribe(bluetooth_callback);
}

static void deinit () {
	window_destroy(s_main_window);
}

int main (void) {
	init();
	app_event_loop();
	deinit();
}