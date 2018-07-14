#ifndef __Cbezier_h__
#define __Cbezier_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cBEZIER.h: CBezier implementation. */
//////////////////////////////////////////////////////////////////////

#include "CVector.h"
#include "cPOLY.h"

//---------------------------------------------------------------------------

/// Geometry: 3rd degree bzier patch generator class.
/**
 * This bezier patch class uses a 3rd degree function to calculate a patch
 * using 4x4 control points. The patch will touch the four outer control points. The rest
 * of the control points will affect the shape of the patch.
\code
// Example:

void add_bezier(CBsp& bsp) {

	CBezier3 bezier;
	
	// Set corner control points.
	
	bezier.p[0][0].set(-1.0, -1.0, -1.0);
	bezier.p[3][0].set(+1.0, -1.0, -1.0);
	bezier.p[3][3].set(+1.0, +1.0, -1.0);
	bezier.p[0][3].set(-1.0, +1.0, -1.0);
	
	// Generate 10x10 patch.
	
	bezier.generate(10);
	
	// Export to mesh. Remember that CBsp is derived from CMesh.
	
	bezier.to_mesh(bsp);

}
\endcode 
 */
class CBezier3 {

 public:
 
	/**
         * Constructor.
         */ 
	CBezier3() {
		v = 0;
		resolution = 0;
	}
	/**
	 * Destructor.
	 */
 	~CBezier3() {
 		generate(0);
 	}
  
 public:
 
	CVector p[4][4];	///< 4x4 grid of control points.
	
 public:
 	
	CVector** v;		///< Output grid points.
	int resolution;		///< Resolution of output grid.
	
 public:
 
	/**
         * Generates the output grid. The grid will be resolution*resolution sized.
         * If resolution <= 0, all memory will be freed instead.
         * @param resolution The resolution of the generated patch. The generated patch will have resolution*resolution points.
         */ 
	void generate(int a_resolution=7) {
		if (v && a_resolution != resolution) {
			for (int i=0; i<resolution; i++)
				delete[] v[i];
			delete[] v;
  			v = 0;
			resolution = 0;
		}
		
		if (a_resolution < 2)
			return;
			
		resolution = a_resolution;
	   	if (!v) {
			v = new CVector*[resolution];
			for (int i=0; i<resolution; i++)
				v[i] = new CVector[resolution];
		}			
 	
		// Interpolate 3rd degree curved surface.
	
		// t + (1-t) = 1
		// (t + (1-t))**3 = 1**3 = 1
		// a = t
		// b = 1-t
		// (a + b)**3 = 1
		// (a*a + 2*a*b + b*b) * (a + b) = 1
		// a*a*a + a*a*b + 2*a*a*b + 2*a*b*b + a*b*b + b*b*b = 1
		// = a*a*a + 3*a*a*b + 3*a*b*b + b*b*b = 1
		// =   A   +    B    +    C    +   D
		// We use A, B, C and D to scale our 4 control points.
	
		CVector tmp[4][resolution];
		
		// Interpolate vertical.
		for (int i=0; i<4; i++) {
			const float step = 1.0 / (resolution - 1.0);
			float t = 0.0;
			for (int j=0; j<resolution; j++) {
				const float it = 1.0 - t;
				const float a = t * t * t;
				const float b = 3.0 * t * t * it;
				const float c = 3.0 * t * it * it;
				const float d = it * it * it;
				tmp[i][j] = p[i][0] * d + p[i][1] * c + p[i][2] * b + p[i][3] * a;
				t += step;
			}
		}
	
		// Interpolate horizontal.
		for (int i=0; i<resolution; i++) {
			const float step = 1.0 / (resolution - 1.0);
			float t = 0.0;
			for (int j=0; j<resolution; j++) {
				const float it = 1.0 - t;
				const float a = t * t * t;
				const float b = 3.0 * t * t * it;
				const float c = 3.0 * t * it * it;
				const float d = it * it * it;
				v[j][i] = tmp[0][i] * d + tmp[1][i] * c + tmp[2][i] * b + tmp[3][i] * a;
				t += step;
			}			
		}
	}
	/**
	 * Translates all control points by the specified vector. You will need to regenerate
	 * to update the output grid.
	 * @param delta The delta vector will be added to all 4x4 control points.
	 */
	void translate(CVector delta) {
		for (int i=0; i<4; i++)
			for (int j=0; j<4; j++)
				p[i][j] += delta;	
	}

 public:

	/**
	 * Outputs the generated points as triangles.
	 * @param mesh Target mesh to output triangles.
	 */
	void to_mesh(CMesh& mesh) {
		if (!v)
        		return;
                
		for (int i=0; i<resolution-1; i++)
        		for (int j=0; j<resolution-1; j++) {
	                	#define V(_p, _v) \
        	                	{ \
	        	                	CEdge* edge = _p->add(new CEdge); \
						edge->p[0] = _v[0]; \
						edge->p[1] = _v[1]; \
						edge->p[2] = _v[2]; \
	                                }
	                	CPoly* p1 = mesh.add(new CPoly);
	                        V(p1, v[i+0][j+0]);
         	                V(p1, v[i+1][j+0]);
                	        V(p1, v[i+1][j+1]);
	                        p1->init();
	                        CPoly* p2 = mesh.add(new CPoly);
	                        V(p2, v[i+0][j+0]);
        	                V(p2, v[i+1][j+1]);
                	        V(p2, v[i+0][j+1]);
	                        p2->init();
				#undef V
	                }
	}
        
};

//---------------------------------------------------------------------------

#endif
