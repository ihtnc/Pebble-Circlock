#include "splasher.h"
#include "splasher.h"
#include "thincfg.h"
#include "options.h"

static bool is_showing;
static SplasherCallbacks splashercallbacks;

bool splasher_is_splash_showing() { return is_showing; }

void splasher_start(void)
{
	bool needs_show = thincfg_get_show_splash_value();
	
	#ifdef ENABLE_LOGGING
	if(needs_show) APP_LOG(APP_LOG_LEVEL_DEBUG, "splasher_start: needs_show=true");
	else APP_LOG(APP_LOG_LEVEL_DEBUG, "splasher_start: needs_show=false");
	#endif
	
	if(needs_show == true)
	{
		is_showing = true;
		
		if(splashercallbacks.splash_init)
		{
			splashercallbacks.splash_init();
		}
		else
		{
			APP_LOG(APP_LOG_LEVEL_DEBUG, "splasher_start: SplasherCallbacks.splash_init not registered");
		}
	}
	else
	{
		//since splash won't be shown anymore, call the splasher_stop right away
		splasher_stop();
	}    
}

void splasher_stop(void)
{
	is_showing = false;
	
	if(splashercallbacks.splash_deinit)
	{
		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "splasher_stop: setting is_showing=false");
		#endif
		
		thincfg_set_show_splash_value(false);
		splashercallbacks.splash_deinit();
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "splasher_stop: SplasherCallbacks.splash_deinit not registered");
	}
}    

void splasher_init(SplasherCallbacks callbacks) 
{              
	is_showing = false;  
	splashercallbacks = callbacks;
}

void splasher_deinit(void) 
{
	splashercallbacks.splash_init = NULL;
	splashercallbacks.splash_deinit = NULL;
}
