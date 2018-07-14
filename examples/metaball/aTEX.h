//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// OpenGL texture manager.
//////////////////////////////////////////////////////////////////////

#include <allegro.h>
#include <gl/gl.h>

namespace TEX {

	extern GLuint load(char* fn);		// Return 0 on failure.
	extern GLuint add(BITMAP* bmp);		// Adds a bitmap to the list.
	extern void remove(GLuint t);		// Removes a texture and it's system-memory equivalent.
	extern void display_switch();		// Restores all texture on display switch.
	
};	
