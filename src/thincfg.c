#include "thincfg.h"
#include "options.h"

static bool do_init = true;

static bool show_splash;
static bool invert_mode;
static bool bt_notification;
static int show_mode;

static bool old_show_splash;
static bool old_invert_mode;
static bool old_bt_notification;
static int old_show_mode;

static ThinCFGCallbacks cfgcallbacks;

bool get_show_splash_value(void) { return show_splash; }
bool get_invert_mode_value(void) { return invert_mode; }
bool get_bt_notification_value(void) { return bt_notification; }
int get_show_mode_value(void) { return show_mode; }

void set_show_splash_value(const bool value) 
{
	show_splash = value; 
	if(old_show_splash != show_splash && cfgcallbacks.field_changed)
	{
		cfgcallbacks.field_changed(CONFIG_KEY_SHOWSPLASH, (void *)&old_show_splash, (void *)&show_splash);
	}
	old_invert_mode = show_splash;
	
	persist_write_bool(CONFIG_KEY_SHOWSPLASH, value); 
}

void set_invert_mode_value(const bool value) 
{
	invert_mode = value; 
	if(old_invert_mode != invert_mode && cfgcallbacks.field_changed)
	{
		cfgcallbacks.field_changed(CONFIG_KEY_INVERTMODE, (void *)&old_invert_mode, (void *)&invert_mode);
	}
	old_invert_mode = invert_mode;
	
	persist_write_bool(CONFIG_KEY_INVERTMODE, value); 
}

void set_bt_notification_value(const bool value) 
{
	bt_notification = value;
	if(old_bt_notification != bt_notification && cfgcallbacks.field_changed)
    {
        cfgcallbacks.field_changed(CONFIG_KEY_BTNOTIFICATION, (void *)&old_bt_notification, (void *)&bt_notification);
    }
    old_bt_notification = bt_notification;
	
	persist_write_bool(CONFIG_KEY_BTNOTIFICATION, value); 
}

void set_show_mode_value(const int value)
{
	show_mode = value; 
	
	if(old_show_mode != show_mode && cfgcallbacks.field_changed)
	{
		cfgcallbacks.field_changed(CONFIG_KEY_SHOWMODE, (void *)&old_show_mode, (void *)&show_mode);
	}
	old_show_mode = show_mode;
	
	persist_write_int(CONFIG_KEY_SHOWMODE, value); 
}

static void read_config(void) 
{
	if (persist_exists(CONFIG_KEY_SHOWSPLASH)) 
	{
		show_splash = persist_read_bool(CONFIG_KEY_SHOWSPLASH);
		
		#ifdef ENABLE_LOGGING
		if(show_splash == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: show_splash=true");
		else APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: show_splash=false");		
		#endif
	}
	else
	{
		show_splash = true; //default value
		
		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: show_splash not configured. default=true");
		#endif
	}
	
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
		show_mode = 1; //default value
		
		#ifdef ENABLE_LOGGING
			APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: show_mode not configured. default=0");
		#endif
	}
	
	old_show_splash = show_splash;
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
		invert_mode = (imode->value->int32 == 1);
		
		#ifdef ENABLE_LOGGING
		if(invert_mode == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: invert_mode=true");
		else APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: invert_mode=false");
		#endif
	}
	else
	{
		//since thinCFG won't pass fields that are not selected, we set the invert_mode to false if its key is not returned
		invert_mode = false;
		
		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: invert_mode=false");
		#endif
	}
	set_invert_mode_value(invert_mode);
	dict_read_first(received);
		
	Tuple *bt = dict_find(received, CONFIG_KEY_BTNOTIFICATION);
	if(bt != NULL) 
	{
		bt_notification = (bt->value->int32 == 1);
		
		#ifdef ENABLE_LOGGING
		if(bt_notification == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: bt_notification=true");
		else APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: bt_notification=false");
		#endif
	}
	else
	{
		//since thinCFG won't pass fields that are not selected, we set the bt_notification to false if its key is not returned
		bt_notification = false;
		
		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: bt_notification=false");
		#endif
    }
    set_bt_notification_value(bt_notification);
	dict_read_first(received);
	
	Tuple *smode = dict_find(received, CONFIG_KEY_SHOWMODE);
	if(smode != NULL) 
	{
		show_mode = (int)smode->value->int32;
		
		#ifdef ENABLE_LOGGING
		char *output = "in_received_handler: show_mode=XXX";
		snprintf(output, strlen(output), "in_received_handler: show_mode=%d", (int)smode->value->int32);
		APP_LOG(APP_LOG_LEVEL_DEBUG, output);
		#endif
	}
	else
	{
		//since thinCFG won't pass fields that are not selected, we set the show_mode to 0 if its key is not returned
		show_mode = 0;
		
		#ifdef ENABLE_LOGGING
			APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: show_mode=0");
		#endif
    }
	set_show_mode_value(show_mode);
	dict_read_first(received);	
}

void thincfg_unsubscribe(void)
{
    cfgcallbacks.field_changed = NULL;
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
	if(do_init == true)
	{
		app_message_init();
		read_config();
		do_init = false;
	}
}

void thincfg_deinit(void)
{
    app_message_deregister_callbacks();
}
