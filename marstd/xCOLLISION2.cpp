// This defines an alternate collision detection algo for sphere -> poly.

// --------------------
//
// collision.cpp
//
// Copyright (C) 1995-2001 Volition, Inc.
// All rights reserved
//
// This code is meant for illustrative purposes only.  Volition takes 
//  no responsibility for any Bad Things(TM) that happen as a result 
//  of using it.  Use at your own risk.
//
// --------------------

#include "marx.h"

// given an edge of a polygon and a moving sphere, find the first contact the sphere
//  makes with the edge, if any.  note that hit_time must be primed with a  value of 1
//  before calling this function the first time.  it will then maintain the closest 
//  collision in subsequent calls.
//
// xs0:			start point (center) of sphere
// vs: 			path of sphere during frame
// rad:			radius of sphere
// v0:			vertex #1 of the edge
// v1:			vertex #2 of the edge
// hit_time:	(OUT) time at which sphere collides with polygon edge
// hit_point:	(OUT) point on edge that is hit
//
// returns - whether the edge (or it's vertex) was hit
bool collide_edge_sphereline(CVector& xs0, CVector& vs, float rad, CVector& v0, CVector& v1, float& hit_time, CVector& hit_point) {

	static CVector temp_sphere_hit;
	bool try_vertex = false;		// Assume we don't have to try the vertices.

	CVector ve = v1 - v0;
	CVector delta = xs0 - v0;
	double delta_dot_ve = delta * ve;
	double delta_dot_vs = delta * vs;
	double delta_sqr = delta.size2();
	double ve_dot_vs = ve * vs;
	double ve_sqr = ve.size2();
	double vs_sqr = vs.size2();

	double temp;

	// position of the collision along the edge is given by: xe = v0 + ve*s, where s is
	//  in the range [0,1].  position of sphere along its path is given by: 
	//  xs = xs + vs*t, where t is in the range [0,1].  t is time, but s is arbitrary.
	//
	// solve simultaneous equations
	// (1) distance between edge and sphere center must be sphere radius
	// (2) line between sphere center and edge must be perpendicular to edge
	//
	// (1) (xe - xs)*(xe - xs) = rad*rad
	// (2) (xe - xs) * ve = 0
	//
	// then apply mathematica

	double A, B, C, root, discriminant;
	double root1 = 0.0f;
	double root2 = 0.0f;
	A = ve_dot_vs * ve_dot_vs - ve_sqr * vs_sqr;
	B = 2 * (delta_dot_ve * ve_dot_vs - delta_dot_vs * ve_sqr);
	C = delta_dot_ve * delta_dot_ve + rad * rad * ve_sqr - delta_sqr * ve_sqr;

	if((A > -0.0001f && A < 0.0001f && 0) || A == 0.0) {
		// degenerate case, sphere is traveling parallel to edge
		try_vertex = true;
	} else {
		discriminant = B*B - 4*A*C;
		if( discriminant > 0 ) {
			root = (float)sqrt(discriminant);
			root1 = (-B + root) / (2 * A);
			root2 = (-B - root) / (2 * A);

			// sort root1 and root2, use the earliest intersection.  the larger root 
			//  corresponds to the final contact of the sphere with the edge on its 
			//  way out.
			if( root2 < root1 ) {
				temp = root1;
				root1 = root2;
				root2 = temp;
			}

			// root1 is a time, check that it's in our currently valid range
			if( (root1 < 0) || (root1 >= hit_time) ) {
				return false;
			}

			// find sphere and edge positions
			temp_sphere_hit = xs0 + vs * root1;

			// check if hit is between v0 and v1
			float s_edge = ((temp_sphere_hit - v0) * ve) / ve_sqr;
			if( (s_edge >= 0) && (s_edge <= 1) ) {
				// bingo
				hit_time = root1;
				hit_point = v0 + ve * s_edge;
				return true;
			}
		} else {
			// discriminant negative, sphere passed edge too far away
			return false;
		}
	}
	
	// sphere missed the edge, check for a collision with the first vertex.  note
	//  that we only need to check one vertex per call to check all vertices.
		A = vs_sqr;
		B = 2 * delta_dot_vs;
		C = delta_sqr - rad * rad;

		discriminant = B*B - 4*A*C;
		if( discriminant > 0 ) {
			root = (float)sqrt(discriminant);
			root1 = (-B + root) / (2 * A);
			root2 = (-B - root) / (2 * A);

			// sort the solutions
			if( root1 > root2 ) {
				temp = root1;
				root1 = root2;
				root2 = temp;
			}

			// check hit vertex is valid and earlier than what we already have
			if( (root1 < 0) || (root1 >= hit_time) ) {
				return false;
			}
		} else {
			// discriminant negative, sphere misses vertex too
			return false;
		}

	// bullseye
	hit_time = root1;
	hit_point = v0;
	return true;
}


