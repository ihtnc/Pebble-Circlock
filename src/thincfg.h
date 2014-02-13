#include "pebble.h"

#ifndef THINCFG
#define THINCFG

#define SHOW_NONE 0
#define SHOW_DATE 1
#define SHOW_STAT 2
#define SHOW_DATE_STAT 3
	
enum 
{
	CONFIG_KEY_INVERTMODE = 29070,
	CONFIG_KEY_BTNOTIFICATION = 29071,
	CONFIG_KEY_SHOWMODE = 29072,
	CONFIG_KEY_ACTION = 29073
};

typedef void(*ThinCFGFieldChangedHandler)(const uint32_t key, const void *old_value, const void *new_value);

typedef struct {
        ThinCFGFieldChangedHandler field_changed;
} ThinCFGCallbacks;

bool get_invert_mode_value(void);
void set_invert_mode_value(bool value);

bool get_bt_notification_value(void);
void set_bt_notification_value(bool value);

int get_show_mode_value(void);
void set_show_mode_value(int value);

void thincfg_init(void);
void thincfg_deinit(void);
void thincfg_subscribe(ThinCFGCallbacks callback);
void thincfg_unsubscribe(void);
        
#endif
