//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <vector>
#include "marstd.h"
#include "marx.h"
#include "framework.h"

#define SCREEN_W 800
#define SCREEN_H 600

//////////////////////////////////////////////////////////////////////
// Material flags.
//////////////////////////////////////////////////////////////////////

// Start of texturing mode.
#define MAT_TEXGEN_UV		0x0001		// Use vertex UV coordinates.
#define MAT_TEXGEN_SCREEN	0x0002		// Use vertex XY coordinates after transform.
#define MAT_TEXGEN_SKY		0x0004		// Use vertex XY coordinates after transform in special sky texgen.
// Start of rendering properties.
#define MAT_OUTLINE			0x0010		// Outline polygons.
#define MAT_TRANSPARENT		0x0020		// Draw polygons using blending.
#define MAT_GRGB			0x0040		// Draw using gouraud RGB interpolation.
// Start of geometric properties.
#define MAT_DOUBLESIDED		0x0100		// Don't apply backface culling.

//////////////////////////////////////////////////////////////////////
// Material type.
//////////////////////////////////////////////////////////////////////

#define DECLARE_MATERIAL(_name) material_t _name;
#define INIT_MATERIAL(_name, _texture, _flags) \
	{ \
		materials.push_back(&_name); \
		_name.texture = _texture; \
		_name.flags = _flags; \
	}		

typedef struct {
	int flags;
	GLuint texture;
} material_t;

static std::vector<material_t*> materials;

typedef std::vector<material_t*>::iterator p_material_ptr;

typedef struct {
	material_t* mat;
	int polytype;
	CSphere sphere;
} polydata_t;

//////////////////////////////////////////////////////////////////////

static int br, bg, bb;		// Background colour.
static CVector p, v, r;		// Position, velocity, rotation.
static CPlane plane[6];		// Clipping planes (viewing frustum).
static CMatrix matrix;		// Transformation matrix.
static float distance = 20.0;	// Max viewing distance.
static polydata_t* polydata = 0;// Polygon data.

//////////////////////////////////////////////////////////////////////
// Materials.
//////////////////////////////////////////////////////////////////////

DECLARE_MATERIAL(mat_default);
DECLARE_MATERIAL(mat_bezier);
DECLARE_MATERIAL(mat_large_cube);
DECLARE_MATERIAL(mat_large_cilinder);
DECLARE_MATERIAL(mat_cone);
DECLARE_MATERIAL(mat_donut);
DECLARE_MATERIAL(mat_cilinder);
DECLARE_MATERIAL(mat_cubes);
DECLARE_MATERIAL(mat_isosurface);

//////////////////////////////////////////////////////////////////////

static struct {
	int draw_vbsp_count;
	int draw_ebsp_count;
	int draw_cbsp_count;
	int draw_poly_count;
	int draw_vertex_count;
} stats;

//////////////////////////////////////////////////////////////////////

static void calculate_frustum();
static int box_in_frustum(CVector& mins, CVector& maxs);
static void draw_bsp(CBsp* bsp);
static void draw_poly(CPoly* poly);
static void reverse(CMesh& mesh);
static void create_world(CBsp& bsp);
static void initialize(CBsp& bsp);
static int nearest_collision(CBsp& bsp, CVector& position, CVector& delta, float& t, CVector& normal);
static void move_pushback(CBsp& bsp, CVector& position, CVector& delta, CVector& deltaout);
static GLuint my_load_bitmap(const char* filename);

//////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

//--------------------------------------------------------------------
// Initialize system.

#if defined(CHIBI_RESOURCE_PATH)
	changeDirectory(CHIBI_RESOURCE_PATH);
