//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file xPOLY.cpp: MARX polygon/mesh generators implementation. */
//////////////////////////////////////////////////////////////////////

#include "marx.h"

static void transform(CPoly* poly) {
	CEdge* edge = poly->edge_head;
	while(edge) {
		MARX::matrix.apply(edge->p, edge->p);
		edge = edge->next;
	}
	poly->init();
}

/// Geometry: Primitives: Cube generator.
/**
 * Creates an orthogonal cube with corners (-1.0, -1.0, -1.0), (+1.0, +1.0, +1.0).
 * The generated polygons will be stored in the specified mesh.
 * @param mesh Mesh to put generated polygon in.
 */
void MARX::gen_cube(CMesh& mesh) {

	const int vertice[8][3] =
        {
            { -1, -1, -1 },
            { +1, -1, -1 },
            { +1, +1, -1 },
            { -1, +1, -1 },
            { -1, -1, +1 },
            { +1, -1, +1 },
            { +1, +1, +1 },
            { -1, +1, +1 }
        };

	const int face[6][4] =
        {
            { 3, 2, 1, 0 },	// OK
            { 4, 5, 6, 7 }, // OK
            { 0, 4, 7, 3 },	// OK
            { 2, 6, 5, 1 },	// OK
            { 0, 1, 5, 4 },	// OK
            { 7, 6, 2, 3 }	// OK
        };

	for (int i=0; i<6; i++) {

        CPoly* poly = mesh.add(new CPoly);

        for (int j=0; j<4; j++) {

            CEdge* edge = poly->add(new CEdge);

            edge->p[0] = vertice[face[i][j]][0];
            edge->p[1] = vertice[face[i][j]][1];
            edge->p[2] = vertice[face[i][j]][2];
                
        }

        transform(poly);
        
    }

}

/// Geometry: Primitives: Circle generator.
/**
 * Creates a circle in the OXY plane, with radius 1.0 and slices corner points.
 * The higher the number of slices, the better the approximation of a true circle will be.
 * The generated polygons will be stored in the specified mesh.
 * @param mesh Mesh to put generated polygon in.
 * @param slices Number of divisions. More division make to shape look smoother.
 */
void MARX::gen_circle(CMesh& mesh, int slices) {

	float a = 0.0;
    float s = 2.0*M_PI/slices;

    CPoly* poly = mesh.add(new CPoly);
        
	for (int i=0; i<slices; i++) {

        CEdge* edge = poly->add(new CEdge);

        edge->p[0] = sin(a);
        edge->p[1] = cos(a);

        a += s;
        
    }

	transform(poly);

}

/// Geometry: Primitives: Cilinder generator.
/**
 * Creates a mesh around the z axis with radius 1.0 and height from -1.0 to +1.0.
 * The higher the number of slices, the better the approximation of a true cilinder will be.
 * The generated polygons will be stored in the specified mesh.
 * @param mesh Mesh to put generated polygon in.
 * @param slices Number of divisions. More divisions make the shape look smoother.
 */
void MARX::gen_cilinder(CMesh& mesh, int slices) {

	float s = 2.0*M_PI/slices;
    float a = 0.0;

	CPoly* cb = mesh.add(new CPoly);
	CPoly* ct = mesh.add(new CPoly);	
	
        for (int i=0; i<slices; i++) {

		// Side.
		
		CPoly* poly = mesh.add(new CPoly);

		CEdge* edge1 = poly->add(new CEdge); edge1->p.set(sin(a  ), cos(a  ), +1.0);
		CEdge* edge2 = poly->add(new CEdge); edge2->p.set(sin(a+s), cos(a+s), +1.0);
		CEdge* edge3 = poly->add(new CEdge); edge3->p.set(sin(a+s), cos(a+s), -1.0);
		CEdge* edge4 = poly->add(new CEdge); edge4->p.set(sin(a  ), cos(a  ), -1.0);

		transform(poly);
		
		// Top and bottom.
                
        CEdge* edge;
                
        edge = cb->add(new CEdge);
        edge->p.set(sin(a), cos(a), -1.0);
                
        edge = ct->add(new CEdge);
        edge->p.set(sin(-a), cos(-a), +1.0);
                
        a += s;

	}
	
	transform(cb);
	transform(ct);

}

