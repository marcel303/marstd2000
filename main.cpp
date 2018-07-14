//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

#ifdef TEST

#include <allegro.h>
#include <vector>
#include "marstd.h"
#include "marx.h"

//////////////////////////////////////////////////////////////////////
// Material flags.
//////////////////////////////////////////////////////////////////////

// Start of texturing mode.
#define MAT_TEXGEN_UV		0x0001		// Use vertex UV coordinates.
#define MAT_TEXGEN_SCREEN	0x0002		// Use vertex XY coordinates after transform.
#define MAT_TEXGEN_SKY		0x0004		// Use vertex XY coordinates after transform in special sky texgen.
// Start of rendering properties.
#define MAT_OUTLINE		0x0010		// Outline polygons.
#define MAT_TRANSPARENT		0x0020		// Draw polygons using blending.
#define MAT_GRGB		0x0040		// Draw using gouraud RGB interpolation.
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
	BITMAP* texture;
} material_t;

static std::vector<material_t*> materials;

typedef std::vector<material_t*>::iterator p_material_ptr;

typedef struct {
	material_t* mat;
	int polytype;
	CSphere sphere;
} polydata_t;

//////////////////////////////////////////////////////////////////////

static PALETTE palette;		// Palette for 8BPP modes.
static RGB_MAP rgbmap;		// RGB lookup table for 8BPP modes.
static COLOR_MAP colormap;	// Colourmap for 8BPP lighting.
static BITMAP* cmap[2];		// Colourmaps.
static ZBUFFER* zbuffer;	// Z-buffer.
static int br, bg, bb;		// Background colour.
static int use_zbuffer;	// Use z-buffering?
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
static BITMAP* my_load_bitmap(char* filename);

//////////////////////////////////////////////////////////////////////

static unsigned long blend15(unsigned long c1, unsigned long c2, unsigned long n);
static unsigned long blend16(unsigned long c1, unsigned long c2, unsigned long n);
static unsigned long blend24(unsigned long c1, unsigned long c2, unsigned long n);

//////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

//--------------------------------------------------------------------
// Initialize system.
        
	allegro_init();
	install_keyboard();
	install_mouse();
	int bpp = desktop_color_depth() * 1;
//	int bpp = 16;
	if (!bpp)
		bpp = 16;
	set_color_depth(bpp);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 320, 240, 0, 0);

	// Create palette, rgb table, and colour table.

	if (1) {
		BITMAP* tmp = load_bitmap("./data/palette.bmp", palette);
		destroy_bitmap(tmp);
	} else
		generate_332_palette(palette);
	set_palette(palette);
	create_rgb_table(&rgbmap, palette, 0);
	rgb_map = &rgbmap;        
	create_light_table(&colormap, palette, 0, 0, 0, 0);
	color_map = &colormap;        

	// Create color map and z buffer.
	
	if (1) {
		cmap[0] = create_bitmap(SCREEN_W, SCREEN_H);
		cmap[1] = create_bitmap(SCREEN_W, SCREEN_H);		
	} else {
		cmap[0] = create_video_bitmap(SCREEN_W, SCREEN_H);
		cmap[1] = create_video_bitmap(SCREEN_W, SCREEN_H);		
	}		
	
	zbuffer = create_zbuffer(cmap[0]);
	set_zbuffer(zbuffer);

        // Define materials.
        
	INIT_MATERIAL(mat_default,		my_load_bitmap("default.bmp"),		MAT_TEXGEN_UV);
	INIT_MATERIAL(mat_bezier,		my_load_bitmap("bezier.bmp"),		/*MAT_GRGB*/MAT_TEXGEN_UV|MAT_DOUBLESIDED);
	INIT_MATERIAL(mat_large_cube,		my_load_bitmap("large_cube.bmp"),	MAT_TEXGEN_UV|MAT_DOUBLESIDED);
	INIT_MATERIAL(mat_large_cilinder,	my_load_bitmap("zola.bmp"),		MAT_TEXGEN_UV|MAT_DOUBLESIDED);
	INIT_MATERIAL(mat_cone,			my_load_bitmap("cone.bmp"),		MAT_TEXGEN_SCREEN|MAT_OUTLINE);
	INIT_MATERIAL(mat_donut, 		my_load_bitmap("donut.bmp"),		MAT_TEXGEN_UV);
	INIT_MATERIAL(mat_cilinder,		my_load_bitmap("default.bmp"),		MAT_TEXGEN_SKY|MAT_OUTLINE|MAT_TRANSPARENT|MAT_DOUBLESIDED);
	INIT_MATERIAL(mat_cubes,		my_load_bitmap("cubes.bmp"),		MAT_TEXGEN_UV);
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
//		iso.output(mesh);
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
		
		clear(screen);
		textprintf(screen, font, 0, 0,  makecol(255, 255, 255), "please be patient. generating BSP.");
		textprintf(screen, font, 0, 10, makecol(255, 255, 255), "%d polygons", bsp.poly_count);		
		if (1)
			bsp.split();
		else
			bsp.init_bounding_box();

	}
	
	p[1] = -3.0;
 