#endif
	
	if (!framework.init(SCREEN_W, SCREEN_H))
		return -1;
	
	mouse.showCursor(false);
	
	// Define materials.
        
	INIT_MATERIAL(mat_default,			my_load_bitmap("default.bmp"),		MAT_TEXGEN_UV);
	INIT_MATERIAL(mat_bezier,			my_load_bitmap("bezier.bmp"),		MAT_GRGB|MAT_TEXGEN_UV|MAT_DOUBLESIDED|MAT_TRANSPARENT);
	INIT_MATERIAL(mat_large_cube,		my_load_bitmap("large_cube.bmp"),	MAT_TEXGEN_UV|MAT_DOUBLESIDED);
	INIT_MATERIAL(mat_large_cilinder,	my_load_bitmap("bezier.bmp"),			MAT_TEXGEN_UV|MAT_DOUBLESIDED);
	INIT_MATERIAL(mat_cone,				my_load_bitmap("cone.bmp"),			MAT_TEXGEN_SCREEN|MAT_OUTLINE);
	INIT_MATERIAL(mat_donut, 			my_load_bitmap("donut.bmp"),		MAT_TEXGEN_UV);
	INIT_MATERIAL(mat_cilinder,			my_load_bitmap("default.bmp"),		MAT_TEXGEN_SKY|MAT_OUTLINE|MAT_TRANSPARENT|MAT_DOUBLESIDED);
	INIT_MATERIAL(mat_cubes,			my_load_bitmap("cubes.bmp"),		MAT_OUTLINE|MAT_TEXGEN_UV|MAT_DOUBLESIDED|MAT_TRANSPARENT);
	INIT_MATERIAL(mat_isosurface,		my_load_bitmap("notexture"),		MAT_TEXGEN_UV|MAT_DOUBLESIDED|MAT_TRANSPARENT);

	// Create geometry.
	
	CBsp bsp;
	
	{
	
		// Add tileable patches.
		
		const int num = 3;
        for (int i=0; i<num; i++) {
            for (int j=0; j<num; j++) {
                MARX::matrix.push();
                MARX::matrix.identity();
                MARX::matrix.translate(-(num-1)/2.0*30+30.0*i, 0.0, -(num-1)/2.0*30+30.0*j);
				create_world(bsp);
                MARX::matrix.pop();
            }
		}

		CMesh mesh;
		MARX::matrix.push();
		// Add isosurface.		
		MARX::matrix.identity();
		MARX::matrix.st(3.0, 3.0, 3.0, 7.5, -2.0, 7.5);
		CIsosurface iso;
		iso.set_size(5, 5, 5);
		CIsosurfaceMetaball ball[3];
		for (int i=0; i<3; i++) {
			ball[i].x = ((rand()&255)/255.0-0.5)*2.0;
			ball[i].y = ((rand()&255)/255.0-0.5)*2.0;
			ball[i].z = ((rand()&255)/255.0-0.5)*2.0;
			ball[i].a = 0.3;
		}
		iso.calculate(ball, 3);
		iso.output(mesh);
		mesh.transform(MARX::matrix);
		reverse(mesh);
		mesh.paint(&mat_isosurface);
		mesh.move(bsp);
		// Add cilinder.
		MARX::matrix.identity();
		MARX::matrix.scale(75.0, 5.0, 75.0);
		MARX::matrix.rotate(M_PI*0.5, 0.0, 0.0);
		MARX::gen_cilinder(mesh, 40);
		reverse(mesh);
		mesh.paint(&mat_large_cilinder);
		mesh.move(bsp);
		MARX::matrix.pop();

		// Initialize and split.
		
		initialize(bsp);
		
		if (1)
			bsp.split();
		else
			bsp.init_bounding_box();

	}
	
	p[1] = -3.0;
 
