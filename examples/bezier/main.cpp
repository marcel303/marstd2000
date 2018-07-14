//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CBezier3 patch example.
//////////////////////////////////////////////////////////////////////

#include <alleggl.h>
#include <gl/glu.h>
#include "marstd.h"

int main(int argc, char* argv[]) {

	allegro_init();
	
//--------------------------------------------------------------------
// Initalize system.
	
	if (install_timer() < 0 || install_keyboard() < 0 || install_mouse() < 0) {
		allegro_message("Error: unable to install system drivers.");
		return -1;
	}
	
//--------------------------------------------------------------------
// We will be using hardware accelerated drawing through OpenGL!
	
	install_allegro_gl();
	
	int bpp = desktop_color_depth();
	allegro_gl_set(AGL_COLOR_DEPTH, bpp?bpp:32);
	allegro_gl_set(AGL_DOUBLEBUFFER, 1);
	allegro_gl_set(AGL_Z_DEPTH, 24);
	allegro_gl_set(AGL_SUGGEST, AGL_COLOR_DEPTH | AGL_DOUBLEBUFFER | AGL_Z_DEPTH);
	
	if (set_gfx_mode(GFX_OPENGL_FULLSCREEN, 800, 600, 0, 0) < 0) {
		allegro_message("Error: unable to set OpenGL graphics mode.");
		return -1;
	}
	
//--------------------------------------------------------------------
// Initialize initial bezier corner points and velocities.
	
	CBezier3 bezier;		// This is our bezier patch object.
	CVector p[4][4], v[4][4];	// Position and velocities of the four corner points.
	
	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++) {
			bezier.p[i][j][0] = (i/3.0-0.5)*2.0;
			bezier.p[i][j][1] = (j/3.0-0.5)*2.0;
			bezier.p[i][j][2] = 0.0;   			
		}
  			
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {	

			v[i][j][2] = ((rand()&255)/255.0+0.5)/1000.0;
   			if (i > 0 && i < 3 && j > 0 && j < 3) {	
				v[i][j][0] = ((rand()&255)/255.0+0.5)/1000.0;
				v[i][j][1] = ((rand()&255)/255.0+0.5)/1000.0;  		
			}
			
			p[i][j] = bezier.p[i][j];
		
		}
	}
 
//--------------------------------------------------------------------
// Main loop.
	
	while (!key[KEY_ESC]) {
	
   		static float rx = 0.0;		
	   	static int resolution = 50;	
	
//--------------------------------------------------------------------
// Input.
		
		// Keyboard input.
		
		while (keypressed()) {
			int c = readkey()>>8;
			if (c == KEY_A)
				resolution++;
			else if (c == KEY_Z)
   				resolution--;				
		}
		
		// Mouse input.
		
		int mx, my;
		get_mouse_mickeys(&mx, &my);
		rx -= my;
		
//--------------------------------------------------------------------
// Update / logic.

		// Move the four corner points of bezier patch.
		
		#if 1
		
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {		
		
				CVector pnew = p[i][j] + v[i][j];
			
				if (pnew[0] < -1.0 || pnew[0] > +1.0)
					v[i][j][0] = -v[i][j][0];
				if (pnew[1] < -1.0 || pnew[1] > +1.0)
					v[i][j][1] = -v[i][j][1];				
				if (pnew[2] < -1.0 || pnew[2] > +1.0)
					v[i][j][2] = -v[i][j][2];				
				
				p[i][j] += v[i][j];				
			
				bezier.p[i][j] = p[i][j];
			
			}
			
		}
		
		#endif
	
		// Generate the bezier patch.
		
  		bezier.generate(resolution);	
  		
//--------------------------------------------------------------------  		
// Rendering.
  		
		// Setup matrices.

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		gluPerspective(90.0, SCREEN_W/(float)SCREEN_H, 0.01, 100.0);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		// Prepare buffers.
		
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		
		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);		
		
		// Setup and enable lighting.

		GLfloat l_direction[2][4] =
			{		
				{ 0.0, 0.0, +1.0, 0.0 },
				{ 0.0, 0.0, -1.0, 0.0 }
			};
			
		GLfloat l_diffuse[2][4] =
			{		
				{ 2.0, 2.0, 0.0, 1.0 },
				{ 2.0, 0.0, 2.0, 1.0 }
			};
			
		for (int i=0; i<2; i++) {   				
			glLightfv(GL_LIGHT1+i, GL_POSITION, l_direction[i]);		
			glLightfv(GL_LIGHT1+i, GL_DIFFUSE, l_diffuse[i]);
			glEnable(GL_LIGHT1+i);
		}
  			
		glEnable(GL_LIGHTING);
  		
    		glEnable(GL_FOG);
      		
		// Camera transformation.

		glTranslatef(0.0, 0.0, -2.0);
		glRotatef(rx, 1.0, 0.0, 0.0);
    		
  		// Render triangles.
  		
		// FIXME: It would be better to output using a triangle strip.

		if (key[KEY_L])
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			
		glBegin(GL_TRIANGLES); {
          		
  		for (int i=0; i<bezier.resolution-1; i++) {

	  		for (int j=0; j<bezier.resolution-1; j++) {
	  		
	  			// Each quad must be split into 2 triangles since a quad would not be planar.
	  			
	  			CVector p1 = bezier.v[i+0][j+0];
	  			CVector p2 = bezier.v[i+1][j+0];
	  			CVector p3 = bezier.v[i+0][j+1];
	  			
	  			CVector d1 = p2-p1;
	  			CVector d2 = p3-p2;
	  			CVector n = d1 % d2;
	  			n.normalize();
            	  			
				glNormal3fv(n);
				glVertex3fv(p1);
				glVertex3fv(p2);
				glVertex3fv(p3);
				
	  			p1 = bezier.v[i+1][j+1];

				// These are the actual values. We negate normal to compensate *-1 orientation.

	  			// p1 = bezier.v[i+1][j+0];
	  			// p2 = bezier.v[i+1][j+1];
	  			// p3 = bezier.v[i+0][j+1];	  			

	  			d1 = p2-p1;
	  			d2 = p3-p2;
	  			n = - (d1 % d2);
	  			n.normalize();

				glNormal3fv(n);
				glVertex3fv(p1);
				glVertex3fv(p2);
				glVertex3fv(p3);

			}

		} 

		} glEnd();		

//--------------------------------------------------------------------
// Make back buffer visible.

  		allegro_gl_flip();

	}

//--------------------------------------------------------------------
// Shutdown system.

	remove_allegro_gl();
	allegro_exit();
	
	return 0;	

} END_OF_MAIN();
