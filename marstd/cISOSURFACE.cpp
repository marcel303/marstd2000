#include <allegro.h>
#include "cISOSURFACE.h"

static float treshold = 1.0;

static inline void interpolate(CIsosurfaceValue* p1, CIsosurfaceValue* p2, CIsosurfaceVertex* pout) {

        if (p1->e == p2->e || p1->e == treshold) {

            // Copy first point.
		
        	pout->x = p1->x;
        	pout->y = p1->y;
        	pout->z = p1->z;
        	pout->na = p1->na;
        	pout->nb = p1->nb;
        	pout->nc = p1->nc;                 	
        
        } else if (p2->e == treshold) {

            // Copy second point.
		
        	pout->x = p2->x;
        	pout->y = p2->y;
        	pout->z = p2->z;
        	pout->na = p2->na;        	
        	pout->nb = p2->nb;        	
        	pout->nc = p2->nc;        	                 	

        } else {
        
        	// Interpolate linearly 99% of time. Note this is probably slightly incorrect, but fast.
        	
		// e1 + de * t = d
        // t = (d - e1) / de
        	
		const float t = (treshold - p1->e) / (p2->e - p1->e);

	        pout->x = p1->x + t * (p2->x - p1->x);
        	pout->y = p1->y + t * (p2->y - p1->y);
	        pout->z = p1->z + t * (p2->z - p1->z);
	        pout->na = p1->na + t * (p2->na - p1->na);
        	pout->nb = p1->nb + t * (p2->nb - p1->nb);
	        pout->nc = p1->nc + t * (p2->nc - p1->nc);	        

        }

}

#include "Cmetab.inc"

/*

This functions does most of the magic. It uses the marching cubes algoritms
to find a nice surface for the energy values in the grid. If you want to
read some texts explaining visualisation of iso surfaces, search the net
for "marching cubes", "meta balls", "iso surfaces" and "implicit surfaces".

*/

void CIsosurface::output(int triangles, CIsosurfaceVertex* v, void (*triangle)(int num, CIsosurfaceVertex* p)) {

        ::treshold = treshold;

        int current_triangle = 0;

        for (int i=0; i<sx-1; i++)
        	for (int j=0; j<sy-1; j++)
                	for (int k=0; k<sz-1; k++) {

	// ** VERY ** SPECIAL CASE! I DON'T INDENT HERE!

    CIsosurfaceCube* g = &this->cube[i][j][k];

    CIsosurfaceVertex p[12];

    // Get cube index. Every cube point could be in or outside
    // treshold. With 8 corners, this is 2**8 = 256 possibilities.
    // 256 different cubes, and we got all there information stored in
    // an array! hah!

	int cube_index = 0;

	if (g->v[0]->e < treshold) cube_index |= 1;
	if (g->v[1]->e < treshold) cube_index |= 2;
	if (g->v[2]->e < treshold) cube_index |= 4;
	if (g->v[3]->e < treshold) cube_index |= 8;
    if (g->v[4]->e < treshold) cube_index |= 16;
	if (g->v[5]->e < treshold) cube_index |= 32;
	if (g->v[6]->e < treshold) cube_index |= 64;
	if (g->v[7]->e < treshold) cube_index |= 128;

    // Edges tell us which edges to interpolate. Return if this is none.

    const int edges = edge_table[cube_index];

	if (edges == 0)
        continue;

	// Check edge table for this cube to know which corners to interpolate between, and which not.

	if (edges & 1)		interpolate(g->v[0], g->v[1], &p[0]);	// "Top"
	if (edges & 2)		interpolate(g->v[1], g->v[2], &p[1]);
	if (edges & 4)		interpolate(g->v[2], g->v[3], &p[2]);
	if (edges & 8)		interpolate(g->v[3], g->v[0], &p[3]);
	if (edges & 16)		interpolate(g->v[4], g->v[5], &p[4]);	// "Bottom"
	if (edges & 32)		interpolate(g->v[5], g->v[6], &p[5]);
	if (edges & 64)		interpolate(g->v[6], g->v[7], &p[6]);
	if (edges & 128)	interpolate(g->v[7], g->v[4], &p[7]);
	if (edges & 256)	interpolate(g->v[0], g->v[4], &p[8]);	// "Left"
	if (edges & 512)	interpolate(g->v[1], g->v[5], &p[9]);
	if (edges & 1024)	interpolate(g->v[2], g->v[6], &p[10]);	// "Right"
	if (edges & 2048)	interpolate(g->v[3], g->v[7], &p[11]);

	int index = 0;

    while (triangle_table[cube_index][index] != -1) {

        // Some copying here. Cannot be prevented if we want to output the way we do.
		
        v[current_triangle*3+0] = p[triangle_table[cube_index][index+0]];
        v[current_triangle*3+1] = p[triangle_table[cube_index][index+1]];
        v[current_triangle*3+2] = p[triangle_table[cube_index][index+2]];

        current_triangle++;

		// Output if buffer is full.
		
        if (current_triangle == triangles) {
            triangle(triangles, v);
            current_triangle = 0;
        }

        index += 3;

    }

                    }

    if (current_triangle)
        triangle(current_triangle, v);

}

static CMesh* output_mesh;

static void cb_output_to_mesh(CIsosurfaceVertex* v) {

	CPoly* poly = output_mesh->add(new CPoly);
	
	for (int i=0; i<3; i++) {
	
		CEdge* edge = poly->add(new CEdge);
		
		edge->p[0] = v[i].x;
		edge->p[1] = v[i].y;
		edge->p[2] = v[i].z;  		
		
	}
 
   	poly->init();		

}

void CIsosurface::output(CMesh& mesh) {

	::output_mesh = &mesh;
	
	output(cb_output_to_mesh);

}
