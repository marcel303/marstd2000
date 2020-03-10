#include "framework.h"
#include "marstd.h"

static CMatrix matrix;
static CCompactVertex* vertex_array;

static void create_mesh(CCompactMesh& mesh);
static void draw_triangle(int v1, int v2, int v3);
static void draw_line(int v1, int v2, Color c);

static int alloc_count = 0;
void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* p) noexcept;
void operator delete[](void* p) noexcept;

int main() {

	setupPaths(CHIBI_RESOURCE_PATHS);
	
	{
	
	framework.init(800, 600);
	
	CCompactMesh mesh;
	
	create_mesh(mesh);
	
	mesh.finalize();
	
	vertex_array = mesh.vertex;
	
	while (!keyboard.wentDown(SDLK_ESCAPE)) {
	
		framework.process();
		
		static float rx=0.0, ry=0.0, rz=0.0;
		
//		rx += 0.00111;
//		ry += 0.00211;
//		rz += 0.00311;
		
		int mx, my;
		mx = mouse.dx;
		my = mouse.dy;
		rx -= my * 0.01;
		ry += mx * 0.01;
		
		matrix.identity();
		matrix.translate(0.0, 0.0, 2.0);
		matrix.rotate(rx, ry, rz);
		
		framework.beginDraw(0, 0, 0, 0);
		
		projectPerspective3d(90.0, 0.01, 100.0);
		
		for (int i=0; i<mesh.poly_count; ++i) {
			CCompactPoly* poly = mesh.poly[i];
			// Do a local -> camera conversion of normal and calculate distance.
			CVector normal;
			matrix.apply(poly->plane.normal, normal);
			// FIXME: Must have a mul3 (ala dp3) function in custom implementation!
			normal[0] -= matrix.m[matrix.depth].GetTranslation()[0];
			normal[1] -= matrix.m[matrix.depth].GetTranslation()[1];
			normal[2] -= matrix.m[matrix.depth].GetTranslation()[2];
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
			Color c;
			if (!edge.poly1 || !edge.poly2) {
				c = Color(127, 127, 127);
			} else if (edge.poly1->flags && edge.poly2->flags) {
				// Both polygons are visible.
				if (mouse.isDown(BUTTON_LEFT))
					continue;
				if (edge.poly1->plane.normal * edge.poly2->plane.normal < 0.0) {
					c = Color(127, 127, 127);
					c = c.mulRGB(cosf(framework.time * 6.0) + 2.0);
				}
				else
					c = Color(63, 63, 63);
			} else if (edge.poly1->flags || edge.poly2->flags) {
				// One polygon is visible. Silhouette edge.
				c = Color(255, 255, 0);
			} else {
				// No polygons visible.
				if (!mouse.isDown(BUTTON_RIGHT))
					continue;
				c = Color(227, 227, 227);
			}
			draw_line(edge.vertex1, edge.vertex2, c);
		}
		
		framework.endDraw();
	
	}
	
	}
	
	framework.shutdown();
	
//	if (alloc_count != 0)
//		allegro_message("%d allocations left.", alloc_count);
	
	return alloc_count;

}

static void create_mesh(CCompactMesh& mesh) {

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

	CCompactVertex& vertex1 = vertex_array[v1];
	CCompactVertex& vertex2 = vertex_array[v2];
	CCompactVertex& vertex3 = vertex_array[v3];
	
	// Transform.
	
	Vec3 transformed[3];

	matrix.apply(vertex1.p[0], vertex1.p[1], vertex1.p[2], transformed[0][0], transformed[0][1], transformed[0][2]);
	matrix.apply(vertex2.p[0], vertex2.p[1], vertex2.p[2], transformed[1][0], transformed[1][1], transformed[1][2]);
	matrix.apply(vertex3.p[0], vertex3.p[1], vertex3.p[2], transformed[2][0], transformed[2][1], transformed[2][2]);

	// Colorize.
	
	Color color[3];
	
#if 0

	color[0] = Color(255, 255, 255);
	color[1] = color[0];
	color[2] = color[0];

#else

	color[0] = Color(int((sin(vertex1.p[0])+1.0)*0.5*255.0), 0, 0);
	color[1] = Color(0, int((sin(vertex2.p[1])+1.0)*0.5*255.0), 0);
	color[2] = Color(0, 0, int((sin(vertex3.p[2])+1.0)*0.5*255.0));

#endif

	gxBegin(GX_TRIANGLES);
	{
	
		for (int i = 0; i < 3; ++i)
		{
		
			gxColor4fv(&color[i].r);
			gxVertex3fv(&transformed[i][0]);
			
		}
		
	}
	gxEnd();

}

static void draw_line(int v1, int v2, Color c) {

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
	
	// Render!

	gxBegin(GX_LINES);
	{
		setColor(c);
		gxVertex3f(x1, y1, z1);
		gxVertex3f(x2, y2, z2);
	}
	gxEnd();

}

void* operator new(size_t size) {
	alloc_count++;
	return malloc(size);
}
void* operator new[](size_t size) {
	return operator new(size);
}
void operator delete(void* p) noexcept {
	free(p);
	alloc_count--;
}
void operator delete[](void* p) noexcept {
	operator delete(p);
}
