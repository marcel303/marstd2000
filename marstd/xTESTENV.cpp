//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file xTESTENV.cpp: MARX test environment. */
//////////////////////////////////////////////////////////////////////

#include <signal.h>
#include <stdarg.h>
#include "marx.h"

/**
 * Initializes a default system with support for keyboard, timers and mouse. A truecolour (32 or 24 BPP) graphics mode of 640x480 pixels is selected.
 * @return True on succes, false otherwise.
 * @see testenv_shutdown()
 */
bool MARX::testenv_initialize() {

	allegro_init();

	// The usual...

        if (install_keyboard() < 0)
        	return false;

	if (install_timer() < 0)
        	return false;

	if (install_mouse() < 0)
        	return false;

	// Set a 640x480 truecolour mode.

	set_color_depth(32);
        if (set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0) < 0) {
		set_color_depth(24);
        	if (set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0) < 0) {
			allegro_message("MARX: testenv: Error: unable to set 640x480x(32/24) graphics mode.");
       			return false;
		}
	}

	return true;

}

/**
 * Closes down the system.
 * @return True on succes, false otherwise.
 */
bool MARX::testenv_shutdown() {

	allegro_exit();
        
	return true;

}
