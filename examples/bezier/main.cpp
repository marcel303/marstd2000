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

#include "marstd.h"
#include "framework.h"

#define SOFTWARE_LIGHTING 1

int main(int argc, char* argv[]) {

//--------------------------------------------------------------------
// Initalize system.
	
	setupPaths(CHIBI_RESOURCE_PATHS);
	
	framework.enableDepthBuffer = true;
	
	if (!framework.init(800, 600)) {
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
	
	while (!keyboard.wentDown(SDLK_ESCAPE)) {
	
		framework.process();
		
   		static float rx = 0.0;		
	   	static int resolution = 50;	
	
//--------------------------------------------------------------------
// Input.
		
		// Keyboard input.
		
		if (keyboard.wentDown(SDLK_a))
			resolution++;
		else if (keyboard.wentDown(SDLK_z))
			resolution--;
		
		// Mouse input.
		
		rx -= mouse.dy;
		
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

		framework.beginDraw(0, 0, 0, 0);
  		
		// Setup matrices.

		projectPerspective3d(90.f, .01f, 100.f);
		gxScalef(1, 1, -1);
		
		// Prepare buffers.
		
		setDepthTest(true, DEPTH_LESS);
		
		// Setup and enable lighting.

		float l_direction[2][4] =
			{		
				{ 0.0, 0.0, +1.0, 0.0 },
				{ 0.0, 0.0, -1.0, 0.0 }
			};
			
		float l_diffuse[2][4] =
			{		
				{ 2.0, 2.0, 0.0, 1.0 },
				{ 2.0, 0.0, 2.0, 1.0 }
			};
		
	#if SOFTWARE_LIGHTING
		auto computeLightColor = [&] (CVector & n) -> Vec3
		{
			const Vec4 n_world = transformToWorld(Vec4(n[0], n[1], n[2], 0.f));

			Vec3 color;

			for (int i = 0; i < 2; ++i)
			{
				const float intensity =
					l_direction[i][0] * n_world[0] +
					l_direction[i][1] * n_world[1] +
					l_direction[i][2] * n_world[2];
				
				if (intensity > 0.f)
				{
					color[0] += l_diffuse[i][0] * intensity;
					color[1] += l_diffuse[i][1] * intensity;
					color[2] += l_diffuse[i][2] * intensity;
				}
			}

			return color;
		};
	#else
		for (int i=0; i<2; i++) {   				
			glLightfv(GL_LIGHT1+i, GL_POSITION, l_direction[i]);		
			glLightfv(GL_LIGHT1+i, GL_DIFFUSE, l_diffuse[i]);
			glEnable(GL_LIGHT1+i);
		}
		
		glEnable(GL_LIGHTING);
		
		glEnable(GL_FOG);
	#endif
      		
		// Camera transformation.

		gxTranslatef(0.0, 0.0, -2.0);
		gxRotatef(rx, 1.0, 0.0, 0.0);
    		
  		// Render triangles.
  		
		// FIXME: It would be better to output using a triangle strip.

		pushWireframe(keyboard.isDown(SDLK_l));
		
		gxBegin(GX_TRIANGLES); {
          		
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
				
			#if SOFTWARE_LIGHTING
				const Vec3 color1 = computeLightColor(n);
				
				gxColor4f(color1[0], color1[1], color1[2], 1.f);
			#else
				gxNormal3fv(n);
			#endif
				gxVertex3fv(p1);
				gxVertex3fv(p2);
				gxVertex3fv(p3);
				
	  			p1 = bezier.v[i+1][j+1];

				// These are the actual values. We negate normal to compensate *-1 orientation.

	  			// p1 = bezier.v[i+1][j+0];
	  			// p2 = bezier.v[i+1][j+1];
	  			// p3 = bezier.v[i+0][j+1];	  			

	  			d1 = p2-p1;
	  			d2 = p3-p2;
	  			n = - (d1 % d2);
	  			n.normalize();
				
			#if SOFTWARE_LIGHTING
				const Vec3 color2 = computeLightColor(n);
				
				gxColor4f(color2[0], color2[1], color2[2], 1.f);
			#else
				gxNormal3fv(n);
			#endif
				gxVertex3fv(p1);
				gxVertex3fv(p2);
				gxVertex3fv(p3);

			}

		} 

		} gxEnd();
		
		popWireframe();

//--------------------------------------------------------------------
// Make back buffer visible.

  		framework.endDraw();

	}

//--------------------------------------------------------------------
// Shutdown system.

	framework.shutdown();
	
	return 0;	

}
