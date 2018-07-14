//////////////////////////////////////////////////////////////////////
// CBsp implementation.
//////////////////////////////////////////////////////////////////////

inline CBsp::CBsp() : CMesh() {

	eps = BSP_DEFAULT_EPS;
	
	b_f = 0;
	b_b = 0;	
	
	initialized = false;
	
	polydata = 0;
	
}

inline CBsp::~CBsp() {

	if (b_f)
		delete b_f;
	if (b_b)
		delete b_b;	   
		
}

inline bool CBsp::coplanar() {

	if (!poly_head || !poly_head->next)
        	return true;
                
	p = poly_head->plane;

        CPoly* p = poly_head->next;

        while (p) {

        	if (!on_f(p, true) || !on_b(p, true))
                	return false;

		p = p->next;
        
        }

        return true;

}


inline void CBsp::move(CPoly* poly, CBsp* bsp) {

	bsp->add(unlink(poly)); 	
	
}

inline int CBsp::on_f(CPoly* poly, int dup) {

	MASSERT(poly, "");
	
	const float eps2 = dup ? eps*4.0 : eps;
	
	#if defined(BSP_USE_SPHERES)
	const CSphere* sphere = &((bsp_polydata_t* )poly->data)->sphere;
	if (p * sphere->position - sphere->radius >= -eps2)
		return 1;
	if (p * sphere->position + sphere->radius <= +eps2)
		return 0;
	#endif   		
	
	CEdge* l = poly->edge_head;
	
	while (l) {
	
		if (p * l->p <= -eps2)
			return 0;
			
		l = l->next;
		
	}
	
	return 1;
	
}

inline int CBsp::on_b(CPoly* poly, int dup) {

	MASSERT(poly, "");	
	
	const float eps2 = dup ? eps * 4.0 : eps;
	
	#if defined(BSP_USE_SPHERES)		
	const CSphere* sphere = &((bsp_polydata_t* )poly->data)->sphere;
  	// FIXME: We do this test twice. Should make a unified function to classify polygons.
	if (p * sphere->position + sphere->radius <= +eps2)
		return 1;
//	if (p * sphere->position - sphere->radius >= -eps2)
//		return false;			
	#endif			
	
	CEdge* l = poly->edge_head;
	
	while (l) {
	
		if (p * l->p >= +eps2)
			return 0;
			
		l = l->next;
		
	}
	
	return 1;
	
}

inline void CBsp::split_info(CPlane& p, bsp_split_info_t& i, bool flip) {

	if (flip)
		i.p = -p;
	else
       		i.p = p;
       		
	i.fc = 0;
       	i.bc = 0;
        i.dc = 0;
       	i.sc = 0;
       	
        this->p = i.p;
        
        CPoly* poly = poly_head;
        
        while (poly) {
        
        	if (on_f(poly))
       	        	i.fc++;
		else if (on_b(poly))
                	i.bc++;
		else {
       	        	i.fc++;
               	        i.bc++;
                        i.sc++;
		}
		
        	poly = poly->next;
        	
        }
        
        i.dc = i.fc-i.bc;
       	if (i.dc < 0)
        	i.dc = -i.dc;
        	
}

inline bool CBsp::test1(bsp_split_info_t* i1, bsp_split_info_t* i2) {

	return (i1->sc < i2->sc || (i1->sc == i2->sc && i1->dc < i2->dc));
	
}	

inline bool CBsp::test2(bsp_split_info_t* i1, bsp_split_info_t* i2) {

	return (i1->dc < i2->dc || (i1->dc == i2->dc && i1->sc < i2->sc));
	
}	

inline bool CBsp::test(bsp_split_info_t* i1, bsp_split_info_t* i2) {

	// Try to reduce splits when the number of polygons is low.
	
	if (poly_count < 10)
		return test1(i1, i2);
	else
  		return test2(i1, i2);			
  		
}         	   

inline CPlane CBsp::split_plane() {

	bsp_split_info_t best_i, i;
  	
	best_i.fc = 0;
	
	CPoly* p = poly_head;
	
	while (p) {
	
		split_info(p->plane, i, false);
		if (i.fc > 0 && i.bc > 0 && (test(&i, &best_i) || best_i.fc == 0))
			best_i = i;
			
		split_info(p->plane, i, true);
		if (i.fc > 0 && i.bc > 0 && (test(&i, &best_i) || best_i.fc == 0))
			best_i = i;
			
		p = p->next;
		
	}
	
	if (best_i.fc == 0)
		no_splitting_plane = true;
	else
		no_splitting_plane = false;
		
	#if defined(MARSTD_PARANOID) && defined(BSP_ALTERNATE_CANCEL)
	if (no_splitting_plane) {
		MARX::message("Error: %s: found no splitting plane while 'alternate cancel' is defined", __FUNCTION__);
		exit(-1);
	}			
  	#endif			
  	
	return best_i.p;
	
}

