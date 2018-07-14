//////////////////////////////////////////////////////////////////////
// CPoly implementation.
//////////////////////////////////////////////////////////////////////

inline CPoly::CPoly() {

	edge_head = 0;
	edge_tail = 0;
	edge_count = 0;

	data = 0;

	prev = 0;
	next = 0;

}

inline CPoly::~CPoly() {

	clear();
	
	unlink();
	
}

inline CEdge* CPoly::add(CEdge* edge) {

	return add_tail(edge);
	
}

inline CEdge* CPoly::add_head(CEdge* edge) {

	MASSERT(edge, "");	
	
	if (edge_head)
		edge->edge2 = edge_head;
	else
		edge->edge2 = 0;			
		
	DLLIST_LINK_HEAD(edge_head, edge_tail, edge, edge_count);
	
	return edge;
	
}

inline CEdge* CPoly::add_tail(CEdge* edge) {

	MASSERT(edge, ""); 	
	
	if (edge_tail)
 		edge_tail->edge2 = edge;
	edge->edge2 = edge_head;
	
	DLLIST_LINK_TAIL(edge_head, edge_tail, edge, edge_count);
	
	return edge;
	
}

inline CEdge* CPoly::unlink(CEdge* edge) {

	MASSERT(edge, ""); 	
	
	if (edge->prev)
		edge->prev->edge2 = edge->edge2;
		
	DLLIST_UNLINK(edge_head, edge_tail, edge, edge_count);
	
	return edge;
	
}

inline void CPoly::remove(CEdge* edge) {

	MASSERT(edge, ""); 	
	
	DLLIST_REMOVE(edge);
	
}

inline void CPoly::clear() {

	DLLIST_CLEAR(edge_head);
	
}
 	
inline void CPoly::init() {

	MASSERT(edge_head && edge_head->next && edge_head->next->next, ""); 	
	
 	if (!edge_head || !edge_head->next || !edge_head->next->next)
 		return;
 		
	plane.setup(edge_head->p, edge_head->next->p, edge_head->next->next->p);
	plane.normalize();
	
	CEdge* edge1;
  	CEdge* edge2;
  	
  	// edge->plane
  	
    	edge1 = edge_head;
	while (edge1) {
		edge2 = edge1->next ? edge1->next : edge_head;
		CVector delta = edge2->p - edge1->p;
		edge1->plane.setup(edge1->p, edge2->p, edge1->p + plane.normal);
		edge1->plane.normalize();
		edge1 = edge1->next;
	}
	
	// edge->edge_plane
	
	edge1 = edge_head;
	while (edge1) {
		edge2 = edge1->next ? edge1->next : edge_head;
		CVector delta = edge2->p - edge1->p;
		float size = delta.size();
		delta /= size;
		edge1->edge_plane.normal = delta;
		edge1->edge_plane.distance = edge1->edge_plane.normal * edge1->p;
		edge1 = edge1->next;
	}
	
	#if defined(MARSTD_PARANOID)
	{
		CVector mid;
		CEdge* edge;
   		edge = edge_head;
		while (edge) {
			mid += edge->p;
			edge = edge->next;
		}
		mid /= edge_count;
		edge = edge_head;
		while (edge) {
			if (edge->plane * mid > 0.0) {
				allegro_message("CPoly::init(): Non convex or non CW polygon.");
				exit(-2);
			}					
			edge = edge->next;
		}
	}
	#endif
	
}

inline CSphere CPoly::calculate_sphere() {

 	CSphere sphere;
	CVector mid;
	int count = 0;
	
	CEdge* edge;
  	edge = edge_head;
  	
	while (edge) {
	
		mid += edge->p;
		count++;
		
		edge = edge->next;
		
	}
	
	sphere.position = mid / count;
	
	edge = edge_head;
	
  	while (edge) {
  	
  		CVector delta = sphere.position - edge->p;
  		
  		if (delta.size2() > sphere.radius)
  			sphere.radius = delta.size2();
  			
  		edge = edge->next;
  		
  	}
  	
	sphere.radius = sqrt(sphere.radius);	
	
	return sphere;
	
}

