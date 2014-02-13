/*
  Inspired by the Polar Clock Lite watch (SDK 2.0) from https://github.com/op12/PolarClockLite2.0
 */

#include "main.h"
#include "btmonitor.h"
#include "thincfg.h"
#include "options.h"

static char date_text[] = "XXX 00";
static char status_text[] = "X X";

Window *window;
InverterLayer *inverter;

Layer *minute_display_layer;
Layer *hour_display_layer;

TextLayer *text_status_layer;
TextLayer *text_date_layer;

static GFont font;
static GFont sym_font;
static bool is_stat_showing;

static GPath *minute_segment_path;
const GPathInfo MINUTE_SEGMENT_PATH_POINTS = 
{
	3,
	(GPoint []) 
	{
		{0, 0},
		{-7, -70}, // 70 = radius + fudge; 7 = 70*tan(6 degrees); 6 degrees per minute;
		{7,  -70},
	}
};

static GPath *hour_segment_path;
const GPathInfo HOUR_SEGMENT_PATH_POINTS = 
{
	3,
	(GPoint []) 
	{
		{0, 0},
		{-6, -58}, // 58 = radius + fudge; 6 = 58*tan(6 degrees); 30 degrees per hour;
		{6,  -58},
	}
};

static AppTimer *timer;

static void determine_invert_status()
{
	bool invert = get_invert_mode_value();
	layer_set_frame(inverter_layer_get_layer(inverter), GRect(0, 0, SCREEN_WIDTH, (invert ? SCREEN_HEIGHT : 0)));
	layer_mark_dirty(inverter_layer_get_layer(inverter));
}

static void minute_display_layer_update_callback(Layer *layer, GContext* ctx) 
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	unsigned int angle = t->tm_min * 6;
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);
	
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, center, 65);
	
	graphics_context_set_fill_color(ctx, GColorBlack);
	for(; angle < 355; angle += 6) 
	{
		gpath_rotate_to(minute_segment_path, (TRIG_MAX_ANGLE / 360) * angle);
		gpath_draw_filled(ctx, minute_segment_path);
	}
	
	graphics_fill_circle(ctx, center, 60);
}

static void layer_set_status(TextLayer *layer)
{
	bool bt = bluetooth_connection_service_peek();
	BatteryChargeState batt = battery_state_service_peek();
	
	char sym_bt;
	if(bt) sym_bt = '`';
	else sym_bt = 'a';
	
	char sym_batt;
	if(batt.is_charging) sym_batt = '+';
	else if(batt.charge_percent <= 25) sym_batt = '?';
	else if(batt.charge_percent <= 50) sym_batt = '&';
	else if(batt.charge_percent <= 75) sym_batt = '%';
	else if(batt.charge_percent <= 100) sym_batt = '$';
	else sym_batt = 'a';
	
	snprintf(status_text, sizeof(status_text), "%c %c", sym_batt, sym_bt);
	text_layer_set_text(layer, status_text);
	text_layer_set_font(layer, sym_font);
}

static void layer_set_date(TextLayer *layer)
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(date_text, sizeof(date_text), "%a %d", t);
	text_layer_set_text(layer, date_text);
	text_layer_set_font(layer, font);
}

static void show_text(struct tm *tick_time)
{ 
	int mode = get_show_mode_value();
	if (mode == SHOW_DATE || mode == SHOW_DATE_STAT)
	{
		if(is_stat_showing == true) layer_set_status(text_date_layer);
		else layer_set_date(text_date_layer);
	}
	
	if (mode == SHOW_STAT || mode == SHOW_DATE_STAT)
	{
		if(is_stat_showing == true) layer_set_date(text_status_layer);
		else layer_set_status(text_status_layer);
	}
	
	if(mode == SHOW_NONE)
	{	
		layer_set_hidden(text_layer_get_layer(text_status_layer), (is_stat_showing == false));
		layer_set_hidden(text_layer_get_layer(text_date_layer), (is_stat_showing == false));
		
		if(is_stat_showing == true)
		{
			layer_set_date(text_date_layer);
			layer_set_status(text_status_layer);
		}
	}
}

static void handle_timer(void *data)
{
	timer = NULL;
	is_stat_showing = false;
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	show_text(t);
}

static void handle_tap(AccelAxisType axis, int32_t direction)
{
	int mode = get_show_mode_value();
	if(mode == SHOW_DATE_STAT) return; //the date and status are already shown so tapping doesn't do anything
	
	if(is_stat_showing == true) return;
	is_stat_showing = true;
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	show_text(t);
	
	timer = app_timer_register(1500, handle_timer, NULL);
}

static void hour_display_layer_update_callback(Layer *layer, GContext* ctx) 
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	bool am = (t->tm_hour < 12);
	
	unsigned int angle = (( t->tm_hour % 12 ) * 30) + (t->tm_min / 2);
	angle = angle - (angle % 6);
	
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);
	
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, center, 55);
	graphics_context_set_fill_color(ctx, GColorBlack);
	
	gpath_move_to(hour_segment_path, grect_center_point(&bounds));
	if(am)
	{
		for(; angle < 355; angle += 6)
		{
			gpath_rotate_to(hour_segment_path, (TRIG_MAX_ANGLE / 360) * angle);
			gpath_draw_filled(ctx, hour_segment_path);
		}
	}
	else
	{
		unsigned int other_angle;
		for(other_angle = 0; other_angle < angle; other_angle += 6)
		{
			gpath_rotate_to(hour_segment_path, (TRIG_MAX_ANGLE / 360) * other_angle);
			gpath_draw_filled(ctx, hour_segment_path);
		}
	}
	
	graphics_fill_circle(ctx, center, 50);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
	determine_invert_status();
	
	layer_mark_dirty(minute_display_layer);
	layer_mark_dirty(hour_display_layer);
	layer_mark_dirty(inverter_layer_get_layer(inverter));
	
	if(is_stat_showing == false) show_text(tick_time);
}

