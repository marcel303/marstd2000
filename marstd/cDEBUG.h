#ifndef __cDEBUG_h__
#define __cDEBUG_h__

#if defined(MARSTD_DEBUG)

#include <allegro.h>
#include <signal.h>
#include <stdarg.h>

#if defined(ALLEGRO_WIN32)
static bool marstd_exceptionbox(char* text, bool autoabort) {

	allegro_message(text);
	
	return true;

}
#else
static bool marstd_exceptionbox(char* text, bool autoabort) {

	allegro_message(text);
	
	return true;

}
#endif

/**
 * Shows an exception dialog box with a descriptive message. Also gives the user to option to continue execution or abort.
 * @param autoabort If true, the program will be exited when the dialog box is closed. If false, the user may choose to cotinue execution or abort.
 * @param code Return code if exiting program.
 * @param format Like the regular printf().
 * @param ... Variable sized array like printf().
 */
static void marstd_exception(bool autoabort, int code, char* format, ...) {

	char text[1024];
	
	va_list ap;
	va_start(ap, format);
	vsprintf(text, format, ap);
	va_end(ap);
	
	if (marstd_exceptionbox(text, autoabort))
		raise(SIGABRT);
		
}

#endif

#if defined(MARSTD_DEBUG)
#define MASSERT(value, message) \
	{ \
		if (!value) \
			marstd_exception(false, -1, "%s: %s: %d: %s", __FILE__, __FUNCTION__, __LINE__, message); \
	}
#else
#define MASSERT(value, message)
#endif

#endif