inline CPoly* CPoly::clip(CPlane* plane) {

    CPoly* tmp = new CPoly;
    
    CEdge* edge1 = edge_head;
    
    while (edge1) {

        CEdge* edge2 = edge1->edge2;

		float d1 = edge1->p * plane->normal - plane->distance;
		float d2 = edge2->p * plane->normal - plane->distance;

        #define COPY(_edge) \
        { \
            CEdge* edge = tmp->add_tail(new CEdge); \
            edge->p = _edge->p; \
        }

        #define DO_INTERP(_v) \
            edge->_v = edge1->_v + (edge2->_v - edge1->_v) * t;

        #define INTERP() \
        { \
            const float t = -d1 / (d2 - d1); \
            CEdge* edge = tmp->add_tail(new CEdge); \
            DO_INTERP(p); \
            for (int i = 0; i < 4; i++) \
                DO_INTERP(c[i]); \
        	for (int i=0; i<VERTEX_TEX_COUNT; i++) \
                for (int j=0; j<VERTEX_TEX_COMPONENTS; j++) \
                    DO_INTERP(t[i][j]); \
        }

        #if 1
        if (d1 > 0.0 && d2 >= 0.0) {
        } else if (d1 > 0.0 && d2 < 0.0) {
            INTERP();
        } else if (d1 <= 0.0 && d2 <= 0.0) {
            COPY(edge1);
        } else if (d1 <= 0.0 && d2 > 0.0) {
            COPY(edge1);
            INTERP();
        }
        #else
        if (d1 < 0.0 && d2 <= 0.0) {
        } else if (d1 < 0.0 && d2 > 0.0) {
            INTERP();
        } else if (d1 >= 0.0 && d2 >= 0.0) {
            COPY(edge1);
        } else if (d1 >= 0.0 && d2 < 0.0) {
            COPY(edge1);
            INTERP();
        }
        #endif

        #undef INTERP
        #undef DO_INTERP
        #undef COPY

        edge1 = edge1->next;

    }

    tmp->init();

    return tmp;

}
 	
inline CPoly* CPoly::unlink() {

	DLLIST_UNLINK_SELF();
	
}   
                     	
//////////////////////////////////////////////////////////////////////
// CMesh implementation.
//////////////////////////////////////////////////////////////////////

inline CMesh::CMesh() {

	poly_head = 0;
	poly_tail = 0;
	poly_count = 0;
	
	data = 0;
	
}   

inline CMesh::~CMesh() {

	clear();
	
}

inline CPoly* CMesh::add(CPoly* poly) {

 	return add_tail(poly);
 	
}

inline CPoly* CMesh::add_head(CPoly* poly) {

	MASSERT(poly, "");	
	
	DLLIST_LINK_HEAD(poly_head, poly_tail, poly, poly_count);
	
	return poly;
	
}

inline CPoly* CMesh::add_tail(CPoly* poly) {

	MASSERT(poly, "");	
	
	DLLIST_LINK_TAIL(poly_head, poly_tail, poly, poly_count);
	
	return poly;
	
} 

inline CPoly* CMesh::unlink(CPoly* poly) {

	MASSERT(poly, "");	
	
	DLLIST_UNLINK(poly_head, poly_tail, poly, poly_count);
	
	return poly;
	
}

inline void CMesh::remove(CPoly* poly) {

	MASSERT(poly, "");	
	
	DLLIST_REMOVE(poly);
	
}

inline void CMesh::clear() {

	DLLIST_CLEAR(poly_head);
	
}

inline void CMesh::transform(CMatrix& matrix) {

       	CPoly* poly = poly_head;
       	
        while (poly) {
        
               	CEdge* edge = poly->edge_head;
               	
                while (edge) {
                
                       	matrix.apply(edge->p, edge->p);
                       	
                       	edge = edge->next;
                       	
                }
                
               	poly = poly->next;
               	
        }
        
}

inline void CMesh::move(CMesh& mesh) {

	while (poly_head)
		mesh.add(unlink(poly_head));
		
}

inline void CMesh::paint(void* data) {

	CPoly* poly = poly_head;
	
	while (poly) {
	
		poly->data = data;
		
		poly = poly->next;
		
	}
	
}

//////////////////////////////////////////////////////////////////////
