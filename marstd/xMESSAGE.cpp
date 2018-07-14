#include "marx.h"
#include <stdarg.h>
#include <stdio.h>

void MARX::message(char* message, ...) {

	char text[1024];
	va_list ap;
	va_start(ap, message);
	vsprintf(text, message, ap);
	va_end(ap);
	
#if 0
	BITMAP* tmp = create_bitmap(SCREEN_W, SCREEN_H);
	blit(screen, tmp, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	
	clear_to_color(screen, makecol(0, 0, 255));
	text_mode(-1);
	textprintf_centre(screen, font, SCREEN_W>>1, SCREEN_H>>1, makecol(255, 255, 255), text);
	textprintf_centre(screen, font, SCREEN_W>>1, (SCREEN_H>>1)+text_height(font)*2, makecol(255, 255, 255), "press any key");

	if (!key[KEY_M])	
		readkey();
	
	blit(tmp, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	destroy_bitmap(tmp);
#endif
	
	FILE* f = fopen("log.txt", "at");
	if (f) {
		fprintf(f, "%s\n", text);
		fclose(f);
	}

}
