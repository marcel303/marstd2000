#ifndef __cMATRIX_h__
#define __cMATRIX_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cMATRIX.h: CMatrix implementation. */
//////////////////////////////////////////////////////////////////////

// FIXME: Use custom matrix functions, not Allegro's ones.
// FIXME: Use degress instead of radians? (OpenGL like).
// FIXME: Use true 4x4 matrix, not allegro hack. Same ordering as OpenGL.

#include <allegro.h>
#include "cMATH.h"
#include "cVECTOR.h"

//---------------------------------------------------------------------------

#define MATRIX_STACK_SIZE	10	///< Max matrix stack depth.

//---------------------------------------------------------------------------

class CMatrix3 {

 public:

	CMatrix3();
	~CMatrix3();

 public:

	float v[9];

 public:

	void make_identity();
	void transpose();
	void make_rotation(float x, float y, float z);
	void make_scaling(float x, float y, float z);
    void mul(float x, float y, float z, float* xout, float* yout, float* zout);
	
 public:
 
 	float& operator()(int x, int y);
 	CVector operator*(CVector& vector);
	CMatrix3& operator*=(CMatrix3& matrix);  	

};

/// Geometry: 4x4 matrix.
/**
 * NOTE: 3D vectors are padded with 1.0. So eg the 3D vector (1.0, 2.0, 3.0) becomes the 4D vector (1.0, 2.0, 3.0, 1.0). This makes multiplication by a 4x4 matrix possible and adds the translation part.
 */
class CMatrix4 {

 public:
 
	CMatrix4();
 	~CMatrix4();
  
 public:
 
	float v[16];
     
 public:
 
	void make_identity();
 	void transpose();
  	void make_x_rotation(float r);
  	void make_y_rotation(float r);
  	void make_z_rotation(float r);
  	void make_rotation(float x, float y, float z);
   	void make_scaling(float x, float y, float z);
    void make_translation(float x, float y, float z);
    void mul(float x, float y, float z, float* xout, float* yout, float* zout);
    void mul3(float x, float y, float z, float* xout, float* yout, float* zout);

 public:

	float operator()(int x, int y);
	CVector operator*(CVector& vector);
	CMatrix4& operator*=(CMatrix4& matrix); 

};

//---------------------------------------------------------------------------

/// Geometry: Matrix stack.
/**
 * The matrix class is currently just a wrapper for Allegro's matrix functions + stack.
 * Matrices are used to transform geometry (vectors). Matrices can be concatenated
 * to create hierarchical transformations.
 * Both vector and float versions exist of every function to make programming a bit more comfortable.
\code
// Example:

CMatrix matrix;
CVector position, rotation;
CTriangle* triangle;		// Just three points.
int triangles;			// Number of points.

void render_scene() {

	matrix.push();
	matrix.identity();
	matrix.rotate(rotation);
	matrix.translate(position);
	
	for (int i=0; i<triangles; i++) {
		CVector tmp[3];
		for (int j=0; j<3; j++)
			matrix.apply(triangle[i][j], tmp[j]);
		render_triangle(tmp);	
	}
	
	matrix.pop();

}
\endcode 
 */
 
class CMatrix {

 public:
 
	CMatrix() {
		depth = 0;
		identity();
	} 
	~CMatrix() {
	}
	
 public:
 
	MATRIX_f m[MATRIX_STACK_SIZE];  	///< Matrix stack.
	float v[MATRIX_STACK_SIZE][4][4];		///< 4x4 Matrix.
	int depth;				///< Matrix stack depth.
	MATRIX_f tmp;				///< Temp matrix used by mul()
	
 public:
 
	/**
     * Pushes the current matrix on the stack.
     * @return The current stack depth.
     * @see pop
     */
	int push() {
		m[depth+1] = m[depth];
		depth++;
		return depth;
	} 
 	/**
     * Pops the last pushed matrix from the stack.
     * @return The current stack depth.
     * @see push
     */
	int pop() {
		depth--;
		return depth;
	}
	/**
	 * Multiplies the current matrix by tmp.
	 * @see tmp
	 */
	void mul() {
		matrix_mul_f(&tmp, &m[depth], &m[depth]);
	}
	