static void setup_layers() 
{
	Layer *window_layer = window_get_root_layer(window);
	Layer *inverter_layer = inverter_layer_get_layer(inverter);
	GRect bounds = layer_get_bounds(window_layer);
	
	int mode = get_show_mode_value();
	if(text_date_layer != NULL) text_layer_destroy(text_date_layer);
	if(text_status_layer != NULL) text_layer_destroy(text_status_layer);
	
	if (mode == SHOW_DATE || mode == SHOW_DATE_STAT || mode == SHOW_NONE)
	{
		text_date_layer = text_layer_create(bounds);
		text_layer_set_text_color(text_date_layer, GColorWhite);
		text_layer_set_background_color(text_date_layer, GColorClear);
		text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);
		
		text_layer_set_font(text_date_layer, font);
		layer_insert_below_sibling(text_layer_get_layer(text_date_layer), inverter_layer);
		
		if (mode == SHOW_DATE_STAT || mode == SHOW_NONE) 
			layer_set_frame(text_layer_get_layer(text_date_layer), GRect(0, 57, 144, 168-57));
		else 
			layer_set_frame(text_layer_get_layer(text_date_layer), GRect(0, 70, 144, 168-70));	  
	}
	
	if (mode == SHOW_STAT || mode == SHOW_DATE_STAT || mode == SHOW_NONE)
	{
		text_status_layer = text_layer_create(bounds);
		text_layer_set_text_color(text_status_layer, GColorWhite);
		text_layer_set_background_color(text_status_layer, GColorClear);
		text_layer_set_text_alignment(text_status_layer, GTextAlignmentCenter);
		
		text_layer_set_font(text_status_layer, font);
		layer_insert_below_sibling(text_layer_get_layer(text_status_layer), inverter_layer);
		
		if (mode == SHOW_DATE_STAT || mode == SHOW_NONE) 
			layer_set_frame(text_layer_get_layer(text_status_layer), GRect(0, 80, 144, 168-80));
		else 
			layer_set_frame(text_layer_get_layer(text_status_layer), GRect(0, 70, 144, 168-70));
	}
}

static void field_changed(const uint32_t key, const void *old_value, const void *new_value)
{
	#ifdef ENABLE_LOGGING
	char *output = "field_changed: key=XXX";
	snprintf(output, strlen(output), "field_changed: key=%d", (int) key);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
	
	if(key == CONFIG_KEY_INVERTMODE)
	{
		determine_invert_status();
	}
	else if(key == CONFIG_KEY_SHOWMODE)
	{
		time_t now = time(NULL);
		struct tm *t = localtime(&now);
		setup_layers();
		show_text(t);
	}
}

static void face_init()
{
	Layer *window_layer = window_get_root_layer(window);
	Layer *inverter_layer = inverter_layer_get_layer(inverter);
	GRect bounds = layer_get_bounds(window_layer);
	
	// Init the layer for the minute display
	minute_display_layer = layer_create(bounds);
	layer_set_update_proc(minute_display_layer, minute_display_layer_update_callback);
	layer_insert_below_sibling(minute_display_layer, inverter_layer);
	
	// Init the minute segment path
	minute_segment_path = gpath_create(&MINUTE_SEGMENT_PATH_POINTS);
	gpath_move_to(minute_segment_path, grect_center_point(&bounds));
	
	// Init the layer for the hour display
	hour_display_layer = layer_create(bounds);
	layer_set_update_proc(hour_display_layer, hour_display_layer_update_callback);
	layer_insert_below_sibling(hour_display_layer, inverter_layer);
	
	// Init the hour segment path
	hour_segment_path = gpath_create(&HOUR_SEGMENT_PATH_POINTS);
	gpath_move_to(hour_segment_path, grect_center_point(&bounds));
	
	setup_layers();
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	show_text(t);
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	accel_tap_service_subscribe(handle_tap);
}

static void inverter_init()
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	inverter = inverter_layer_create(GRect(0, 0, SCREEN_WIDTH, 0));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(inverter));
	determine_invert_status(t);
}

static void init(void) 
{	
	is_stat_showing = false;
	font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_20));
	sym_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SYMBOLS_25));
	
	window = window_create();
	window_set_background_color(window, GColorBlack);
	window_stack_push(window, true);
	
	thincfg_init();
    thincfg_subscribe((ThinCFGCallbacks) { .field_changed = field_changed, });
	
	inverter_init();
	btmonitor_init();
	
	face_init();
}

static void deinit(void)
{
	gpath_destroy(minute_segment_path);
	gpath_destroy(hour_segment_path);
	
	tick_timer_service_unsubscribe();
	
	layer_remove_from_parent(inverter_layer_get_layer(inverter));
	inverter_layer_destroy(inverter);
	free(inverter);
	
	window_destroy(window);
	layer_destroy(minute_display_layer);
	layer_destroy(hour_display_layer);
	
	if(text_status_layer != NULL) text_layer_destroy(text_status_layer);
	if(text_date_layer != NULL) text_layer_destroy(text_date_layer);
	
	fonts_unload_custom_font(font);
	fonts_unload_custom_font(sym_font);
	
	accel_data_service_unsubscribe();
}

int main(void)
{
	init();
	app_event_loop();
	deinit();
}