namespace CBspExtern {

static inline void COPY(CEdge* edge, CPoly* poly) {

	CEdge* tmp = poly->add(new CEdge);

	*tmp = *edge;

}

static inline void INTERP(CEdge* l1, CEdge* l2, float t, CPoly* p_f, CPoly* p_b) {

	#define DO_INTERP(_n) l_f->_n = l_b->_n = l1->_n + (l2->_n-l1->_n) * t

        CEdge* l_f = p_f->add(new CEdge);
        CEdge* l_b = p_b->add(new CEdge);

        DO_INTERP(p[0]);
        DO_INTERP(p[1]);
        DO_INTERP(p[2]);
        DO_INTERP(c[0]);
        DO_INTERP(c[1]);
        DO_INTERP(c[2]);
        DO_INTERP(c[3]);        
        for (int i=0; i<VERTEX_TEX_COUNT; i++)
        	for (int j=0; j<VERTEX_TEX_COMPONENTS; j++)
		        DO_INTERP(t[i][j]);

}

}

inline void CBsp::split(CPoly* a_p) {

	CPoly* p_f = b_f->add(new CPoly);
	CPoly* p_b = b_b->add(new CPoly);
	
	p_f->data = a_p->data;
	p_b->data = a_p->data;
        
	CEdge* l1 = a_p->edge_head;

        while (l1) {

        	CEdge* l2 = l1->next ? l1->next : a_p->edge_head;

                float d1 = p * l1->p;
                float d2 = p * l2->p;

		#if 0
                if (d1 == 0.0) {
			CBspExtern::COPY(l1, p_f);
                        CBspExtern::COPY(l1, p_b);
                } else if (d1 >= 0.0 && d2 >= 0.0) {
                        CBspExtern::COPY(l1, p_f);
                } else if (d1 <= 0.0 && d2 <= 0.0) {
                	CBspExtern::COPY(l1, p_b);
                } else if (d1 >= 0.0 && d2 <= 0.0) {
                	CBspExtern::COPY(l1, p_f);
                        CBspExtern::INTERP(l1, l2, -d1/(d2-d1), p_f, p_b);
                } else if (d1 <= 0.0 && d2 >= 0.0) {
                	CBspExtern::COPY(l1, p_b);
                        CBspExtern::INTERP(l1, l2, -d1/(d2-d1), p_f, p_b);
                }
                #else
                const float eps2 = eps*0.5;
                if (d1 >= -eps2 && d1 <= +eps2) {
			CBspExtern::COPY(l1, p_f);
                        CBspExtern::COPY(l1, p_b);
                } else if (d1 >= -eps2 && d2 >= -eps2) {
                        CBspExtern::COPY(l1, p_f);
                } else if (d1 <= +eps2 && d2 <= +eps2) {
                	CBspExtern::COPY(l1, p_b);
                } else if (d1 >= 0.0 && d2 <= 0.0) {
                	CBspExtern::COPY(l1, p_f);
                        CBspExtern::INTERP(l1, l2, -d1/(d2-d1), p_f, p_b);
                } else if (d1 <= 0.0 && d2 >= 0.0) {
                	CBspExtern::COPY(l1, p_b);
                        CBspExtern::INTERP(l1, l2, -d1/(d2-d1), p_f, p_b);
		}
		#endif                               
                
        	l1 = l1->next;
        
        }

	// FIXME: Shouldn't really do this. copying normal is OK.

        p_f->init();
        p_b->init();

	#if 0
	CPoly* p[2] = { p_f, p_b };
	
 	for (int i=0; i<2; i++)        
	        if (p[i]->plane.normal * a_p->plane.normal < 0.5) {
	        	// New polygon has mirrored plane!
	        	CPoly* tmp = new CPoly;
	        	while (p[i]->edge_head)
		        	tmp->add(p[i]->unlink(p[i]->edge_head));
			while (tmp->edge_head)
				p[i]->add_head(tmp->unlink(tmp->edge_head));
			delete tmp;
   			p[i]->init(); 	
        	}
	#endif        	

}

