#ifndef THINCFG
#define THINCFG

#include "pebble.h"

#define SHOW_NONE 0
#define SHOW_DATE 1
#define SHOW_STAT 2
#define SHOW_DATE_STAT 3

#define CONFIG_KEY_SHOWSPLASH 70
#define CONFIG_KEY_INVERTMODE 71
#define CONFIG_KEY_BTNOTIFICATION 72
#define CONFIG_KEY_SHOWMODE 73

typedef void(*ThinCFGFieldChangedHandler)(const uint32_t key, const void *old_value, const void *new_value);

typedef struct {
        ThinCFGFieldChangedHandler field_changed;
} ThinCFGCallbacks;

bool thincfg_get_show_splash_value(void);
bool thincfg_get_invert_mode_value(void);
bool thincfg_get_bt_notification_value(void);
int thincfg_get_show_mode_value(void);

void thincfg_set_show_splash_value(const bool value);
void thincfg_set_invert_mode_value(const bool value);
void thincfg_set_bt_notification_value(const bool value);
void thincfg_set_show_mode_value(const int value);

void thincfg_init(void);
void thincfg_deinit(void);
void thincfg_subscribe(ThinCFGCallbacks callback);
void thincfg_unsubscribe(void);
        
#endif
