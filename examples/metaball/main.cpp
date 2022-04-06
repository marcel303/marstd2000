//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Metaballs example using CIsosurface class.
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include "data/engine/ShaderCommon.txt"
#include "framework.h"
#include "gx_mesh.h"
#include "marstd.h"
#include <SDL2/SDL.h>

//////////////////////////////////////////////////////////////////////
// Could also use isosurfaces to do csg add, substract, xor, etc like in some raytracers.
// simple implementation requires a high resolution though.
//////////////////////////////////////////////////////////////////////

static void draw_triangles(int num, CIsosurfaceVertex* v);

int main(int argc, char* argv[]) {

	setupPaths(CHIBI_RESOURCE_PATHS);
	
//--------------------------------------------------------------------
// Initialize system.
	
	SDL_ShowSimpleMessageBox(
		SDL_MESSAGEBOX_INFORMATION,
		"MarSTD - CIsosurface & CIsosurfaceMetaball classes example.",
		"Press L to render as wireframe.\nPress I to invert metaball energy values.\nPress mouse button 1 & 2 to change treshold.\n------\nLogic execution rate is currently equal to the number of frames per second it renders.\nIf the example appears to execute too fast, enable vsync in your display properties for OpenGL applications.\n------\nMarcel Smit, 2003.", nullptr);
	
//--------------------------------------------------------------------
// Open a window.

	framework.enableDepthBuffer = true;
	framework.enableRealTimeEditing = true;
	
	if (!framework.init(800, 600)) {
		return -1;
	}
	
	// Field with medium resolution.
 	
	CIsosurface field; // This is our CIsosurface object.
	field.set_size(60, 60, 60);

	// Four metaballs.
	
	const int balls = 4;
	CIsosurfaceMetaball metaball[balls];
	
	// Textures.
	
	GxTextureId envmap = getTexture("envmap.bmp");
	GxTextureId texmap = getTexture("texmap.bmp");
 	
//--------------------------------------------------------------------
// Main loop.
 	
	while (!keyboard.wentDown(SDLK_ESCAPE)) {
	
		framework.process();
		
		static float t = 0.0;
		static float ry=0.0;	
	
//--------------------------------------------------------------------
// Input.
	
		int mx, my;
		mx = mouse.dx;
		my = mouse.dy;
		ry += mx;
		
		if (mouse.isDown(BUTTON_LEFT))
			field.treshold += 0.01;
		if (mouse.isDown(BUTTON_RIGHT))
			field.treshold -= 0.01;			
   	
//--------------------------------------------------------------------
// Update / logic.
	
		// Animate metaballs.
		
		for (int i=0; i<balls; i++) {
		
			metaball[i].x = sin(t/(111.111+i*11.1))*0.6;
			metaball[i].y = cos(t/(133.333+i*13.3))*0.6;			
			metaball[i].z = sin(t/(266.666+i*26.6))*0.6;
			metaball[i].a = +0.05;				
		
		}

		// Calculate energy values using metaballs.
		
		field.calculate(metaball, balls);
		
		#if 1
		
		// Add 2 lines.
		
		float s1 = (cos(t/1111.111*1.3)+1.5)/2.5;
		float s2 = (cos(t/1333.333*1.3)+1.5)/2.5;
		for (int i=0; i<field.sx; i++)
			for (int j=0; j<field.sy; j++)
				for (int k=0; k<field.sz; k++) {
				
					float dx, dy, dz, d, e=0.0;
					
                    dx = field.v[i][j][k].x;
					dy = 0.0;
					dz = field.v[i][j][k].z * s1;					
					d = dx*dx + dy*dy + dz*dz;
					
					e += 0.02f * field.treshold / (d + 0.001f);
					
                    dx = 0.0;
					dy = field.v[i][j][k].y * s2;
					dz = field.v[i][j][k].z;					
					d = dx*dx + dy*dy + dz*dz;
					
					e += 0.02f * field.treshold / (d + 0.001f);
					
					field.v[i][j][k].e += e;
				
				}
				
		#endif				
		
		// Invert?
		
		if (keyboard.isDown(SDLK_i)) {
			for (int i=0; i<field.sx; i++)
				for (int j=0; j<field.sy; j++)
					for (int k=0; k<field.sz; k++)
						field.v[i][j][k].e = -field.v[i][j][k].e;
		}
		
		// We will be using the normal vectors, so calculate them.

		field.calculate_normals();

		t += 1.0;
		ry += 0.3;
		
//--------------------------------------------------------------------
// Render.
		
		framework.beginDraw(0, 0, 0, 0);
		
		// Setup matrices.
		
		projectPerspective3d(90.0, 0.001, 100.0);
		
		pushDepthTest(true, DEPTH_LESS);
		
		setBlend(BLEND_OPAQUE);
		
		// Setup and enable lighting.

	#if 0 // todo : enable lighting
		const float l_direction[4][4] =
			{		
				{ +1.0, +1.0, 0.0, 0.0 },
				{ -1.0, +1.0, 0.0, 0.0 },
				{ -1.0, -1.0, 0.0, 0.0 },
				{ +1.0, -1.0, 0.0, 0.0 }
			};
			
		const float l_diffuse[4][4] =
			{		
				{ 1.5, 0.5, 0.5, 1.0 },
				{ 1.5, 0.5, 1.5, 1.0 },
				{ 0.5, 0.5, 1.5, 1.0 },
				{ 2.0, 2.0, 2.0, 1.0 }
			};
		
		for (int i=0; i<4; i++) {   				
			for (int j=0; j<3; j++)
				if (l_diffuse[i][j] == 0.5)
					l_diffuse[i][j] = 1.0;
			glLightfv(GL_LIGHT1+i, GL_POSITION, l_direction[i]);
			glLightfv(GL_LIGHT1+i, GL_DIFFUSE, l_diffuse[i]);
			glEnable(GL_LIGHT1+i);
		}
  			
		glEnable(GL_LIGHTING);
	#endif

		// Camera transformation.
		
		gxScalef(1, 1, -1);
		gxTranslatef(0.0, 0.0, -2.0);
		gxTranslatef(0.0, (sin(t*0.003)-0.5)/3.0, 0.0);
		gxRotatef(ry/1.111, 0.0, 1.0, 0.0);

		// Enable sphere mapping. This will give a nice reflective effect.

		Shader shader("envmap");
		setShader(shader); {
		
			shader.setTexture("envmap", 0, envmap, true, true);
			
			CIsosurfaceVertex v[600];             // Vertex buffer.
			field.output(200, v, draw_triangles); // Render triangles using specified callback function.
			
		} clearShader();
		
		// Draw floor.
	
		gxSetTexture(texmap, GX_SAMPLE_LINEAR, true);
		
  		pushDepthWrite(false); // We disable writing to the depth buffer. When drawing multiple unsorted transparent surfaces, you will need to do this or suffer the consequences.
	
		gxColor4f(1.0, 1.0, 1.0, 0.5);
	
		pushBlend(BLEND_ALPHA);
		
		gxBegin(GX_QUADS); {
	
			gxTexCoord2f(0.0, 0.0);
			gxVertex3f(-2.0, -1.0, -2.0);
			gxTexCoord2f(1.0, 0.0);
			gxVertex3f(+2.0, -1.0, -2.0);
			gxTexCoord2f(1.0, 1.0);
			gxVertex3f(+2.0, -1.0, +2.0);
			gxTexCoord2f(0.0, 1.0);
			gxVertex3f(-2.0, -1.0, +2.0);
			
		} gxEnd();
	
		popBlend();
	
		popDepthWrite();
		
		gxClearTexture();
		
		popDepthTest();
 		
//--------------------------------------------------------------------
// Make back buffer visible.
		
		framework.endDraw();
	
	}

//--------------------------------------------------------------------
// Shutdown system.

	framework.shutdown();
	
	return 0;

}