#if 0

// determines a collision between a ray and a sphere
//
// ray_start:	the start pos of the ray
// ray_dir:		the normalized direction of the ray
// length:		length of ray to check
// sphere_pos:	sphere position
// sphere_rad:	sphere redius
// hit_time:	(OUT) if a collision, contains the distance from ray.pos
// hit_pos:		(OUT) if a collision, contains the world point of the collision
//
// returns: true if a collision occurred
//
bool collide_ray_sphere( vector &ray_start, vector &ray_dir, float length, vector &sphere_pos, float sphere_rad, float &hit_time, vector &hit_pos )
{
	// get the offset vector
	vector offset = sphere_pos - ray_start;

	// get the distance along the ray to the center point of the sphere
	float ray_dist = ray_dir * offset;
	if( ray_dist <= 0 || (ray_dist - length) > sphere_rad) {
		// moving away from object or too far away
		return false;
	}

	// get the squared distances
	float off2 = offset * offset;
	float rad2 = sphere_rad * sphere_rad;
	if( off2 <= rad2 ) {
		// we're in the sphere
		hit_pos = ray_start;
		hit_time = 0;
		return true;
	}

	// find hit distance squared
	float d = rad2 - (off2 - ray_dist * ray_dist);
	if( d < 0 ) {
		// ray passes by sphere without hitting
		return false;
	}

	// get the distance along the ray
	hit_time = (float)(ray_dist - sqrt( d ));
	if( hit_time > length ) {
		// hit point beyond length
		return false;
	}

	// sort out the details
	hit_pos = ray_start + ray_dir * hit_time;
	hit_time /= length;
	return true;
}

#endif

bool MARX::collision(CPoly& poly, CSphere& sphere, CVector& delta, float& tout, CVector& normal) {

	#if 1

        CVector point;
        {
        	float distance = poly.plane * sphere.position;
        	CVector offset = poly.plane.normal * sphere.radius;
                if (distance > 0.0)
                	point = sphere.position - offset;
		else
                	point = sphere.position + offset;
	}

	float d1 = poly.plane * point;
        float d2 = poly.plane * (point + delta);

        tout = 1.0;
        
        if (1) {

        	float t;
                if (d1 == 0.0)
                	 t = 0.0;
		else if (d2 == 0.0)
			t = 1.0;
		else
        		t = - d1 / (d2 - d1);
                if (t >= 0.0 && t <= 1.0) {
			CVector impact = point + delta * t;
        		if (MARX::collision(poly, impact)) {
	        		tout = t;
	        	        normal = poly.plane.normal;
			}
		}
	}

        #endif

        // Next try collision with edges.

        CEdge* edge = poly.edge_head;
        while (edge) {
        	CVector tmp;
		CEdge* next = edge->next ? edge->next : poly.edge_head;
		if (collide_edge_sphereline(sphere.position, delta, sphere.radius, edge->p, next->p, tout, tmp)) {
                        normal = sphere.position - tmp;
			normal.normalize();
		}
		edge = edge->next;
        }

	if (tout < 1.0)
        	return true;
                
        return false;

}