//--------------------------------------------------------------------
// Main loop.   		

	while (!keyboard.wentDown(SDLK_ESCAPE)) {

		framework.process();
		
		mouse.setRelative(true);
		
		static int t = 0;
		
//--------------------------------------------------------------------
// Input.		

		// Keyboard input.
		
		#if 0
		
		if (keyboard.isDown(SDLK_LEFT))
			r[1] += 0.025;
		if (keyboard.isDown(SDLK_RIGHT))
			r[1] -= 0.025;
			
		#endif
			
		if (keyboard.isDown(SDLK_a))
			v[1] -= 0.07;
		if (keyboard.isDown(SDLK_z))
			v[1] += 0.02;
				
		float a = 0.0;

		if (keyboard.isDown(SDLK_UP) || mouse.isDown(BUTTON_LEFT))
			a += 0.02;          				
		if (keyboard.isDown(SDLK_DOWN) || mouse.isDown(BUTTON_RIGHT))
			a -= 0.02;  			
  		
		float a2 = 0.0;
		
		if (keyboard.isDown(SDLK_LEFT))
			a2 -= 0.02;
		if (keyboard.isDown(SDLK_RIGHT))
			a2 += 0.02;
			
  		if (a) {
            float ay = sin(r[0]) * a;
			float axz = a - fabs(ay/a)*a;
       		
			v[0] -= sin(r[1]) * axz;
			v[2] += cos(r[1]) * axz;
			v[1] += ay;
		}

		v[0] += cos(r[1]) * a2;
		v[2] += sin(r[1]) * a2;
   	
		v[1] += 0.01;

		// Jump if on the ground.

        if (keyboard.isDown(SDLK_SPACE)) {
            CVector tmp;
            tmp.set(0.0, 0.2, 0.0);
            float t_t;
            CVector t_normal;
            if (nearest_collision(bsp, p, tmp, t_t, t_normal))
                v[1] = -1.0;
        }

		// Mouse input.
		
		const int mx = mouse.dx;
		const int my = mouse.dy;
		
   		static float vr[2] = { 0.0, 0.0 };
     
        #if 0
		
        vr[0] += my * 0.001;
      	vr[1] -= mx * 0.001;
		
      	#else
          	
		r[0] += my*0.01;
		r[1] -= mx*0.01;
		
		#endif
		
		r[0] += vr[0];
		r[1] += vr[1];
		
		vr[0] *= 0.9;
		vr[1] *= 0.9;

		if (r[0] < -M_PI*0.5)
			r[0] = -M_PI*0.5;
		if (r[0] > +M_PI*0.5)
			r[0] = +M_PI*0.5;

//--------------------------------------------------------------------
// Update.

		// Try to move with collision detection.
                        
        CVector delta = v * 0.1;
        CVector deltaout;
        move_pushback(bsp, p, delta, deltaout);
        v = deltaout / 0.1;

		v *= 0.99;

		t++;

//--------------------------------------------------------------------
// Render.

		// Set fog colour.
		
		br = 0;
		bg = 0;
		bb = 0;
		
		// Clear color map.   			
		
		framework.beginDraw(br, bg, bb, 0);
		setFont("VeraMono.ttf");
		
		projectPerspective3d(90.f, .01f, 1000.f);
		gxScalef(1, -1, 1);
		
   		// Setup matrix.
     	
		matrix.identity();
        matrix.rotate(-r);
	  	matrix.translate(-p);	
		
   		// Setup frustum.
     	
		calculate_frustum();
   		
        // Clear depth map.
   			
   		memset(&stats, 0, sizeof(stats));
   		
        // Render BSP.
        	
		draw_bsp(&bsp);
		
		// Show statistics.
		
		projectScreen2d();
		setBlend(BLEND_ALPHA);
		setColor(colorWhite);
		
        if (keyboard.isDown(SDLK_s)) {
			drawText(5, 5, 10, +1, +1, "%d", t);
		}			
		if (keyboard.isDown(SDLK_d)) {
			drawText(5, 15, 10, +1, +1, "VBSP   : %d", stats.draw_vbsp_count);
			drawText(5, 25, 10, +1, +1, "EBSP   : %d", stats.draw_ebsp_count);
			drawText(5, 35, 10, +1, +1, "CBSP   : %d", stats.draw_cbsp_count);
			drawText(5, 45, 10, +1, +1, "POLY   : %d", stats.draw_poly_count);
			drawText(5, 55, 10, +1, +1, "VERTEX : %d", stats.draw_vertex_count);
		}

//--------------------------------------------------------------------
// Make back buffer visible.
		
		framework.endDraw();

	}
	
//--------------------------------------------------------------------
// Clean up and shutdown system.
	
	if (polydata)
		delete[] polydata;
	polydata = 0;			

	framework.shutdown();
	
	return 0;
	
}

