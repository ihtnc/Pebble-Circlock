#include "thincfg.h"
#include "options.h"

static bool invert_mode;
static bool bt_notification;
static int show_mode;

static bool old_invert_mode;
static bool old_bt_notification;
static int old_show_mode;

static ThinCFGCallbacks cfgcallbacks;

bool get_invert_mode_value(void) { return invert_mode; }
bool get_bt_notification_value(void) { return bt_notification; }
int get_show_mode_value(void) { return show_mode; }

static void read_config(void) 
{
	if (persist_exists(CONFIG_KEY_INVERTMODE)) 
	{
		invert_mode = persist_read_bool(CONFIG_KEY_INVERTMODE);
		
		#ifdef ENABLE_LOGGING
		if(invert_mode == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: invert_mode=true");
		else APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: invert_mode=false");		
		#endif
	}
	else
	{
		invert_mode = false; //default value
		
		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: invert_mode not configured. default=false");
		#endif
	}
	
	if (persist_exists(CONFIG_KEY_BTNOTIFICATION)) 
	{
		bool bt = persist_read_bool(CONFIG_KEY_BTNOTIFICATION);
		bt_notification = bt;
		
		#ifdef ENABLE_LOGGING
		if(bt == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: bt_notification=true");
		else APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: bt_notification=false");
		#endif
	} 
	else
	{
		bt_notification = true; //default value
		
		#ifdef ENABLE_LOGGING
			APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: bt_notification not configured. default=true");
		#endif
	}
	
	if (persist_exists(CONFIG_KEY_SHOWMODE)) 
	{
		show_mode = persist_read_int(CONFIG_KEY_SHOWMODE);
		
		#ifdef ENABLE_LOGGING
		char *output = "read_config: show_mode=XXX";
		snprintf(output, strlen(output), "read_config: show_mode=%d", show_mode);
		APP_LOG(APP_LOG_LEVEL_DEBUG, output);
		#endif
	}
	else
	{
		show_mode = 0; //default value
		
		#ifdef ENABLE_LOGGING
			APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: show_mode not configured. default=0");
		#endif
	}
	
	old_bt_notification = bt_notification;
	old_invert_mode = invert_mode;
	old_show_mode = show_mode;
}

static void in_dropped_handler(AppMessageResult reason, void *context) 
{
	vibes_long_pulse();
	
	#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "in_dropped_handler: done");
	#endif
}

static void in_received_handler(DictionaryIterator *received, void *context) 
{
	Tuple *imode = dict_find(received, CONFIG_KEY_INVERTMODE);
	if(imode != NULL) 
	{
		persist_write_bool(CONFIG_KEY_INVERTMODE, (imode->value->int32 == 1));
		invert_mode = (imode->value->int32 == 1);
		
		#ifdef ENABLE_LOGGING
		if(invert_mode == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: invert_mode=true");
		else APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: invert_mode=false");
		#endif
	}
	else
	{
		//since thinCFG won't pass fields that are not selected, we set the invert_mode to false if its key is not returned
		persist_write_bool(CONFIG_KEY_INVERTMODE, false);
		invert_mode = false;
		
		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: invert_mode=false");
		#endif
	}
	dict_read_first(received);
	
	if(old_invert_mode != invert_mode && cfgcallbacks.field_changed)
	{
		cfgcallbacks.field_changed(CONFIG_KEY_INVERTMODE, (void *)&old_invert_mode, (void *)&invert_mode);
	}
	old_invert_mode = invert_mode;
	
	Tuple *bt = dict_find(received, CONFIG_KEY_BTNOTIFICATION);
	if(bt != NULL) 
	{
		persist_write_bool(CONFIG_KEY_BTNOTIFICATION, (bt->value->int32 == 1));
		bt_notification = (bt->value->int32 == 1);
		
		#ifdef ENABLE_LOGGING
		if(bt_notification == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: bt_notification=true");
		else APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: bt_notification=false");
		#endif
	}
	else
	{
		//since thinCFG won't pass fields that are not selected, we set the bt_notification to false if its key is not returned
		persist_write_bool(CONFIG_KEY_BTNOTIFICATION, false);
		bt_notification = false;
		
		#ifdef ENABLE_LOGGING
			APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: bt_notification=false");
		#endif
    }
	dict_read_first(received);
	
    if(old_bt_notification != bt_notification && cfgcallbacks.field_changed)
    {
        cfgcallbacks.field_changed(CONFIG_KEY_BTNOTIFICATION, (void *)&old_bt_notification, (void *)&bt_notification);
    }
    old_bt_notification = bt_notification;
	
	Tuple *smode = dict_find(received, CONFIG_KEY_SHOWMODE);
	if(smode != NULL) 
	{
		persist_write_int(CONFIG_KEY_SHOWMODE, (int)smode->value->int32);
		show_mode = (int)smode->value->int32;
		
		#ifdef ENABLE_LOGGING
		char *output = "in_received_handler: show_mode=XXX";
		snprintf(output, strlen(output), "in_received_handler: show_mode=%d", (int)smode->value->int32);
		APP_LOG(APP_LOG_LEVEL_DEBUG, output);
		#endif
	}
	dict_read_first(received);
	
	if(old_show_mode != show_mode && cfgcallbacks.field_changed)
	{
		cfgcallbacks.field_changed(CONFIG_KEY_SHOWMODE, (void *)&old_show_mode, (void *)&show_mode);
	}
	old_show_mode = show_mode;
}

void thincfg_unsubscribe(void)
{
    //
}

void thincfg_subscribe(ThinCFGCallbacks callbacks)
{
    cfgcallbacks = callbacks;
}

static void app_message_init(void) 
{
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

void thincfg_init(void) 
{
    app_message_init();
    read_config();
}

void thincfg_deinit(void)
{
    app_message_deregister_callbacks();
}
