#ifndef __cISOSURFACE_h__
#define __cISOSURFACE_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cISOSURFACE.h: CIsosurface implementation. */
//////////////////////////////////////////////////////////////////////

#include "cPOLY.h"

/// Grid value for CIsosurface.
typedef struct {
	float x, y, z;			// Position. Range [-1.0..+1.0].
	float na, nb, nc;		// Normal vector.
        float e;			///< Energy value.
} CIsosurfaceValue;

/// Triangle vertex for CIsosurface.
typedef struct {
	float x, y, z;			///< Position. Range [-1.0..+1.0].
	float na, nb, nc;		///< Normal vector.
} CIsosurfaceVertex;

// Cube definition for CIsosurface.
typedef struct {
	CIsosurfaceValue* v[8];		///< 8 corner points of cube.
} CIsosurfaceCube;

/// Metaball type for CIsosurface.
class CIsosurfaceMetaball {

 public:

	CIsosurfaceMetaball() {
	}

	~CIsosurfaceMetaball() {
	}

 public:

	float x, y, z;		///< XYZ Position.
	float a;		///< Energy. May be negative too.

};

/// Geometry: Iso surface class.
/**
 * Use this class to visualize iso surfaces. Support has been added to calculate energy values for metaballs.
 * The calculation of the generated triangles is done using the marching cubes algoritm.
 * This algoritms is patented, so be carefull using it in a commercial product!<BR>
\code
// Example:

void render() {

	load_camera();
	
	// Create a medium sized iso surface.
	static CIsosurface iso;
	iso.set_size(10, 10, 10);

	// Create a metaball.
	CIsosurfaceMetaball ball;
	ball.x = sin(time);
	ball.y = 0.0;
	ball.z = cos(time);
	ball.a = 1.0;

	iso.calculate(&ball, 1);		// Calculate energy values using metaballs.
	iso.calculate_normals();		// Calculate normals.

	// Output triangles.	
	
	CIsosurfaceVertex v[300];
	glVertexPointer(GL_FLOAT, 3, v, sizeof(CIsosurfaceVertex));
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, v, sizeof(CIsosurfaceVertex));
	glEnableClientState(GL_VERTEX_ARRAY);	

	glBegin(GL_TRIANGLES); {	
		iso.output(100, v, render_triangle);
	} glEnd();
	
}

void render_triangle(int triangles, CIsosurfaceVertex* v) {
	glDrawArrays(0, triangles*3);
}
\endcode
 */
class CIsosurface {

 public:

	CIsosurface();
	~CIsosurface();

 public:

	CIsosurfaceValue*** v;		///< Energy values.
        CIsosurfaceCube*** cube;	///< Cube for marching cubes algo.
	int sx, sy, sz;			///< Size of the grid.
        float treshold;			///< Isosurface border. If energy(x, y, z) >= treshold, (x, y, z) is solid.

 public:

	/**
	 * Sets the size of the grid. Passing (0, 0, 0) will free memory.
	 * @param sx Resolution in x direction.
	 * @param sy Resolution in y direction.
	 * @param sz Resolution in z direction.  	 
	 */
	void set_size(int sx, int sy, int sz);

 public:

	/**
	 * Calculates energy values for a given number of mataballs.
	 * Note that you don't have to use this function to generate energy values.
	 * You can use all kinds of custom functions to do this.
	 * @param ball Pointer to first ball in array.
	 * @param balls The size of the ball array.
	 */
	void calculate(CIsosurfaceMetaball* ball, int balls);
	/**
	 * Call this just before output() to have valid normal values if you need them.
	 */
	void calculate_normals();
	/**
	 * Spawns triangles. The callback function will be called for every triangle with
	 * three metavert_t structures.
	 * @param triangle Callback function that will be called for every generated triangle.
	 */
	void output(void (*triangle)(CIsosurfaceVertex* v));
	/**
	 * Spawns multiple triangles at once. Triangles is the maximum number of triangles
         * to spawn at once. v must be an array of triangles*3 metavert_t structures.
         * The callback function will be called with the number of triangles to render and
         * a pointer to v. Triangles are stored 123.123.123.
         * @param triangles The maximum number of triangles to output at once.
         * @param v An array of vertices large enough to store triangles*3 vertices.
         * #param triangle Callback function that will be called for multiple generated triangles.
	 */
	void output(int triangles, CIsosurfaceVertex* v, void (*triangle)(int num, CIsosurfaceVertex* v));
	/**
	 * Spawn triangles to a mesh. This method isn't recommended for real-time use.
	 * @param mesh Mesh to output to.
	 */
	void output(CMesh& mesh);

};

#include "inline/cISOSURFACE.inl"

#endif