//--------------------------------------------------------------------
// Main loop.   		

	while (!key[KEY_ESC]) {

		static int t = 0;
		
//--------------------------------------------------------------------
// Input.		

		// Keyboard input.
		
		#if 0
		
		if (key[KEY_LEFT])
			r[1] += 0.025;
		if (key[KEY_RIGHT])
			r[1] -= 0.025;
			
		#endif
			
		if (key[KEY_A])
			v[1] -= 0.07;
		if (key[KEY_Z])
			v[1] += 0.02;
				
		float a = 0.0;

		if (key[KEY_UP] || mouse_b & 1)
			a += 0.02;          				
		if (key[KEY_DOWN] || mouse_b & 2)
			a -= 0.02;  			
  		
		float a2 = 0.0;
		
		if (key[KEY_LEFT])
			a2 -= 0.02;
		if (key[KEY_RIGHT])
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

        if (key[KEY_SPACE]) {
            CVector tmp;
            tmp.set(0.0, 0.2, 0.0);
            float t_t;
            CVector t_normal;
            if (nearest_collision(bsp, p, tmp, t_t, t_normal))
                v[1] = -1.0;
        }

		// Mouse input.
		
		int mx, my;
		get_mouse_mickeys(&mx, &my);
		
   		static float vr[2] = { 0.0, 0.0 };
     
      	vr[0] += my * 0.001;	
      	vr[1] -= mx * 0.001;
           
        #if 0
          	
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
			
		if (key[KEY_R])
			use_zbuffer = 1;
		else
   			use_zbuffer = 0;				

		t++;

//--------------------------------------------------------------------
// Render.

		// Set BPP dependent fog colour and shader.
		
		if (bpp != 8) {
			br = 191;
			bg = 255;
			bb = 127;
            br = 63;
            bg = 31;
            bb = 15;
			set_blender_mode(blend15, blend16, blend24, br, bg, bb, 255);
		} else {
			br = 0;
			bg = 0;
			bb = 0;
			set_trans_blender(0, 0, 0, 255);
		}
		
		// Clear color map.   			
		
		clear_to_color(cmap[0], makecol(br, bg, bb));

		set_projection_viewport(0, 0, cmap[0]->w, cmap[0]->h);
		
   		// Setup matrix.
     	
		matrix.identity();
        matrix.rotate(-r);
	  	matrix.translate(-p);	
		
   		// Setup frustum.
     	
		calculate_frustum();
   		
        // Clear depth map.
        	
   		if (use_zbuffer)
	   		clear_zbuffer(zbuffer, 0.0);
   			
   		memset(&stats, 0, sizeof(stats));
   		
        // Render BSP.
        	
		draw_bsp(&bsp);

		// Show statistics.

        if (key[KEY_S]) {
			text_mode(-1);
			textprintf(cmap[0], font, 0, 0, makecol(255, 255, 255), "%d", t);
		}			
		if (key[KEY_D]) {			
			text_mode(-1);		
			textprintf(cmap[0], font, 0, 15, makecol(255, 255, 255), "VBSP   : %d", stats.draw_vbsp_count);
			textprintf(cmap[0], font, 0, 25, makecol(255, 255, 255), "EBSP   : %d", stats.draw_ebsp_count);
			textprintf(cmap[0], font, 0, 35, makecol(255, 255, 255), "CBSP   : %d", stats.draw_cbsp_count);
			textprintf(cmap[0], font, 0, 45, makecol(255, 255, 255), "POLY  : %d", stats.draw_poly_count);
			textprintf(cmap[0], font, 0, 55, makecol(255, 255, 255), "VERTEX: %d", stats.draw_vertex_count);
		}

//--------------------------------------------------------------------
// Make back buffer visible.
		
		if (!is_video_bitmap(cmap[0])) {
            if (key[KEY_V])
                vsync();
			blit(cmap[0], screen, 0, 0, 0, 0, cmap[0]->w, cmap[0]->h);
		} else
			show_video_bitmap(cmap[0]);
		
		std::swap(cmap[0], cmap[1]);

	}
	