//--------------------------------------------------------------------

static void calculate_frustum() {

	matrix.push();
	matrix.identity();
	
	// Inverse of camera matrix.
	
	matrix.translate(p);
	matrix.rotate(0.0, 0.0, r[2]);
	matrix.rotate(0.0, r[1], 0.0);
	matrix.rotate(r[0], 0.0, 0.0); 	

	// Describe viewing piramid.
 	
	float a = SCREEN_W/float(SCREEN_H);
	
	CVector v[5] =
		{
			CVector(0.0, 0.0, 0.0),
			CVector(-distance*a, -distance, distance),
			CVector(+distance*a, -distance, distance),
			CVector(+distance*a, +distance, distance),
			CVector(-distance*a, +distance, distance)
		};
		
	for (int i=0; i<5; i++)
 		matrix.apply(v[i], v[i]);
   
	// Back.
	
	plane[0].setup(v[1], v[2], v[3]);
	plane[0].distance = plane[0].normal * v[0];
	
	// Front.

	plane[1].setup(v[3], v[2], v[1]);
	plane[1].distance = plane[1].normal * v[1];
     		
	// Left and right.
      	
	plane[2].setup(v[0], v[2], v[3]);
	plane[3].setup(v[0], v[4], v[1]);

	// Top and bottom.
	
	plane[4].setup(v[0], v[1], v[2]);
	plane[5].setup(v[0], v[3], v[4]);	
	
	matrix.pop();
	
	for (int i=0; i<6; i++)
		plane[i].normalize();

}

//--------------------------------------------------------------------

static int sphere_out_frustum(CSphere& sphere) {

	if (keyboard.isDown(SDLK_t))
		return 0;
		
	for (int i=0; i<6; i++)
		if (plane[i] * sphere.position + sphere.radius < 0.0)
			return 1;

	return 0;
	
}

//--------------------------------------------------------------------

static int box_in_frustum(CVector& mins, CVector& maxs) {

	if (keyboard.isDown(SDLK_t))
		return 1;

	float x = mins[0];
	float y = mins[1];
	float z = mins[2];
	float x2 = maxs[0];
	float y2 = maxs[1];
	float z2 = maxs[2];
	
	for (int i=0; i<6; i++) {
	
		// Test if all 8 corner points on the back side of the plane.
		
		if (plane[i].normal[0] * x  + plane[i].normal[1] * y  + plane[i].normal[2] * z	- plane[i].distance >= 0.0)
  			continue;
		if (plane[i].normal[0] * x2 + plane[i].normal[1] * y  + plane[i].normal[2] * z	- plane[i].distance >= 0.0)
  			continue;		
		if (plane[i].normal[0] * x  + plane[i].normal[1] * y2 + plane[i].normal[2] * z	- plane[i].distance >= 0.0)
  			continue;		
		if (plane[i].normal[0] * x2 + plane[i].normal[1] * y2 + plane[i].normal[2] * z	- plane[i].distance >= 0.0)
  			continue;		
		if (plane[i].normal[0] * x  + plane[i].normal[1] * y  + plane[i].normal[2] * z2 - plane[i].distance >= 0.0)
  			continue;		
		if (plane[i].normal[0] * x2 + plane[i].normal[1] * y  + plane[i].normal[2] * z2 - plane[i].distance >= 0.0)
  			continue;		
		if (plane[i].normal[0] * x  + plane[i].normal[1] * y2 + plane[i].normal[2] * z2 - plane[i].distance >= 0.0)
  			continue;		
		if (plane[i].normal[0] * x2 + plane[i].normal[1] * y2 + plane[i].normal[2] * z2 - plane[i].distance >= 0.0)
  			continue;		
  			
		return 0;
		
	}

	return 1;

}

//--------------------------------------------------------------------

static int draw_bsp_depth = 0;

