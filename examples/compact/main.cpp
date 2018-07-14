#include <allegro.h>
#include "marstd.h"

static BITMAP* colourmap;
static CMatrix matrix;
static CCompactVertex* vertex_array;

static void create_mesh(CCompactMesh& mesh);
static void draw_triangle(int v1, int v2, int v3);
static void draw_line(int v1, int v2, int c);

static int alloc_count = 0;
void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* p);
void operator delete[](void* p);

int main() {

	allegro_init();
	
	{
	
	install_keyboard();
	install_mouse();
	set_color_depth(16);
	set_gfx_mode(GFX_AUTODETECT, 320, 240, 0, 0);
	
	colourmap = create_bitmap(SCREEN_W, SCREEN_H);
	
	CCompactMesh mesh;
	
	create_mesh(mesh);
	
	mesh.finalize();
	
	vertex_array = mesh.vertex;
	
	while (!key[KEY_ESC]) {
	
		static float rx=0.0, ry=0.0, rz=0.0;
		
//		rx += 0.00111;
//		ry += 0.00211;
//		rz += 0.00311;
		
		int mx, my;
		get_mouse_mickeys(&mx, &my);
		rx -= my * 0.01;
		ry += mx * 0.01;
		
		matrix.identity();
		matrix.translate(0.0, 0.0, 2.0);
		matrix.rotate(rx, ry, rz);
		
		set_projection_viewport(0, 0, colourmap->w, colourmap->h);
		
		clear(colourmap);
		
		for (int i=0; i<mesh.poly_count; ++i) {
			CCompactPoly* poly = mesh.poly[i];
			// Do a local -> camera conversion of normal and calculate distance.
			CVector normal;
			matrix.apply(poly->plane.normal, normal);
			// FIXME: Must have a mul3 (ala dp3) function in custom implementation!
			normal[0] -= matrix.m[matrix.depth].t[0];
			normal[1] -= matrix.m[matrix.depth].t[1];
			normal[2] -= matrix.m[matrix.depth].t[2];   			
			CVector vertex;
			matrix.apply(vertex_array[poly->vertex[0]].p, vertex);
			float distance = - normal * vertex;
			if (distance < 0.0)
				poly->flags = 0;
			else
   				poly->flags = 1;				
			if (poly->flags)   				
				draw_triangle(poly->vertex[0],
					      poly->vertex[1],
					      poly->vertex[2]);
		}
		
		for (int i=0; i<mesh.edge_count; ++i) {
			CCompactEdge& edge = mesh.edge[i];
			int c;							
			if (!edge.poly1 || !edge.poly2) {
				c = makecol(127, 127, 127);
			} else if (edge.poly1->flags && edge.poly2->flags) {
				// Both polygons are visible.
				continue;				
				if (edge.poly1->plane.normal * edge.poly2->plane.normal < 0.0)
					c = makecol(191, 191, 191);
				else
    					c = makecol(63, 63, 63);					
			} else if (edge.poly1->flags || edge.poly2->flags) {
				// One polygon is visible. Silhouette edge.
				c = makecol(255, 255, 0);
			} else {
				// No polygons visible.
				continue;				
				c = makecol(227, 227, 227);
			}
			draw_line(edge.vertex1, edge.vertex2, c);
		}
		
		blit(colourmap, screen, 0, 0, 0, 0, colourmap->w, colourmap->h);
	
	}
	
	destroy_bitmap(colourmap);
	
	}
	
//	if (alloc_count != 0)
//		allegro_message("%d allocations left.", alloc_count);
		
	allegro_exit();		
	
	return alloc_count;

} END_OF_MAIN();

static void create_mesh(CCompactMesh& mesh) {

	const int start = mesh.vertex_count;
	CCompactVertex vertex;
 	
	// Add vertices.
	
	const int slices = 10;
	const float step = 2.0*M_PI/slices;
	float angle = 0.0;
	
	vertex.p[0] = 0.0;
	vertex.p[1] = 0.0;
	vertex.p[2] = 0.0;
	
	mesh.add(vertex);
	
	vertex.p[2] = 1.0;
	
	mesh.add(vertex);
	
	for (int i=0; i<slices; ++i) {
		
		vertex.p[0] = sin(angle);
		vertex.p[1] = cos(angle);
		vertex.p[2] = 0.0;
		
		mesh.add(vertex);
		
		angle += step;
	
	}

	// Add triangles.

	for (int i=0; i<slices; ++i) {

		CCompactPoly* poly;

  		poly = new CCompactPoly;

		poly->add(0);
		poly->add(2+i);		
		poly->add(2+((i+1)%slices));		

		mesh.add(poly);

		poly = new CCompactPoly;

		poly->add(1);
		poly->add(2+((i+1)%slices));						
		poly->add(2+i);

		mesh.add(poly);		

	}
	

}