inline void CBsp::split() {

	if (b_f)
        	delete b_f;
	if (b_b)
        	delete b_b;

	b_f = 0;
        b_b = 0;
        
        if (!initialized) {
        	init_bounding_spheres();
        	initialized = true;
        }
        
	init_bounding_box();        

	#if 0
	if (poly_count < 10) {
		restore_polydata();
		return;
	}
	#endif
	
	#if defined(BSP_ALTERNATE_CANCEL)
	if (coplanar()) {
		restore_polydata();
        	return;
	}        	
	#endif

	p = split_plane();

	#if !defined(BSP_ALTERNATE_CANCEL)
        if (no_splitting_plane) {
        	restore_polydata();
        	return;
	}        	
	#endif
                
	b_f = new CBsp;
        b_b = new CBsp;
        
        // Children don't need to be initialized.
        
        b_f->initialized = true;
        b_b->initialized = true;
	
	{
	
        CPoly* p = poly_head;

        while (p) {

        	CPoly* next = p->next;
                
        	if (on_f(p))
                	move(p, b_f);
		else if (on_b(p))
                	move(p, b_b);
		else {
                	split(p);
                        remove(p);
		}
                
        	p = next;
        
        }
        
        }

        b_f->split();
        b_b->split();

	free_bounding_spheres();        
        initialized = false;

}

inline CSphere CBsp::bounding_sphere(CPoly* poly) {

	MASSERT(poly, "");	
	MASSERT(poly->edge_head, "");
	
	CVector mins, maxs;
	
	mins = poly->edge_head->p;
	maxs = poly->edge_head->p;
	
	CEdge* edge = poly->edge_head->next;
	
	while (edge) {
	
		for (int i=0; i<3; i++) {
			if (edge->p[i] < mins[i])
				mins[i] = edge->p[i];
			if (edge->p[i] > maxs[i])
    				maxs[i] = edge->p[i];					
		}
		
		edge = edge->next;
		
	}
	
	CSphere sphere;          			
	sphere.position = (mins + maxs) * 0.5;
	
	edge = poly->edge_head;
	
	while (edge) {
	
		CVector delta = edge->p - sphere.position;
		
		float size2 = delta.size2();
		if (size2 > sphere.radius)
			sphere.radius = size2;
			
		edge = edge->next;
		
	}
	
	sphere.radius = sqrt(sphere.radius) + eps*4.0;
	
	return sphere;
	
}

inline void CBsp::init_bounding_spheres() {

	if (poly_count <= 0)
		return;
		
	polydata = new bsp_polydata_t[poly_count];
	
	CPoly* poly = poly_head;
	
	int index = 0;
	
	while (poly) {
	
		polydata[index].data = poly->data;
		polydata[index].sphere = bounding_sphere(poly);
		
		poly->data = &polydata[index];
		poly = poly->next;
		
		index++;
		
	}
	
}

inline void CBsp::free_bounding_spheres() {

	if (polydata)
		delete[] polydata;
		
}

inline void CBsp::restore_polydata() {

	CPoly* poly = poly_head;
	
	while (poly) {
	
		bsp_polydata_t* data = (bsp_polydata_t* )poly->data;
		poly->data = data->data;
		
		poly = poly->next;
		
	}
	
}

inline void CBsp::init_bounding_box() {

	if (!poly_head) {
//		allegro_message("Error: empty BSP node!");
		exit(-2);
	}
 
   	// Calculate bounding box.		
	
	mins = poly_head->edge_head->p;
	maxs = poly_head->edge_head->p;
	
	CPoly* poly = poly_head;
	
	while (poly) {
	
		CEdge* line = poly->edge_head;
		
		while (line) {
		
			for (int i=0; i<3; i++) {
				if (line->p[i] < mins[i])
					mins[i] = line->p[i];
				if (line->p[i] > maxs[i])
					maxs[i] = line->p[i];					
			}
   				
			line = line->next;
		
		}
		
		poly = poly->next;
	
	}
	
	// Calculate bounding sphere.
	
	CVector mid;
	int count = 0;
	poly = poly_head;
	
	while (poly) {
	
		CEdge* edge = poly->edge_head;
		
		while (edge) {
		
			mid += edge->p;
			count++;
			
			edge = edge->next;
		
		}
		
		poly = poly->next;
	
	}
	
	sphere.position = mid / count;
	
	poly = poly_head;
 
  	while (poly) {
  	
  		CEdge* edge = poly->edge_head;
  		
  		while (edge) {
  		
  			CVector delta = sphere.position - edge->p;
  			
  			if (delta.size2() > sphere.radius)
  				sphere.radius = delta.size2();
  				
  			edge = edge->next;
  			
  		}
  		
  		poly = poly->next;
  	
	}
 
  	sphere.radius = sqrt(sphere.radius);	
	
	// Calculate 'volume'.
	
	volume = 1.0;
	for (int i=0; i<3; i++)
		volume *= maxs[i] - mins[i];

}