static void draw_bsp(CBsp* bsp) {

	stats.draw_vbsp_count++;
	
	if (draw_bsp_depth < 10) {
 	
	if (bsp->poly_count == 0 && sphere_out_frustum(bsp->sphere))
		return;
		
	if (bsp->poly_count == 0 && !box_in_frustum(bsp->mins, bsp->maxs))
		return;
		
	}
 
   	draw_bsp_depth++;		
		
	if (!bsp->b_f || !bsp->b_b) {

		stats.draw_cbsp_count++;
	
		CPoly* poly = bsp->poly_head;

		while (poly) {
		
			draw_poly(poly);
			
			poly = poly->next;
			
		}
	
	} else {
	
		stats.draw_ebsp_count++;
 	
		float d = bsp->p * p;
		
		if (d > 0.0) {

			draw_bsp(bsp->b_b);
			draw_bsp(bsp->b_f);
			
		} else {

			draw_bsp(bsp->b_f);
			draw_bsp(bsp->b_b);
			
		}

	}
	
	draw_bsp_depth--;

}

//--------------------------------------------------------------------

static void draw_poly(CPoly* p) {

	polydata_t* data = (polydata_t* )p->data;
	
	if (sphere_out_frustum(data->sphere))
		return;
	
	// Cull backfacing polygons.
	
	if (!(data->mat->flags & MAT_DOUBLESIDED) && p->plane * ::p <= 0.0)
		return;
	
	stats.draw_vertex_count += p->edge_count;	
	stats.draw_poly_count++;
	
	if (data->mat->flags & MAT_TRANSPARENT)
		setBlend(BLEND_ALPHA);
	else
		setBlend(BLEND_OPAQUE);
	
	gxSetTexture((data->mat->flags & MAT_GRGB) ? 0 : data->mat->texture);
	{
		gxBegin(GL_TRIANGLE_FAN);
		{
			CEdge* l = p->edge_head;
			
			for (int i = 0; i < p->edge_count; ++i)
			{
				float x;
				float y;
				float z;
				
				matrix.apply(l->p[0], l->p[1], l->p[2], x, y, z);
				
				// Generate texture coordinates.
				
				float u = l->t[0][0];
				float v = l->t[0][1];
				
				if (data->mat->flags & MAT_TEXGEN_SCREEN) {
					float iz = 1.0/z;
					u = x*iz;
					v = y*iz;
				} else if (data->mat->flags & MAT_TEXGEN_SKY) {
					float iz = 1.0/z;
					u = x*iz-r[1];
					v = y*iz+r[0];
				}
				
				// Add fog.
		
				float t = z/distance;
			#if 0 // disabled clamp since we don't do clipping anymore
				if (t < 0.f)
					t = 0.f;
				else if (t > 1.f)
					t = 1.f;
			#endif
				
				float r = l->c[0];
				float g = l->c[1];
				float b = l->c[2];
				
				if (data->mat->flags & MAT_GRGB) {
					r = (r * ( 1.f - t) + br * t);
					g = (g * ( 1.f - t) + bg * t);
					b = (b * ( 1.f - t) + bb * t);
				} else {
					r = 255 * (1.f - t) + br * t;
					g = 255 * (1.f - t) + bg * t;
					b = 255 * (1.f - t) + bb * t;
				}
				
				gxTexCoord2f(u, v);
				gxColor4ub(r, g, b, 140);
				gxVertex3f(x, y, z);
				l = l->next;
			}
		}
		gxEnd();
	}
	gxSetTexture(0);
	
	if ((data->mat->flags & MAT_OUTLINE) || keyboard.isDown(SDLK_i))
	{
		gxColor3ub(63, 63, 63);
		
		gxBegin(GL_LINE_LOOP);
		{
			CEdge* l = p->edge_head;
			
			for (int i = 0; i < p->edge_count; ++i)
			{
				float x;
				float y;
				float z;
			
				matrix.apply(l->p[0], l->p[1], l->p[2], x, y, z);
				
				gxVertex3f(x, y, z);
				
				l = l->next;
			}
		}
		gxEnd();
	}
	
	return;

}

//--------------------------------------------------------------------

