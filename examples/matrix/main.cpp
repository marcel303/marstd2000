#include <allegro.h>
#include <math.h>
#include "marstd.h"
#include "marx.h"
#include "marx.cpp"

static BITMAP* colourmap;
static BITMAP* texturemap;
static PALETTE palette;
static RGB_MAP rgbmap;
static COLOR_MAP lightmap;
static CMatrix4 matrix;

static void draw_triangle(V3D_f* v1, V3D_f* v2, V3D_f* v3);

int main() {

        allegro_init();
        if (install_keyboard() < 0 || install_mouse() < 0) {
        	allegro_message("Unable to install system drivers.");
        	exit(-1);
        }
        set_color_depth(16);
        if (set_gfx_mode(GFX_AUTODETECT, 320, 240, 0, 0) < 0) {
        	allegro_message("Unable to set graphics mode.");
        	exit(-1);
        }
        
        generate_332_palette(palette);
        set_palette(palette);
        create_rgb_table(&rgbmap, palette, 0);
        rgb_map = &rgbmap;
        create_light_table(&lightmap, palette, 0, 0, 0, 0);
        color_map = &lightmap;
        
        colourmap = create_bitmap(SCREEN_W, SCREEN_H);
        
        // Create texture.
        
        texturemap = create_bitmap(128, 128);
        for (int i=0; i<texturemap->w; i++)
        	for (int j=0; j<texturemap->h; j++) {
        		float u = i/(texturemap->w-1.0)-0.5;
        		float v = j/(texturemap->h-1.0)-0.5;
        		float r = (sin(u*1.0)+1.0)*0.5;
        		float g = (cos(v*2.0)+1.0)*0.5;
        		float b = (sin((u+v)*3.0)+1.0)*0.5;
        		putpixel(texturemap, i, j, makecol((int)(r*255.0), (int)(g*255.0), (int)(b*255.0)));
        	}
	text_mode(makecol(0, 0, 0));        	
	textprintf_centre(texturemap, font, texturemap->w/2, texturemap->h/2, makecol(255, 255, 255), "TEXTURE");

        set_projection_viewport(0, 0, SCREEN_W, SCREEN_H);
        
        while (!key[KEY_ESC]) {

        	static float x=0.0, y=0.0, z=-1.0;
		static float rx=0.0, ry=0.0;
		static int frame = 0;

		if (key[KEY_LEFT])
  			x -= 0.01;
     		if (key[KEY_RIGHT])
       			x += 0.01;                	
		if (key[KEY_A])
  			y -= 0.01;
     		if (key[KEY_Z])
       			y += 0.01;       			
                if (key[KEY_UP])
                	z += 0.01;
		if (key[KEY_DOWN])
                	z -= 0.01;
                	
		int mx, my;
  		get_mouse_mickeys(&mx, &my);
  		
    		rx -= my * 0.01;                	
    		ry -= mx * 0.01;
                	
                matrix.make_translation(-x, -y, -z);
                
                CMatrix4 tmp;
                tmp.make_scaling(0.1, 0.1, 0.1);
                matrix *= tmp;                                
                tmp.make_x_rotation(rx);
                matrix *= tmp;
                tmp.make_y_rotation(ry);
                matrix *= tmp;

                clear(colourmap);
                
                // Render axis.
                
                for (int i=0; i<3; i++) {
                
                	CMatrix4 matrix;
                	matrix.make_identity();
                	
                	V3D_f v[3];
                	
                	v[0].x = v[0].y = v[0].z = 0.0;
                	v[0].u = 0.0;
                	v[0].v = 0.0;
                	
                	v[1].x = matrix.v[0+i*4];
                	v[1].y = matrix.v[1+i*4];
                	v[1].z = matrix.v[2+i*4];
                	v[1].u = 1.0;
                	v[1].v = 0.0;
                	
                	v[2].x = v[2].y = v[2].z = 0.1;
                	v[2].u = 1.0;
                	v[2].v = 1.0;
                	
                	draw_triangle(&v[0], &v[1], &v[2]);
                
                }
	        
              	textprintf(colourmap, font, 5, 5, makecol(255, 255, 255), "%d", frame);
              
		vsync();                       
		blit(colourmap, screen, 0, 0, 0, 0, colourmap->w, colourmap->h);	                
		
		frame++;
        
        }
        
        destroy_bitmap(colourmap);
        
        return 0;

} END_OF_MAIN();

static void draw_triangle(V3D_f* v1, V3D_f* v2, V3D_f* v3) {

	static int initialized = 0;
	static V3D_f s_vold[3];    	
        static V3D_f* vold[3] = { &s_vold[0], &s_vold[1], &s_vold[2] };
        static V3D_f* vnew[3*8];
        static V3D_f* vtmp[3*8];
        static int itmp[3*8];
        static V3D_f s_vnew[3*8];
        static V3D_f s_vtmp[3*8];

	// Initialize vertex structures.
	
	if (!initialized) {
	        for (int i=0; i<3*8; i++) {
        		vnew[i] = &s_vnew[i];
        		vtmp[i] = &s_vtmp[i];
	        }
	        initialized = 1;
	}	        

	// Transform.
	
        matrix.mul(v1->x, v1->y, v1->z, &vold[0]->x, &vold[0]->y, &vold[0]->z);
        matrix.mul(v2->x, v2->y, v2->z, &vold[1]->x, &vold[1]->y, &vold[1]->z);
        matrix.mul(v3->x, v3->y, v3->z, &vold[2]->x, &vold[2]->y, &vold[2]->z);
        
        vold[0]->u = v1->u*(texturemap->w-1);
        vold[0]->v = v1->v*(texturemap->h-1);
        vold[1]->u = v2->u*(texturemap->w-1);
        vold[1]->v = v2->v*(texturemap->h-1);
        vold[2]->u = v3->u*(texturemap->w-1);
        vold[2]->v = v3->v*(texturemap->h-1);

        vold[0]->c = makecol(255, 255, 255);

	// Clip.
	
        const int vc = clip3d_f(POLYTYPE_PTEX, 0.01, 10.0, 3, (const V3D_f** )vold, vnew, vtmp, itmp);

        if (vc < 3)
        	return;

	// Project first three vertices.
	
       	persp_project_f(vnew[0]->x, vnew[0]->y, vnew[0]->z, &vnew[0]->x, &vnew[0]->y);
       	persp_project_f(vnew[1]->x, vnew[1]->y, vnew[1]->z, &vnew[1]->x, &vnew[1]->y);
       	persp_project_f(vnew[2]->x, vnew[2]->y, vnew[2]->z, &vnew[2]->x, &vnew[2]->y);                 	
       	
       	// Project the rest.
       	
	for (int i=3; i<vc; i++)
        	persp_project_f(vnew[i]->x, vnew[i]->y, vnew[i]->z, &vnew[i]->x, &vnew[i]->y);

	polygon3d_f(colourmap, POLYTYPE_PTEX, texturemap, vc, vnew);

}
