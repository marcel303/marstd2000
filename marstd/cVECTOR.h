#ifndef __cVECTOR_h__
#define __cVECTOR_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cVECTOR.h: CVector implementation. */
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include "cDEBUG.h"

//---------------------------------------------------------------------------

/// Geometry: 3D vector type.
/**
 * A vector has three components (x, y, z). These components can be accessed using operator[].
 * Vectors are commonly used to represent rotation, velocity, position, acceleration, force, etc.
\code
// Example:

CVector position, velocity, acceleration;
float friction_constant = 0.1;			// Velocity is halved every second.

void update_physics(float dt) {

	velocity += acceleration * dt;
	position += velocity * dt;
	
	velocity *= pow(fraction_constant, dt);

}
\endcode 
 */
class CVector {

 public:
 
	/**
	 * Default constructor. Initializes all three components to 0.0.
	 */
	CVector() {
		v[0] = v[1] = v[2] = 0.0;
	}
	/**
	 * Constructor with explicit initial values for (x, y, z).
	 */
	CVector(float x, float y, float z) {
		v[0] = x;
		v[1] = y;
		v[2] = z;
	}
	~CVector() {
	}
	
 public:
 
	float v[4];
  	
 public:
 
	/**
     * Convenient way to set all three components at once.
     * @param x New x value.
     * @param y new y value.
     * @param z new z value.
     */
 	void set(float x, float y, float z) {
 		v[0] = x;
 		v[1] = y;
 		v[2] = z;
 	}
 	/**
 	 * @return The magnitude of the vector, using pythagoras' theorem.
 	 */
	float size() {
		return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	}  
 	/**
     * @return The magnitude of the vector squared. This is faster than size() because it doesn't do a sqrt().
     * @see size()
     */
	float size2() {
		return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	}
	/**
	 * Makes the vector a unit vector if size() != 0.0. Leaves vector unchanged if 0.0.
	 * @see size()
	 */
	void normalize() {
		float size = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
		MASSERT(size, "");		
		if (size == 0.0)
			return;
		float isize = 1.0/size;			
		v[0] *= isize;
		v[1] *= isize;
		v[2] *= isize;  		
	}
	/**
	 * Returns a vector that equals this + vector * scale. Use it to interpolate vectors.
	 * @param vector Vector to add to this vector. Delta when interpolating vectors.
	 * @param scale Scaling factor to scale vector.
	 */
	CVector MA(CVector& vector, float scale) {
		CVector tmp;
		tmp.v[0] = v[0] + scale*vector.v[0];
		tmp.v[1] = v[1] + scale*vector.v[1];
		tmp.v[2] = v[2] + scale*vector.v[2];  		
		return tmp;
	}

 public:

	/**
	 * Copies vector into this vector.
	 */
	void operator=(CVector vector) {
		v[0] = vector.v[0];
		v[1] = vector.v[1];
		v[2] = vector.v[2];  		
	}
	/**
	 * Vector subtraction. vout[i] = v1[i] - v[2].
	 */
	CVector operator-(CVector vector) {
		CVector tmp;
		tmp.v[0] = v[0] - vector.v[0];
		tmp.v[1] = v[1] - vector.v[1];
		tmp.v[2] = v[2] - vector.v[2];  		
		return tmp;
	} 
	/**
	 * Vector addition. vout[i] = v1[i] + v2[i].
	 */
	CVector operator+(CVector vector) {
		CVector tmp;
		tmp.v[0] = v[0] + vector.v[0];
		tmp.v[1] = v[1] + vector.v[1];
		tmp.v[2] = v[2] + vector.v[2];  		
		return tmp;
	}
	/**
	 * Subtract vector from this vector.
	 */
	CVector& operator-=(CVector& vector) {
		v[0] -= vector.v[0];
		v[1] -= vector.v[1];
		v[2] -= vector.v[2];  		
		return *this;
	}
	/**
	 * Adds vector to this vector.
	 */
	CVector& operator+=(CVector& vector) {
		v[0] += vector.v[0];
		v[1] += vector.v[1];
		v[2] += vector.v[2];  		
		return *this;
	}
 	/**
     * Vector scaling. vout[i] = v[i] * scale.
     */
	CVector operator*(float scale) {
		CVector tmp;
		tmp.v[0] = v[0] * scale;
		tmp.v[1] = v[1] * scale;
		tmp.v[2] = v[2] * scale;
  		return tmp;  		
	}
	/**
	 * Vector scaling inverse. vout[i] = v[i] * 1/scale.
	 */
	CVector operator/(float scale) {
		CVector tmp;
		tmp.v[0] = v[0] / scale;
		tmp.v[1] = v[1] / scale;
		tmp.v[2] = v[2] / scale;
  		return tmp;  		
	}
	/**
	 * Multiplies this vector by scaling factor.
	 */
	CVector& operator*=(float scale) {
		v[0] = v[0] * scale;
		v[1] = v[1] * scale;
		v[2] = v[2] * scale;
  		return *this;  		
	}
	/**
	 * Divides this vector by scaling factor.
	 */
	CVector& operator/=(float scale) {
		MASSERT(scale, "");	
		float iscale = 1.0/scale;
		v[0] = v[0] * iscale;
		v[1] = v[1] * iscale;
		v[2] = v[2] * iscale;
  		return *this;  		
	}
	/**
	 * Negates this vector. vout[i] = -v[i].
	 */
 	CVector operator-() {
 		CVector tmp;
 		tmp.v[0] = -v[0];
 		tmp.v[1] = -v[1];
 		tmp.v[2] = -v[2];    		
		return tmp;
	}
	/**
	 * Returns the dot product of this vector and vector.
	 * dot = a[0]*b[0] + a[1]*b[1] + a[2]*b[2].
	 */
	float operator*(CVector& vector) {

	        #if defined(MARSTD_3DNOW)	// Is slower here. Clobbers entire FPU stack + femms*2

                float tmp;
                
                __asm__ __volatile__ (


                	"femms			\n\t"
                        "movq (%%eax), %%mm0	\n\t"
			"movd 8(%%eax), %%mm1	\n\t"
                        "pfmul (%%ebx), %%mm0	\n\t"
                        "movd 8(%%ebx), %%mm2	\n\t"			                        
                        "pfmul %%mm2, %%mm1	\n\t"
                        "pfacc %%mm0, %%mm0	\n\t"
                        "pfadd %%mm1, %%mm0	\n\t"
                        "movd %%mm0, (%%ecx)	\n\t"
                        "femms			    "

                        :
                        : "eax" (v), "ebx" (vector.v), "ecx" (&tmp)
                        : "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
                
                );

                return tmp;
                
                #else
                
		return v[0]*vector.v[0] + v[1]*vector.v[1] + v[2]*vector.v[2];
                
                #endif
                
	}
	/**
	 * Returns the cross product of this vector and vector.
	 * The resulting vector is perpendicular to both input vectors.
	 */
 	CVector operator%(CVector& vector) {
 		CVector tmp;
		tmp.v[0] = v[1]*vector.v[2] - v[2]*vector.v[1];
		tmp.v[1] = v[2]*vector.v[0] - v[0]*vector.v[2];
		tmp.v[2] = v[0]*vector.v[1] - v[1]*vector.v[0]; 	
		return tmp;
	}
	/**
	 * Vector multiplication by vector. vout[i] = a[i] * b[i].
	 */
 	CVector operator|(CVector& vector) {
 		CVector tmp;
 		tmp.v[0] = v[0] * vector.v[0];
 		tmp.v[1] = v[1] * vector.v[1];
 		tmp.v[2] = v[2] * vector.v[2];    		
 		return tmp;
	}
 	/**
 	 * Returns component[index].
	 */	
	float& operator[](int index) {
		return v[index];
	}
	/**
     * Returns a float* pointer to the internal representation of the vector.
     */
	operator float*() {
		return v;
	}
	/**
	 * Returns true if vector is exactly the same as this vector.
	 */
	bool operator==(CVector& vector) {
		for (int i=0; i<3; i++) {
			if (v[i] != vector.v[i])
				return false;
		}
		return true;
	}
	/**
	 * Returns true if vector is not the same as this vector.
	 */ 	
	bool operator!=(CVector& vector) {
		for (int i=0; i<3; i++) {
			if (v[i] != vector.v[i])
				return true;
		}
		return false;
	}
	
};

//---------------------------------------------------------------------------

#endif
