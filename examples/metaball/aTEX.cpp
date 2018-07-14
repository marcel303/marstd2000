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

#include <gl/glu.h>
#include "Atex.h"

#define MAX_TEX		1000

class texture_t {

 public:
 
	texture_t() {
		t = 0;
		bmp = 0;
		used = false;
	} 
	
	~texture_t() {
	}
	
 public:
 
	GLuint t;
	BITMAP* bmp;
	bool used;
	
 public:
 
	void upload() {
	
		if (bmp->h > 1) {
		
			glBindTexture(GL_TEXTURE_2D, t);
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp->w, bmp->h, GL_RGBA, GL_UNSIGNED_BYTE, bmp->line[0]);
		
			#if 1
		
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);				
		
			#else
		
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);						
		
			#endif
			
		} else {
		
			glBindTexture(GL_TEXTURE_1D, t);
			glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, bmp->w, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp->line[0]);
	
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		}			
		
	}    	
	
};

static texture_t tex[MAX_TEX];

static int find(GLuint t) {

	for (int i=0; i<MAX_TEX; i++)
		if (tex[i].used && tex[i].t == t)
			return i;
			
	return -1;			

}

GLuint TEX::load(char* fn) {

	BITMAP* bmp = load_bitmap(fn, 0);
	
	if (!bmp) {
		allegro_message("Error: unable to load texture %s.", fn);
		return 0;
	}		
		
	GLuint t = TEX::add(bmp);
 		
	destroy_bitmap(bmp);		
	
	return t;

}

GLuint TEX::add(BITMAP* bmp) {

	int t = -1;
	
	for (int i=0; i<MAX_TEX && t == -1; i++)
		if (!tex[i].used)
			t = i;
	
	if (t == -1) {
		allegro_message("Error: unable to find free texture slot.");
		return 0;
	}		
	
	tex[t].used = true;
	
	glGenTextures(1, &tex[t].t);
	
  	tex[t].bmp = create_bitmap_ex(32, bmp->w, bmp->h);
	blit(bmp, tex[t].bmp, 0, 0, 0, 0, bmp->w, bmp->h);
	
	tex[t].upload();
	
	return tex[t].t;

}

void TEX::remove(GLuint a_t) {

	int t = find(a_t);
	
	if (t == -1)
		return;

	destroy_bitmap(tex[t].bmp);		
	
	glDeleteTextures(1, &tex[t].t);		
	
	tex[t].used = false;

}

void TEX::display_switch() {

	for (int i=0; i<MAX_TEX; i++)
		if (tex[i].used)
			tex[i].upload();
			
}