static void draw_triangle(int v1, int v2, int v3) {

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

	CCompactVertex& vertex1 = vertex_array[v1];
	CCompactVertex& vertex2 = vertex_array[v2];
	CCompactVertex& vertex3 = vertex_array[v3];
  	
	// Transform.
	
        matrix.apply(vertex1.p[0], vertex1.p[1], vertex1.p[2], vold[0]->x, vold[0]->y, vold[0]->z);
        matrix.apply(vertex2.p[0], vertex2.p[1], vertex2.p[2], vold[1]->x, vold[1]->y, vold[1]->z);
        matrix.apply(vertex3.p[0], vertex3.p[1], vertex3.p[2], vold[2]->x, vold[2]->y, vold[2]->z);

        vold[0]->c = makecol24(255, 255, 255);        

//        vold[0]->c = makecol24(int((sin(vertex1.p[0])+1.0)*0.5*255.0), 0, 0);
//        vold[1]->c = makecol24(0, int((sin(vertex2.p[1])+1.0)*0.5*255.0), 0);
//        vold[2]->c = makecol24(0, 0, int((sin(vertex3.p[2])+1.0)*0.5*255.0));        

	// Clip.
	
        const int vc = clip3d_f(POLYTYPE_FLAT, 0.01, 10.0, 3, (const V3D_f** )vold, vnew, vtmp, itmp);

        if (vc < 3)
        	return;

	// Project first three vertices.
	
       	persp_project_f(vnew[0]->x, vnew[0]->y, vnew[0]->z, &vnew[0]->x, &vnew[0]->y);
       	persp_project_f(vnew[1]->x, vnew[1]->y, vnew[1]->z, &vnew[1]->x, &vnew[1]->y);
       	persp_project_f(vnew[2]->x, vnew[2]->y, vnew[2]->z, &vnew[2]->x, &vnew[2]->y);                 	
       	
       	// Project the rest.
       	
	for (int i=3; i<vc; i++)
        	persp_project_f(vnew[i]->x, vnew[i]->y, vnew[i]->z, &vnew[i]->x, &vnew[i]->y);

	polygon3d_f(colourmap, POLYTYPE_FLAT, 0, vc, vnew);

}

static void draw_line(int v1, int v2, int c) {

	float x1, y1, z1;
	float x2, y2, z2;
	
        matrix.apply(vertex_array[v1].p[0], vertex_array[v1].p[1], vertex_array[v1].p[2], x1, y1, z1);
        matrix.apply(vertex_array[v2].p[0], vertex_array[v2].p[1], vertex_array[v2].p[2], x2, y2, z2);

	// Do clipping manually.
         
        if (z1 <= 0.01 && z2 <= 0.01)
        	return;
        	
	if (z1 < 0.01 && z2 > 0.01) {
		float t = (0.01 - z1) / (z2 - z1);
		x1 = x1 + (x2 - x1) * t;
		y1 = y1 + (y2 - y1) * t;
		z1 = z1 + (z2 - z1) * t;
	} else if (z1 > 0.01 && z2 < 0.01) {
		float t = (0.01 - z2) / (z2 - z1);
		x2 = x2 + (x2 - x1) * t;		
		y2 = y2 + (y2 - y1) * t;
		z2 = z2 + (z2 - z1) * t;  		
	}
	
	// Clipping against other planes is not neccessary.
	
	// Project.
	
	persp_project_f(x1, y1, z1, &x1, &y1);
	persp_project_f(x2, y2, z2, &x2, &y2);
	
	// Render!

	line(colourmap, (int)x1, (int)y1, (int)x2, (int)y2, c);	
	line(colourmap, (int)x1, (int)y1-1, (int)x2, (int)y2-1, c);
	line(colourmap, (int)x1, (int)y1+1, (int)x2, (int)y2+1, c);	

}

void* operator new(size_t size) {
	alloc_count++;
	return malloc(size);
}
void* operator new[](size_t size) {
	return operator new(size);
}
void operator delete(void* p) {
	free(p);
	alloc_count--;
}
void operator delete[](void* p) {
	operator delete(p);
}
