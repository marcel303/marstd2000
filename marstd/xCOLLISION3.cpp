#include "marstd.h"

#define EPS	0

static bool point_collision(CSphere sphere, CVector delta, CVector point, CVector& impact, float& t) {

	// !!WARNING!! IT MIGHT BE A GOOD IDEA TO SKIP THE FOLLOWING..
	
	// d = sphere.position + delta * t - point
	// solve t for d.size() == r
	
	// d.x(t) = sx + dx * t - px
	// d.y(t) = sy + dy * t - py
	// d.z(t) = sz + dz * t - pz
	
	// d.x(t)*d.x(t) + d.y(t)*d.y(t) + d.z(t)*d.z(t) = r*r
	
	// d.x(t)*d.x(t) =
	// (sx + dx * t - px) * (sx + dx * t - px) =
	// sx*sx + px*px - 2*sx*px + 2*sx*dx*t - 2*px*dx*t + dx*dx*t*t
	
	// =>
	// 2*t*(sx*dx - px*dx + sy*dy - py*dy + sz*dz - pz*dz) +
	// t*t*(dx*dx + dy*dy + dz*dz) +
	// sx*sx + px*px - 2*sx*px +
	// sy*sy + py*py - 2*sy*py +
	// sz*sz + pz*pz - 2*sz*pz -
	// r*r = 0
	
	// ABC formula:

	// A = dx*dx + dy*dy + dz*dz	
	// B = 2*(sx*dx - px*dx + sy*dy - py*dy + sz*dz - pz*dz)	
	// C = sx*sx + px*px - 2*sx*px +
	//     sy*sy + py*py - 2*sx*py +
	//     sz*sz + pz*pz - 2*sz*pz - r*r
	
	// D = B*B - 4*A*C

	// R = - B +/- SQRT(D)
	//     ---------------
	//           2*A
	
	#if 1
	
    	const float A = delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2];	
	
	const float B = 2 * 
 		(sphere.position[0]*delta[0] - point[0]*delta[0] +
   		 sphere.position[1]*delta[1] - point[1]*delta[1] + 
      		 sphere.position[2]*delta[2] - point[2]*delta[2]);
      		 
	const float C =
 		sphere.position[0]*sphere.position[0] + point[0]*point[0] - 2*sphere.position[0]*point[0] +
 		sphere.position[1]*sphere.position[1] + point[1]*point[1] - 2*sphere.position[1]*point[1] +
 		sphere.position[2]*sphere.position[2] + point[2]*point[2] - 2*sphere.position[2]*point[2] - sphere.radius*sphere.radius;      		 
 		
	#else
 	
 	// If you look at the code above, you'll see most stuff is just dot products..
 	
 	const float A = delta*delta;
 	const float B = 2 * (sphere.position*delta - point*delta);
 	const float C = (sphere.position*sphere.position) + (point*point) - 2*(sphere.position*point) - sphere.radius*sphere.radius;
 
    	#endif 		
 
        const float D = B*B - 4*A*C;
        
        // Point will never hit sphere.
        
        if (A == 0) {
        	// Savety precuation to keep sphere away from point.
        	// Don't know if this is so usefull though.
        	CVector tmp = sphere.position - point;
        	if (tmp.size2() <= sphere.radius*sphere.radius) {
        		allegro_message("Warning: point_collision: A == 0 && delta <= radius");
        		t = 0;
        		impact = point;
        		return true;
		}        		
        }
        
	if (A == 0 || D < 0)
 		return false;            		
 	
   	const float root = sqrt(D);
    	
	const float time1 = (-B - root) / (2 * A); 		
	const float time2 = (-B + root) / (2 * A);
	
	t = MIN(time1, time2);
	
	if (t >= 0 && t <= 1) {
		impact = point;		
		return true;
	}		
	
	return false;

}
static bool point_on_edge(CVector point, CVector edge_v1, CVector edge_v2) {

	CPlane edge_plane;
	
	edge_plane.normal = edge_v2 - edge_v1;
	edge_plane.distance = edge_plane.normal * edge_v1;
	
	const float max_distance = edge_plane * edge_v2;
	
	const float distance = edge_plane * point;
	
	if (distance >= 0 && distance <= max_distance)
		return true;

	return false;		

}

