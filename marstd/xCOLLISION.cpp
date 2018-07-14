//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file xCOLLISION.cpp: MARX collision detection routines. */
//////////////////////////////////////////////////////////////////////

#include "marx.h"
#include "xCOLLISION3.h"

bool MARX::collision(CPoly& poly, CVector& point) {

	CEdge* edge = poly.edge_head;
	
	while (edge) {
	
		float d = edge->plane * point;
		
		if (d > 0.0)
			return false;
		
		edge = edge->next;
	
	}
	
	return true;

}
	
bool MARX::collision(CPoly& poly, CVector& point, CVector& delta, float& t) {

	float d1 = poly.plane * point;
	float d2 = poly.plane * (point + delta);
	
	if (d1 < 0.0 && d2 < 0.0)
		return false;
	if (d1 > 0.0 && d2 > 0.0)
		return false;
	
  	if (d1 == 0.0)
   		t = 0.0;
     	else if (d2 == 0.0)
      		t = 1.0;
        else	
		t = - d1 / (d2 - d1);

	CVector impact = point + delta * t;

        CVector tmp = delta;
        tmp.normalize();
	t = 1.0 - fabs(tmp * poly.plane.normal);
	
	if (MARX::collision(poly, impact))
		return true;
		
	return false;		

}

bool MARX::collision(CPoly& poly, CSphere& sphere) {

	float distance = poly.plane * sphere.position;
	
	if (distance < -sphere.radius || distance > sphere.radius)
		return false;
		
	CSphere circle;
	circle.position = sphere.position - poly.plane.normal * distance;
 	circle.radius = sqrt(sphere.radius*sphere.radius - distance*distance);
  
	if (MARX::collision(poly, circle.position))
		return true;
		
	CEdge* edge = poly.edge_head;
 	while (edge) {
 		float distance = edge->plane * circle.position;
		if (distance >= 0.0 && distance <= circle.radius) {
	 		CVector p = circle.position - edge->plane.normal * distance;
	 		CVector p1 = edge->p;
	 		CVector p2 = edge->edge2->p;
	 		CVector pn = p2 - p1;
			float pd = p1 * pn;
			float d1 = p1 * pn - pd;
			float d2 = p2 * pn - pd;
			float d = p * pn - pd;
			if (d >= d1 && d <= d2)
				return true;
		}
		CVector delta = edge->p - sphere.position;
		if (delta.size2() <= sphere.radius*sphere.radius)
			return true;
 		edge = edge->next;
	}
 
   	return false;		

}

static bool point_in_planes(CPlane* plane, int planes, CVector& point, int exclude) {

	for (int i=0; i<planes; i++)
        	if (i != exclude)
                	if (plane[i] * point > 0.0)
                        	return false;

        return true;

}

bool MARX::collision(CPoly& poly, float extrusion, CVector& point, CVector& delta, float& t, CVector& normal) {

	float best_t = -1.0;
        const int planes = poly.edge_count * 2 + 2;

	CPlane plane[planes];

        // Setup top and bottom planes.

        plane[0] = poly.plane;
        plane[0].distance += extrusion;
        plane[1] = -poly.plane;
        plane[1].distance += extrusion;

        if ((plane[0] * point > 0.0) && (plane[0] * (point + delta) > 0.0))
        	return false;
        if ((plane[1] * point > 0.0) && (plane[1] * (point + delta) > 0.0))
        	return false;                        

	CEdge* edge = poly.edge_head;

        // Setup edge planes.
        
        for (int i=0; i<poly.edge_count; i++) {

        	plane[2+i*2] = edge->plane;
                plane[2+i*2].distance += extrusion;

                CEdge* prev = edge->prev ? edge->prev : poly.edge_tail;
                
                plane[3+i*2].normal = (edge->plane.normal + prev->plane.normal) * 0.5;
                plane[3+i*2].normal.normalize();	// LERP may change magnitude.
                CVector point = edge->p + plane[3+i*2].normal * extrusion;
                plane[3+i*2].distance = plane[3+i*2].normal * point;

                edge = edge->next;
        
        }

        // Cycle through planes and test for collisions.

        for (int i=0; i<planes; i++) {

//        	float dot = delta * plane[i].normal;
                float d1 = plane[i] * point;

                if (/*dot < 0.0 &&*/ d1 >= 0.0) {

	                float d2 = plane[i] * (point + delta);

                        if (d2 <= 0.0) {

                        	if (d1 == 0.0)
                                	t = 0.0;
				else if (d2 == 0.0)
                                	t = 1.0;
				else
					t = - d1 / (d2 - d1);

                                if (t < best_t || best_t < 0.0) {
                                
	                                CVector impact = point + delta * t;

                                        if (point_in_planes(plane, planes, impact, i)) {

                                        	normal = plane[i].normal;
                                                best_t = t;
                                        
                                        }
                                        
				}
                        
                        }
                
                }
        
        }

        if (best_t < 0.0)
        	return false;
	else
	        return true;

}

#if 0

bool MARX::collision(CPoly& poly, CSphere& sphere, CVector& delta, float& t, CVector& normal) {

	#if 1
	
	// Very simple, yet not working collision test.

        CVector tmp = delta;
        tmp.normalize();
	t = 1.0 - fabs(tmp * poly.plane.normal);
	
	CSphere sphere2 = sphere;
	sphere2.position += delta;	// Adjust position.
	
	// A: Plane proximity.
	
	float distance = poly.plane * sphere2.position;
	if (distance < 0.0)
		distance = -distance;
	
	if (distance > sphere2.radius)
		return false;
		

       	CEdge* edge = poly.edge_head;
       	
       	while (edge) {
       	
       		float distance = edge->plane * sphere2.position;
       		
		if (distance > sphere2.radius)
       			return false;
       		
       		edge = edge->next;
       	
       	}
       	
       	return true;
	
	#else
	
	// A: Sphere -> poly. Test nearest point.

	float distance = poly.plane * sphere.position;
	
	CVector point;
	
	if (distance < 0.0)
		point = sphere.position + poly.plane.normal * sphere.radius;
	else
 		point = sphere.position - poly.plane.normal * sphere.radius;		
 		
	if (MARX::collision(poly, point, delta, t)) {
		normal = poly.plane.normal;
		return true;
	}
	
	// B: Sphere -> edge. Test for collision with every edge in polygon. If collision, only update t and normal if t < best_t.
	
	CEdge* edge = poly.edge_head;
	
	while (edge) {
	
		edge = edge->next;
	
	}
	
	return false;
	
	#endif

}

#else

bool MARX::collision(CPoly& poly, CSphere& sphere, CVector& delta, float& t, CVector& normal) {

	CVector impact;
	
	if (!sp_collision(sphere, delta, &poly, impact, t))
		return false;

	CVector tmp = sphere.position + delta * t;
	
	normal = tmp - impact;
	normal.normalize();
	
  	return true;

}

#endif