/// Geometry: Primitives: Cone generator.
/**
 * Creates a cone around the z axis with radius 1.0. The top will be at z=+1.0, the bottom at z=-1.0.
 * The higher the number of slices, the better the approximation of a true cone will be.
 * The generated polygons will be stored in the specified mesh.
 * @param mesh Mesh to put generated polygon in.
 * @param slices Number of divisions. More divisions make the shape look smoother.
 */
void MARX::gen_cone(CMesh& mesh, int slices) {

	float s = 2.0*M_PI/slices;
    float a = 0.0;

	CPoly* cb = mesh.add(new CPoly);
	
    for (int i=0; i<slices; i++) {

		CPoly* poly = mesh.add(new CPoly);

		CEdge* edge1 = poly->add(new CEdge); edge1->p.set(0.0,      0.0,      +1.0);
		CEdge* edge2 = poly->add(new CEdge); edge2->p.set(sin(a+s), cos(a+s), -1.0);
		CEdge* edge3 = poly->add(new CEdge); edge3->p.set(sin(a  ), cos(a  ), -1.0);

		transform(poly);
                
        CEdge* edge = cb->add(new CEdge);
        edge->p.set(sin(a), cos(a), -1.0);
                
        a += s;

	}
	
	transform(cb);
        
}


void MARX::gen_sphere(CMesh& mesh, float radius, int slices, int division) {
}

/// Geometry: Primitives: Donut generator.
/**
 * Generates a donut around the z axis, with an inner radius of r1 and an outer radius of r2.
 * The higher the number of slices and sides, the better the approximation of a true donut will be.
 * The generated polygons will be stored in the specified mesh.
 * @param mesh Mesh to put generated polygon in.
 * @param sides Number of divisions around z axis. The more the smoother.
 * @param slices Number of divisions around local y axis. The more the smoother.
 * @param r1 Inner radius.
 * @param r2 Outer radius.
 */
void MARX::gen_donut(CMesh& mesh, int sides, int slices, float r1, float r2) {

	r1 += r2;
        
	float s1 = 2.0*M_PI/sides;
    float a1 = 0.0;

    for (int i=0; i<sides; i++) {

        float s2 = 2.0*M_PI/slices;
        float a2 = 0.0;

        for (int j=0; j<slices; j++) {

            CPoly* poly = mesh.add(new CPoly);
                        
            MARX::matrix.push();
            MARX::matrix.identity();
                        
            MARX::matrix.push();
            CVector r, t;
            r[0] = 0.0;
            r[1] = 0.0;
            r[2] = a1;
            t[0] = r1;
            t[1] = 0.0;
            t[2] = 0.0;
            MARX::matrix.rotate(r);
            MARX::matrix.translate(t);
            CEdge* edge1 = poly->add(new CEdge); edge1->p.set(sin(a2+s2)*r2, 0.0, cos(a2+s2)*r2);
            CEdge* edge2 = poly->add(new CEdge); edge2->p.set(sin(a2   )*r2, 0.0, cos(a2   )*r2);
            MARX::matrix.apply(edge1->p, edge1->p);
            MARX::matrix.apply(edge2->p, edge2->p);
            MARX::matrix.pop();

            MARX::matrix.push();
            r[0] = 0.0;
            r[1] = 0.0;
            r[2] = a1+s1;
            t[0] = r1;
            t[1] = 0.0;
            t[2] = 0.0;
            MARX::matrix.rotate(r);
            MARX::matrix.translate(t);
            CEdge* edge3 = poly->add(new CEdge); edge3->p.set(sin(a2   )*r2, 0.0, cos(a2   )*r2);
            CEdge* edge4 = poly->add(new CEdge); edge4->p.set(sin(a2+s2)*r2, 0.0, cos(a2+s2)*r2);
            MARX::matrix.apply(edge3->p, edge3->p);
            MARX::matrix.apply(edge4->p, edge4->p);
            MARX::matrix.pop();

			MARX::matrix.pop();
			
			transform(poly);
                        
            a2 += s2;
                
        }

        a1 += s1;

    }

}

