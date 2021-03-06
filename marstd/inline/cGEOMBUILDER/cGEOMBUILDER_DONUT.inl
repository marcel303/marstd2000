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