//--------------------------------------------------------------------
// Triangles drawer.

static void draw_triangles(int num, CIsosurfaceVertex* v) {

	// Use line mode?
	
	pushWireframe(keyboard.isDown(SDLK_l));
	
	// Prepare the mesh to draw.
	
	GxMesh mesh;
	
	const GxVertexInput inputs[2] =
	{
		{ VS_POSITION, 3, GX_ELEMENT_FLOAT32, false, offsetof(CIsosurfaceVertex,  x), 0 },
		{ VS_NORMAL,   3, GX_ELEMENT_FLOAT32, false, offsetof(CIsosurfaceVertex, na), 0 }
	};
	
	GxVertexBuffer vb;
	
	mesh.setVertexBuffer(&vb, inputs, 2, sizeof(CIsosurfaceVertex));

	// Draw mesh.
	
	vb.alloc(v, num * 3 * sizeof(CIsosurfaceVertex));
	mesh.draw(GX_TRIANGLES, 0, num * 3);
	vb.free();
	
	// Mirror y coordinate.
	
	for (int i=0; i<num*3; i++) {
	
		v[i].y = -v[i].y-2.0;
		v[i].nb = -v[i].nb;
	
	}
	
	// Draw the mesh a second time.
	
	vb.alloc(v, num * 3 * sizeof(CIsosurfaceVertex));
	mesh.draw(GX_TRIANGLES, 0, num * 3);
	vb.free();

	popWireframe();
	
}