 public:
 	
	/**
 	 * Loads the current matrix with the identity matrix.
   	 */ 	
	void identity() {
		m[depth] = identity_matrix_f;
	}
	/**
	 * Scales the matrix.
	 * @param x Factor to scale x axis.
	 * @param y Factor to scale y axis.
	 * @param z Factor to scale z axis.  	 
	 */
	void scale(float x, float y, float z) {
		get_scaling_matrix_f(&tmp, x, y, z);
		mul();
	}
	/**
	 * Scales the matrix using vector[0], vector[1], vector[2].
	 * @param vector Vector with (x, y, z) scaling values.
	 * @see scale()
	 */
	void scale(CVector vector) {
		scale(vector[0], vector[1], vector[2]);
	}
	/**
	 * Translates x, y, z along x, y and z axis.
	 * @param x Amount to translate along x axis.
	 * @param y Amount to translate along y axis.
	 * @param z Amount to translate along z axis.  	 
	 */
	void translate(float x, float y, float z) {
		get_translation_matrix_f(&tmp, x, y, z);
		mul();
	}
	/**
	 * Translates by vector[0], vector[1] and vector[2].
	 * @param vector Vector with (x, y, z) translation.
	 * @see translate()
	 */
	void translate(CVector vector) {
		translate(vector[0], vector[1], vector[2]);
	}
	/**
	 * Rotates the matrix around the x, y and z axis by specified amounts in radians.
	 * @param x Rotation around the x axis.
	 * @param y Rotation around the y axis.
	 * @param z Rotation around the z axis.  	 	 
	 */	
	void rotate(float x, float y, float z) {
		get_rotation_matrix_f(&tmp, RAD2BIN(x), RAD2BIN(y), RAD2BIN(z));
		mul();
	}
	/**
	 * @param vector Vector with (x, y, z) rotation.	
	 * @see rotate()
	 */
	void rotate(CVector vector) {
		rotate(vector[0], vector[1], vector[2]);
	}
	/**
	 * Scales, rotates and traslates matrix.
	 * @param s* Scaling factors.
	 * @param t* Translation.
	 * @see scale()
	 * @see translate()
	 */
	void st(float sx, float sy, float sz, float tx, float ty, float tz) {
		translate(tx, ty, tz);	
		scale(sx, sy, sz);
	}	 
	/**
	 * Clears to rotation part of the matrix. This will still translate to points, but won't rotate them.
	 * Use it to implement billboarding.
	 * @see clear_translation()
	 */
	void clear_rotation() {
	}
	/**
	 * Clears the translation part of the matrix. The new matrix will only rotate points.
	 * Use it to rotate eg. normal vectors.
	 * @see clear_rotation()
	 */
	void clear_translation() {
	}
	/**
	 * Applies the current matrix to (x, y, z), storing the result in xout, yout and zout.
	 * @param x X component to transform.
	 * @param y Y component to transform.
	 * @param z Z component to transform.  	 
	 * @param xout Variable to store new x value.
	 * @param yout Variable to store new y value.
	 * @param zout Variable to store new z value.  	 
	 */
	void apply(float x, float y, float z, float& xout, float& yout, float& zout) {
		apply_matrix_f(&m[depth], x, y, z, &xout, &yout, &zout);
	}
	/**
	 * @param v Vector to transform.
	 * @param vout Vector to store result.
	 * @see apply()
	 */
	void apply(CVector& v, CVector& vout) {
		apply_matrix_f(&m[depth], v[0], v[1], v[2], &vout[0], &vout[1], &vout[2]);
	}
 
};

#include "inline/cMATRIX.inl"

//---------------------------------------------------------------------------

#endif
