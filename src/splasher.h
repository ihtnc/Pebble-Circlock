#ifndef SPLASHER
#define SPLASHER
        
#include "pebble.h"

typedef void(*SplasherDefaultHandler)(void);

typedef struct {
        SplasherDefaultHandler splash_init;
        SplasherDefaultHandler splash_deinit;
} SplasherCallbacks;

void splasher_init(SplasherCallbacks callback);
void splasher_deinit(void);
void splasher_start(void);
void splasher_stop(void);
bool splasher_is_splash_showing(void);

#endif