static void reverse(CMesh& mesh) {

	CPoly* poly = mesh.poly_head;
	
	while (poly) {
	
		CPoly tmp;
		while (poly->edge_head)
			tmp.add(poly->unlink(poly->edge_head));
		while (tmp.edge_head)
			poly->add_head(tmp.unlink(tmp.edge_head));
			
		poly->init();			
			
		poly = poly->next;
	
	}

}

//--------------------------------------------------------------------

static void create_world(CBsp& bsp) {

	CMesh mesh;
        
	CBezier3 bezier;

	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++) {
			bezier.p[i][j][0] = (i/3.0)*15.0-15.0;
			bezier.p[i][j][1] = 0.0;
			bezier.p[i][j][2] = (j/3.0)*15.0-15.0;
		}
		
	for (int i=0; i<2; i++)
		for (int j=0; j<2; j++)
			bezier.p[i*3][j*3][1] = -4.0;

	#if 1
	
	// Add four bezier patches.

    int resolution = 5;

	bezier.translate(CVector(0.0, 2.0, 0.0));
	bezier.generate(resolution);
	bezier.to_mesh(mesh);

	bezier.translate(CVector(15.0, 0.0, 0.0));
	bezier.generate(resolution);
	bezier.to_mesh(mesh);

	bezier.translate(CVector(0.0, 0.0, 15.0));
	bezier.generate(resolution);
	bezier.to_mesh(mesh);

	bezier.translate(CVector(-15.0, 0.0, 0.0));
	bezier.generate(resolution);
	bezier.to_mesh(mesh);
	
	mesh.paint(&mat_bezier);
    mesh.transform(MARX::matrix);
	mesh.move(bsp);
	
	#endif

	// Add a large box.
	
	MARX::matrix.push();
	MARX::matrix.scale(15.0, 3.0, 15.0);
	
//	MARX::gen_cube(mesh);
	mesh.paint(&mat_large_cube);
	mesh.move(bsp);

	MARX::matrix.pop();
	
	// Add a cone.
	
	MARX::matrix.push();
	MARX::matrix.rotate(M_PI*0.5, 0.0, 0.0);
	MARX::matrix.scale(1.0, 1.0, 2.0);

	MARX::gen_cone(mesh, 10);
	mesh.paint(&mat_cone);
	mesh.move(bsp);

	MARX::matrix.pop();
	
	// Add a donut as well.
	
	MARX::matrix.push();
	MARX::matrix.translate(0.0, -3.0, 0.0);
	
//        MARX::gen_donut(mesh, 10, 10, 1.0, 0.5);
	mesh.paint(&mat_donut);
	mesh.move(bsp);
	
	MARX::matrix.pop();

	// Add a cilinder.
        
	MARX::matrix.push();
	MARX::matrix.translate(0.0, -2.0, 2.0);
	MARX::matrix.rotate(M_PI*0.5, 0.0, 0.0);
	MARX::matrix.scale(0.5, 0.5, 4.0);
	
	MARX::gen_cilinder(mesh, 10);
	mesh.paint(&mat_cilinder);
	mesh.move(bsp);
	
	MARX::matrix.pop();

	// Add some random cubes.

	for (int i=0; i<10*1; i++) {
		MARX::matrix.push();
		CVector p;
		p[0] = ((rand()&4095)/4097.0-0.5)*4.0;
		p[1] = ((rand()&4095)/4097.0-0.5)*4.0;
		p[2] = ((rand()&4095)/4097.0-0.5)*4.0;
		MARX::matrix.translate(p);
		MARX::matrix.rotate(p);
		MARX::matrix.scale(0.3, 0.3, 0.3);
		MARX::gen_cube(mesh);
		MARX::matrix.pop();
	}
	
	// Add some more random cubes.

	for (int i=0; i<10*1; i++) {
		MARX::matrix.push();
		float x = ((rand()&4095)/4097.0-0.5)*25.0;
		float y = ((rand()&4095)/4097.0-0.5)*5.0;
		float z = ((rand()&4095)/4097.0-0.5)*25.0;
		MARX::matrix.translate(x, y, z);
		MARX::matrix.rotate(x, y, z);
		MARX::matrix.scale(0.3, 0.3, 0.3);
		MARX::matrix.scale(2.0, 2.0, 2.0);
		MARX::gen_cube(mesh);
		MARX::matrix.pop();
	}
	
	mesh.paint(&mat_cubes);
	mesh.move(bsp);
	
}

