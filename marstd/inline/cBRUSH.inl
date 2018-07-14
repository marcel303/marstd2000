//////////////////////////////////////////////////////////////////////
// CBrush implementation.
//////////////////////////////////////////////////////////////////////

inline CBrush::CBrush() {

	plane_head = 0;
	plane_tail = 0;
	plane_count = 0;
	
	data = 0;
	
	prev = 0;
	next = 0;
	
}

inline CBrush::~CBrush() {

	clear();
	
	unlink();
	
}

inline CPlane* CBrush::add(CPlane* plane) {

	return add_tail(plane);
	
}

inline CPlane* CBrush::add_head(CPlane* plane) {

	DLLIST_LINK_HEAD(plane_head, plane_tail, plane, plane_count);
	
	return plane;
	
}

inline CPlane* CBrush::add_tail(CPlane* plane) {

	DLLIST_LINK_TAIL(plane_head, plane_tail, plane, plane_count);
	
	return plane;
	
}

inline void CBrush::remove(CPlane* plane) {

	DLLIST_REMOVE(plane);
	
}

inline CPlane* CBrush::unlink(CPlane* plane) {

	DLLIST_UNLINK(plane_head, plane_tail, plane, plane_count);
	
	return plane;
	
}

inline void CBrush::clear() {

	DLLIST_CLEAR(plane_head);
	
}

inline CPoly* CBrush::initial(CPlane* plane) {

	float max = -1.0;
	int x = -1;
	
	for (int i=0; i<3; i++) {
	
		float v = fabs(plane->normal[i]);
		
		if (v > max) {
			x = i;
			max = v;
		}
		
	}
	
    CVector vup;
        
    if (x < 2)
        vup[2] = 1.0;
	else
        vup[0] = 1.0;
               	
	// Origin of our poly, a point on the plane.
	
    CVector org = plane->normal * plane->distance;
        
	// Get translation vectors.
	
	float v = vup * plane->normal;
	vup = vup.MA(plane->normal, -v);
    vup.normalize();
        
    CVector vright = vup % plane->normal;
        
	const float huge = 65536.0;
	
    vup *= huge;
    vright *= huge;
        
    CPoly* tmp = new CPoly;
        
    // Add four corner points.
	
    CEdge* edge[4];
       	
	for (int i=0; i<4; i++)
        edge[i] = tmp->add(new CEdge);
	        
    edge[0]->p = org - vright + vup;
    edge[1]->p = org + vright + vup;
    edge[2]->p = org + vright - vup;
	edge[3]->p = org - vright - vup;
	
    tmp->init();
        
	return tmp;
	
}

inline CPoly* CBrush::poly(CPlane* plane) {

	CPoly* poly = initial(plane);
	CPlane* plane2 = plane_head;
	
    while (plane2 && poly) {
        
        if (plane2 != plane) {
        	
            CPoly* poly2 = poly->clip(plane2);
            delete poly;
               	        
            poly = poly2;
                       	
        }
                
        plane2 = plane2->next;
        	
    }
        
    return poly;
        
}

inline CMesh* CBrush::mesh() {

	CMesh* mesh2 = new CMesh;
    CPlane* plane = plane_head;
        
    while (plane) {
        
        CPoly* poly2 = poly(plane);
       		
        if (poly2)
            mesh2->add(poly2);
       	        	
        plane = plane->next;
        	
    }
        
    return mesh2;
        
}

inline CEdge* CBrush::points() {

	CMesh* mesh2 = mesh();
	
        CEdge* point = 0;
       	CPoly* poly = mesh2->poly_head;
       	
        while (poly) {
        
       		CEdge* edge = poly->edge_head;
       		
       		while (edge) {
       		
                	CEdge* edge2 = new CEdge;
       	                *edge2 = *edge;
               	        edge2->next = point;
               	        
                        if (point)
       	                	point->prev = edge2;
       	                	
			point = edge2;
			
                	edge = edge->next;
                	
       	        }
       	        
       	        poly = poly->next;
       	        
        }
        
       	delete mesh2;
       	
        return point;
        
}

inline CBrush* CBrush::unlink() {

	DLLIST_UNLINK_SELF();
	
}