//--------------------------------------------------------------------
// Clean up and shutdown system.
  	
  	p_material_ptr ptr;
  	for (ptr = materials.begin(); ptr != materials.end(); ptr++) {
  		if ((*ptr)->texture)
  			destroy_bitmap((*ptr)->texture);
  	}
  	
	if (polydata)
		delete[] polydata;
	polydata = 0;			

	destroy_zbuffer(zbuffer);
	destroy_bitmap(cmap[0]);
	destroy_bitmap(cmap[1]);	

	return 0;
	
}
END_OF_MAIN();

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

	if (key[KEY_T])
		return 0;
		
	for (int i=0; i<6; i++)
		if (plane[i] * sphere.position + sphere.radius < 0.0)
			return 1;

	return 0;
	
}

//--------------------------------------------------------------------

static int box_in_frustum(CVector& mins, CVector& maxs) {

	if (key[KEY_T])
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

		// Reverse drawing when using z-buffered rendering to maximize overdraw.
		
		if (d > 0.0) {

			if (!use_zbuffer) {
				draw_bsp(bsp->b_b);
				draw_bsp(bsp->b_f);
			} else {				
				draw_bsp(bsp->b_f);
				draw_bsp(bsp->b_b);			
			}
			
		} else {

			if (!use_zbuffer) {
				draw_bsp(bsp->b_f);
				draw_bsp(bsp->b_b);
			} else {				
				draw_bsp(bsp->b_b);
				draw_bsp(bsp->b_f);
			}				
			
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
	
	const int max_lc = 20;
	
	static int initialized = 0;
	
	static V3D_f t_vold[max_lc];
	static V3D_f t_vnew[max_lc*8];
	static V3D_f t_vtmp[max_lc*8];
	static V3D_f* vold[max_lc];
	static V3D_f* vnew[max_lc*8];
	static V3D_f* vtmp[max_lc*8];
	static int itmp[max_lc*8];
	
	if (!initialized) {
		for (int i=0; i<max_lc; i++)
			vold[i] = &t_vold[i];
		for (int i=0; i<max_lc*8; i++) {
			vnew[i] = &t_vnew[i];
			vtmp[i] = &t_vtmp[i];
		}
		initialized = 1;
	}

        static float aspect = SCREEN_H/float(SCREEN_W);
        
	CVector m;
	CEdge* l = p->edge_head;
	for (int i=0; i<p->edge_count; i++) {
		matrix.apply(l->p[0], l->p[1], l->p[2], vold[i]->x, vold[i]->y, vold[i]->z);
		vold[i]->x *= aspect;
//		m += CVector(vold[i]->x, vold[i]->y, vold[i]->z);
		// Will need to interpolate these.
		if (data->mat->flags & MAT_TEXGEN_UV) {
			vold[i]->u = l->t[0][0];
			vold[i]->v = l->t[0][1];
		}
		// Will need to interpolate these too.
		if (data->polytype == POLYTYPE_GRGB)
	                vold[i]->c = makecol24((int)l->c[0], (int)l->c[1], (int)l->c[2]);
		l = l->next;
	}
	m /= p->edge_count;
	
   	int type = data->polytype;

	// Use z-buffering? If so, set the bit so clip3d_f knows, just to be safe.
	
	if (use_zbuffer)
		type |= POLYTYPE_ZBUF;			
  	
	// Clip against planes.
   	
	int new_lc = clip3d_f(type, 0.01, distance, p->edge_count, (const V3D_f** )vold, vnew, vtmp, itmp);

	// Any vertices left?
	
	if (new_lc < 3)
		return;

	// Process clipped vertices.
	
	for (int i=0; i<new_lc; i++) {
	
		// Generate texture coordinates.
		
		if (data->mat->flags & MAT_TEXGEN_SCREEN) {
			float iz = 1.0/vnew[i]->z;
			vnew[i]->u = vnew[i]->x*iz;
			vnew[i]->v = vnew[i]->y*iz;
		} else if (data->mat->flags & MAT_TEXGEN_SKY) {
			float iz = 1.0/vnew[i]->z;
			vnew[i]->u = vnew[i]->x*iz-r[1];
			vnew[i]->v = vnew[i]->y*iz+r[0];
		}
		
		// Project to screen space.
		
		persp_project_f(vnew[i]->x, vnew[i]->y, vnew[i]->z, &vnew[i]->x, &vnew[i]->y);
		
		// Add fog.
		
		int t = int(vnew[i]->z*255.0/distance);
		if (t < 0)
			t = 0;
		else if (t > 255)
  			t = 255;			
		
		if (data->polytype == POLYTYPE_GRGB) {
			int r = getr24(vnew[i]->c);
			int g = getg24(vnew[i]->c);
			int b = getb24(vnew[i]->c);
			r = (r * ( 255 - t) + br * t) >> 8;
			g = (g * ( 255 - t) + bg * t) >> 8;
			b = (b * ( 255 - t) + bb * t) >> 8;   			
			vnew[i]->c = makecol24(r, g, b);
		} else {
	  		vnew[i]->c = 255-t;
		} 
  
       		// Scale texture coordinates. 				
  			
		if (data->mat->texture) {
			vnew[i]->u *= data->mat->texture->w;
			vnew[i]->v *= data->mat->texture->h;
		}
		
	}
 
 	// Finally render the polygon.
 	
	polygon3d_f(cmap[0], type, data->mat->texture, new_lc, vnew);

	#if 1
        if (key[KEY_I] && m[2] > 0.01) {
		persp_project_f(m[0], m[1], m[2], &m[0], &m[1]);
		text_mode(-1);
		textprintf(cmap[0], font, (int)m[0]-4, (int)m[1]-4, makecol(255, 255, 255), "%d/%d", new_lc, p->edge_count);
	}
	#endif

	#if 1
	if ((data->mat->flags & MAT_OUTLINE && !use_zbuffer) || key[KEY_I])
		for (int i=0; i<new_lc; i++)
			line(cmap[0], (int)vnew[i]->x, (int)vnew[i]->y, (int)vnew[(i+1)%new_lc]->x, (int)vnew[(i+1)%new_lc]->y, makecol(63, 63, 63));
	#endif			

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

	for (int i=0; i<10*0; i++) {
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
		if (ABS(p->plane.normal[0]) > ABS(p->plane.normal[1]))
			plane = 0;
		else
			plane = 1;
		if (ABS(p->plane.normal[2]) > ABS(p->plane.normal[plane]))
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

		int type = POLYTYPE_FLAT;
		
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
		
		// Disable translucent rendering when colourmap is a video bitmap.
		
		if (is_video_bitmap(cmap[0])) {
			if (type == POLYTYPE_ATEX_TRANS)
				type = POLYTYPE_ATEX_LIT;
			else if (type == POLYTYPE_PTEX_TRANS)
				type = POLYTYPE_PTEX_LIT;				
		}

		polydata[index].polytype = type;
		
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
	
	if (key[KEY_M]) {
        position += delta;
        return;
	}

	// Do this test four times to slide along multiple planes.
	
	CVector t_delta = delta;
	
    float t;
    CVector normal;
    CVector stored_normal[4];
        
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

static BITMAP* my_load_bitmap(char* filename) {
	char filename2[512];
	sprintf(filename2, "./data/%s", filename);
	set_color_conversion(COLORCONV_DITHER);
	int bpp = bitmap_color_depth(screen);
	set_color_depth(32);
	BITMAP* tmp = load_bitmap(filename2, 0);
	if (!tmp) {
		// Create a default texture with some standard colours.	
		set_color_depth(bpp);
		tmp = create_bitmap(128, 128);
		clear_to_color(tmp, makecol(127, 127, 127));
		text_mode(makecol(0, 0, 0));
		textprintf_centre(tmp, font, tmp->w/2, (tmp->h-text_height(font))/2, makecol(255, 0, 0), "NO TEXTURE");
		for (int i=0; i<5; i++)
			rect(tmp, i, i, tmp->w-1-i, tmp->h-1-i, makecol(0, 0, 0));
		return tmp;
	}
	set_color_depth(bpp);
	BITMAP* bmp = create_bitmap(tmp->w, tmp->h);
	blit(tmp, bmp, 0, 0, 0, 0, tmp->w, tmp->h);
 	destroy_bitmap(tmp);
	return bmp;	
}

//--------------------------------------------------------------------
// Custom blender functions. 3:1 transparency.

static unsigned long blend15(unsigned long c1, unsigned long c2, unsigned long n) {
	const unsigned long r = (getr15(c1)+getr15(c2)*3)>>2;
	const unsigned long g = (getg15(c1)+getg15(c2)*3)>>2;
	const unsigned long b = (getb15(c1)+getb15(c2)*3)>>2; 	
	return makecol15(r, g, b);
}
static unsigned long blend16(unsigned long c1, unsigned long c2, unsigned long n) {
	const unsigned long r = (getr16(c1)+getr16(c2)*3)>>2;
	const unsigned long g = (getg16(c1)+getg16(c2)*3)>>2;
	const unsigned long b = (getb16(c1)+getb16(c2)*3)>>2; 	
	return makecol16(r, g, b);
}
static unsigned long blend24(unsigned long c1, unsigned long c2, unsigned long n) {
	const unsigned long r = (getr24(c1)+getr24(c2)*3)>>2;
	const unsigned long g = (getg24(c1)+getg24(c2)*3)>>2;
	const unsigned long b = (getb24(c1)+getb24(c2)*3)>>2; 	
	return makecol24(r, g, b);
}

#endif