//--------------------------------------------------------------------

static void initialize(CBsp& bsp) {

	// Initialize polygons. This should already be done.
	
	if (1) {
		CPoly* poly = bsp.poly_head;
		while (poly) {
			poly->init();
			poly = poly->next;
		}
	}
	
	// Calculate texture coordinates.
	
	CPoly* p = bsp.poly_head;

	const float uv[3][2][3] =
		{
			{
				{ 0.0, 0.0, 1.0 },
				{ 0.0, 1.0, 0.0 }
			},
			{
				{ 0.0, 0.0, 1.0 },
				{ 1.0, 0.0, 0.0 }
			},
			{
				{ 1.0, 0.0, 0.0 },
				{ 0.0, 1.0, 0.0 }
			}
		};
		
	while (p) {

		// Find best texture plane.
		
		int plane;
		if (fabsf(p->plane.normal[0]) > fabsf(p->plane.normal[1]))
			plane = 0;
		else
			plane = 1;
		if (fabsf(p->plane.normal[2]) > fabsf(p->plane.normal[plane]))
			plane = 2;

		CEdge* edge = p->edge_head;

		while (edge) {

			// Create a nice per vertex colour.
			
			edge->c[0] = (sin((edge->p[0] + edge->p[1] - edge->p[2]) * 2.0) + 1.0) *0.5 * 255.0;
			edge->c[1] = (sin((edge->p[1] + edge->p[2] - edge->p[0]) * 2.0) + 1.0) *0.5 * 255.0;
			edge->c[2] = (sin((edge->p[2] + edge->p[0] - edge->p[1]) * 2.0) + 1.0) *0.5 * 255.0;

			// Apply texture plane to get texture coordinates.
			
			for (int i=0; i<2; i++)
				edge->t[0][i] = (uv[plane][i][0] * edge->p[0] + uv[plane][i][1] * edge->p[1] + uv[plane][i][2] * edge->p[2])*0.5;

			edge = edge->next;
		
		}
		
		p = p->next;
	
	}

	// Calculate bounding sphere, fill in polydata struct.
	// This expects poly->data to point to the material, or 0 for default meterial.

	if (bsp.poly_count > 0) {
	
		polydata = new polydata_t[bsp.poly_count];
 	
		CPoly* poly = bsp.poly_head;
		int index = 0;
	
		while (poly) {
	
			polydata[index].mat = poly->data ? (material_t* )poly->data : &mat_default;
			polydata[index].sphere = poly->calculate_sphere();
			poly->data = &polydata[index];
			
			poly = poly->next;
			index++;
	
		}
		
	}
 
	// Determine shader type.
	
	CPoly* poly = bsp.poly_head;
	int index = 0;
	
	while (poly) {
	
		const int lit = 1;
	
		int tex = 0;
		int ptex = 0;
		int trans = 0;

	#if 0
		int type = POLYTYPE_FLAT;
	#endif
		
		if (polydata[index].mat->flags & MAT_GRGB)
 			tex = 0;
	   	else if ((polydata[index].mat->flags & MAT_TEXGEN_SCREEN) || (polydata[index].mat->flags & MAT_TEXGEN_SKY)) {
   			tex = 1;
			ptex = 0;
		} else {
			tex = 1;
			ptex = 1;
		}
		if (polydata[index].mat->flags & MAT_TRANSPARENT)
			trans = 1;
		
	#if 0
		if (tex) {
			if (ptex) {
				if (trans)
					type = POLYTYPE_PTEX_TRANS;
				else if (lit)
   					type = POLYTYPE_PTEX_LIT;
				else
	   				type = POLYTYPE_PTEX;   				
			} else {
				if (trans)
					type = POLYTYPE_ATEX_TRANS;
				else if (lit)
					type = POLYTYPE_ATEX_LIT;
				else				
   					type = POLYTYPE_ATEX;				
			}
		} else {
			type = POLYTYPE_GRGB;
		}

		polydata[index].polytype = type;
	#endif
		
		poly = poly->next;
		index++;
		
 	}		

}

