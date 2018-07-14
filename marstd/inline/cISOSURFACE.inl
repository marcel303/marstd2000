//////////////////////////////////////////////////////////////////////
// CIsosurface implementation.
//////////////////////////////////////////////////////////////////////

inline CIsosurface::CIsosurface() {

	v = 0;
        cube = 0;
        
       	sx = sy = sz = 0;
       	
        treshold = 1.0;	
        
}

inline CIsosurface::~CIsosurface() {

	set_size(0, 0, 0);
	
}

inline void CIsosurface::set_size(int a_sx, int a_sy, int a_sz) {

	if (v) {
		
        	for (int i=0; i<sx; i++) {
                	for (int j=0; j<sy; j++)
                        	delete[] v[i][j];
			delete[] v[i];
		}

                delete[] v;

                v = 0;

                for (int i=0; i<sx-1; i++) {
                	for (int j=0; j<sy-1; j++)
                        	delete[] cube[i][j];
			delete[] cube[i];
		}

                delete[] cube;

                cube = 0;

                sx = sy = sz = 0;

        }

        if (a_sx <= 0 || a_sy <= 0 || a_sz <= 0)
        	return;

	sx = a_sx;
        sy = a_sy;
        sz = a_sz;

        // Create and setup grid.
        
        v = new CIsosurfaceValue**[sx];
        for (int i=0; i<sx; i++) {
        	v[i] = new CIsosurfaceValue*[sy];
                const float x = -1+2*i/(sx-1.0);
                for (int j=0; j<sy; j++) {
                	v[i][j] = new CIsosurfaceValue[sz];
	                const float y = -1+2*j/(sy-1.0);                        
                        for (int k=0; k<sz; k++) {
		                const float z = -1+2*k/(sz-1.0);
                        	v[i][j][k].x = x;
                        	v[i][j][k].y = y;
                        	v[i][j][k].z = z;
                        }
		}
	}

        // Create and setup cubes.
        
        cube = new CIsosurfaceCube**[sx-1];
        for (int i=0; i<sx-1; i++) {
        	cube[i] = new CIsosurfaceCube*[sy-1];
                for (int j=0; j<sy-1; j++) {
			cube[i][j] = new CIsosurfaceCube[sz-1];
                        for (int k=0; k<sz-1; k++) {
                        
                        	// Point to the eight shared corner vertices.
                                
                        	#define P(_p, _x, _y, _z) cube[i][j][k].v[_p] = &v[i+_x][j+_y][k+_z]
                                
				P(0, 0, 0, 0);
                                P(1, 1, 0, 0);
                                P(2, 1, 0, 1);
                                P(3, 0, 0, 1);
                                P(4, 0, 1, 0);
                                P(5, 1, 1, 0);
                                P(6, 1, 1, 1);
                                P(7, 0, 1, 1);
                                
                                #undef P
                                
			}
		}
	}

}

inline void CIsosurface::calculate(CIsosurfaceMetaball* ball, int balls) {

	// Calculate energy values.
	
        for (int i=0; i<sx; i++)
        	for (int j=0; j<sy; j++)
                	for (int k=0; k<sz; k++) {

                        	// FIXME: This could be optimized quite a bit.

                        	float e = 0.0;
                                
                        	for (int b=0; b<balls; b++) {

                                	float dx = v[i][j][k].x - ball[b].x;
                                	float dy = v[i][j][k].y - ball[b].y;
                                	float dz = v[i][j][k].z - ball[b].z;

                                        float d2 = dx*dx + dy*dy + dz*dz;

                                        // E = A / (R*R)
                                        
                                        if (d2 == 0.0)
                                        	e += treshold;
                                        else
	                                        e += ball[b].a / d2;
                                
                                }

                                v[i][j][k].e = e;
                        
                        }
                
}

inline void CIsosurface::calculate_normals() {

	// Calculate normals. We do this for every value in the grid. Maybe should do it only
	// for used points when outputting triangles.
	// FIXME: Interpolating normals linearly changes size.
 
	for (int i=1; i<sx-1; i++)
 		for (int j=1; j<sy-1; j++)
   			for (int k=1; k<sz-1; k++) {
   				const float na = v[i-1][j][k].e - v[i+1][j][k].e;
   				const float nb = v[i][j-1][k].e - v[i][j+1][k].e;
   				const float nc = v[i][j][k-1].e - v[i][j][k+1].e;
//   				v[i][j][k].na = v[i-1][j][k].e - v[i+1][j][k].e;
//				v[i][j][k].nb = v[i][j-1][k].e - v[i][j+1][k].e;
//				v[i][j][k].nc = v[i][j][k-1].e - v[i][j][k+1].e;          								
				#if 1
				float is = 1.0/sqrt(na*na + nb*nb + nc*nc);
				v[i][j][k].na = na * is;
				v[i][j][k].nb = nb * is;
				v[i][j][k].nc = nc * is;
				#endif
			}  
			
}

namespace CIsosurfaceExtern {

	static void (*cb_triangle)(CIsosurfaceVertex* v);

	static void cb_triangles(int num, CIsosurfaceVertex* v) {

		for (int i=0; i<num; i++) {

			// Triangle per triangle output.
        
        		cb_triangle(v);
                
	                v += 3;
                
		}
		
	}

};

inline void CIsosurface::output(void (*triangle)(CIsosurfaceVertex* v)) {

	CIsosurfaceExtern::cb_triangle = triangle;
        
	CIsosurfaceVertex v[300];

        output(100, v, CIsosurfaceExtern::cb_triangles);

}