static bool circle_collision(CSphere circle, CVector delta, CVector edge_v1, CVector edge_v2, CPlane slide_plane, CVector& a_impact, float& a_t) {

	CVector edge_delta = edge_v2 - edge_v1;

	CPlane edge_plane;
 	edge_plane.normal = edge_delta % slide_plane.normal;
	edge_plane.distance = edge_plane.normal * edge_v1;
	edge_plane.normalize();
	
	const float circle_distance = edge_plane * circle.position;
	
	if (fabs(circle_distance) > circle.radius) {
	
		CVector circle_point;
		
 		if (circle_distance >= 0)
	 		circle_point = circle.position - edge_plane.normal * circle.radius;
		else
	 		circle_point = circle.position + edge_plane.normal * circle.radius;
	 	
    		const float point_distance1 = edge_plane * circle_point;
    		const float point_distance2 = edge_plane * (circle_point + delta);
    		
    		if (point_distance1 < 0 && point_distance2 < 0)
    			return false;
		if (point_distance1 > 0 && point_distance2 > 0)
			return false;
			
     		float t;
     		
		if (point_distance1 == point_distance2)
			t = 0;
		else if (point_distance1 == 0)
  			t = 0;
		else if (point_distance2 == 0)
  			t = 1;     			
		else			
  			t = - point_distance1 / (point_distance2 - point_distance1);
  			
		if (t < 0 || t > 1) {
			allegro_message("Warning: circle_collision: t < 0 || t > 1");
  			t = 0;
		}  			
  		
		CVector impact = circle_point + delta * t;
  	
       		if (point_on_edge(impact, edge_v1, edge_v2)) {
       			a_impact = impact;
       			a_t = t;
         		return true;  			
		}         			

	}	 		

	return false;
 	
}

bool collision(CSphere sphere, CVector delta, CVector edge_v1, CVector edge_v2, CVector& impact, float& t) {

	// Moving edge 2 static sphere.
	
	CVector edge_delta = edge_v2 - edge_v1;
	
	CPlane slide_plane;

	slide_plane.normal = delta % edge_delta;
	slide_plane.distance = slide_plane.normal * edge_v1;
	slide_plane.normalize();
	
	const float sphere_distance = slide_plane * sphere.position;
	
	// Edge will never hit sphere.
	
	if (fabs(sphere_distance) > sphere.radius)
		return false;
	
  	// Convert to a 2D problem.
   	
	CSphere circle;
 
	circle.position = sphere.position - slide_plane.normal * sphere_distance;
	circle.radius = sqrt(sphere.radius*sphere.radius - sphere_distance*sphere_distance);
   	
   	if (circle_collision(circle, delta, edge_v1, edge_v2, slide_plane, impact, t))
   		return true;

	// Test for collision against corner points.	

	bool t_collision1;
	bool t_collision2;
	CVector t_impact;
	float t_t;
	
	t_collision1 = point_collision(sphere, delta, edge_v1, t_impact, t_t);
	t_collision2 = point_collision(sphere, delta, edge_v2, impact, t);
	
	if (t_collision1 && (!t_collision2 || t_t < t)) {
		impact = t_impact;
		t = t_t;
	}

	return t_collision1 || t_collision2;	

}

static bool point_in_poly(CPoly* poly, CVector point) {

	CEdge* edge = poly->edge_head;
	
	while (edge) {
	
		const float distance = edge->plane * point;
		
		if (distance > +EPS)
			return false;
			
		edge = edge->next;
		
	}
	
	return true;

}

bool sp_collision(CSphere sphere, CVector delta, CPoly* poly, CVector& impact, float& t) {

	t = 2;
	
	// Test against inner surface.
	
	const float distance = poly->plane * sphere.position;
	CVector point1;
	if (distance >= 0)
		point1 = sphere.position - poly->plane.normal * sphere.radius;
	else
 		point1 = sphere.position + poly->plane.normal * sphere.radius;
	CVector point2 = point1 + delta;
	const float distance1 = poly->plane * point1;
	const float distance2 = poly->plane * point2;	
	if (distance1 < -EPS && distance2 < -EPS) {
	} else if (distance1 > +EPS && distance2 > +EPS) {
	} else {
		float t_t;
		if (distance1 == distance2)
			t_t = 0;
		else if (distance1 == 0)
			t_t = 0;
		else if (distance2 == 0)
  			t_t = 1;
		else
  			t_t = distance1 / (distance1 - distance2);
		CVector t_impact = point1 + delta * t_t;
  		if (point_in_poly(poly, t_impact)) {
  			impact = t_impact;
  			t = t_t;
		}       		
	}
	
	// Test against edges.
	
	CVector t_impact;
	float t_t;
	
	CEdge* edge = poly->edge_head;
	while (edge) {
		CEdge* edge2 = edge->next ? edge->next : poly->edge_head;
		if (collision(sphere, delta, edge->p, edge2->p, t_impact, t_t)) {
			if (t_t < t) {
				impact = t_impact;
				t = t_t;
			}
		}
		edge = edge->next;
	}
	
	if (t < 2)
		return true;
	
   	return false;		

}