//--------------------------------------------------------------------

const float radius = 0.9;
	
static int nearest_collision(CBsp& bsp, CVector& position, CVector& delta, float& t, CVector& normal) {

	// Radius of moving sphere.
        
    CSphere sphere;
    sphere.position = position;
    sphere.radius = radius;

    // BSP nodes hit potentially hit by moving sphere.
        
	std::vector<CBsp*> leafs;
	MARX::get_hit_leafs(bsp, position, delta.size()+radius*2.0+0.01, leafs);
        
	float best_t = -1.0;

        for (unsigned int leaf=0; leaf<leafs.size(); leaf++) {

        	CBsp* bsp = leafs[leaf];

            CPoly* poly = bsp->poly_head;

            while (poly) {

                float t;
                CVector tmp;
                        
                if (MARX::collision(*poly, sphere, delta, t, tmp) && (t < best_t || best_t < 0.0)) {

                    normal = tmp;
                    best_t = t;
                        
                }
                        
                poly = poly->next;
                
            }
        
        }

        t = best_t;
        
        if (best_t < 0.0)
        	return 0;

	return 1;

}

static void move_pushback(CBsp& bsp, CVector& position, CVector& delta, CVector& deltaout) {

	deltaout = delta;
	
	if (keyboard.isDown(SDLK_m)) {
        position += delta;
        return;
	}

	// Do this test four times to slide along multiple planes.
	
	CVector t_delta = delta;
	
    float t;
    CVector normal;
    CVector stored_normal[5];
        
    for (int i=0; i<5; i++) {

        if (nearest_collision(bsp, position, delta, t, normal)) {

            // Clip delta.

			float distance = normal * delta;
			CVector bounce = normal * distance * 1.0;
			delta -= bounce;
			
			// Check if we already ran into this normal.
			
			for (int j=0; j<i; j++) {
				if (normal * stored_normal[j] > 0.99) {
				
					float distance = position * stored_normal[j];
					
					CVector bounce;
					
					if (distance > 0)
						bounce = stored_normal[j] * 0.0001;
					else
						bounce = -(stored_normal[j] * 0.0001);
						
					delta -= bounce;
					
				}					
			}
			
			stored_normal[i] = normal;

        }

    }
        
//	if (delta * t_delta < 0)
//		delta *= 0;
        
    deltaout = delta;
        
	#if 0
	// FIXME: This sometime falls through geometry. Don't know what's causing this!!
	if (!nearest_collision(bsp, position, delta, t, normal))
        	position += delta;
	#else
	// FIXME: This will get the sphere stuck sometimes.
	CSphere sphere;
	sphere.position = position+delta;
	sphere.radius = radius;
	std::vector<CBsp*> leafs;
	MARX::get_hit_leafs(bsp, sphere.position, sphere.radius*1.0+0.01, leafs);
	int collision = 0;
	for (unsigned int i=0; i<leafs.size(); i++) {
		CBsp* leaf = leafs[i];
		CPoly* poly = leaf->poly_head;
		while (poly) {
			if (MARX::collision(*poly, sphere))
				collision = 1;
			poly = poly->next;
		}
	}
	if (!collision)
		position = sphere.position;
//	else if (!nearest_collision(bsp, position, delta, t, normal))
// 		allegro_message("Warning: move_pushback: result nearest_collision arguable");
 	#endif        	

}

//--------------------------------------------------------------------

static GLuint my_load_bitmap(const char* filename) {
	
	const GLuint texture = getTexture(filename);
	
	if (texture)
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		checkErrorGL();
		
		// set filtering

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		checkErrorGL();
		
		glBindTexture(GL_TEXTURE_2D, 0);
		checkErrorGL();
	}
	
	return texture;
}
