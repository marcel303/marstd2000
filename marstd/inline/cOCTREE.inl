//////////////////////////////////////////////////////////////////////
// COctree implementation.
//////////////////////////////////////////////////////////////////////

COctree::COctree() {

	zero();

}

COctree::~COctree() {

	clear();

}

void COctree::zero() {

	zero_children();

}

void COctree::zero_children() {

	for (int i=0; i<8; i++)
		child[i] = 0;	

	leaf = true;				

}

void COctree::clear() {

	clear_children();
}

void COctree::clear_children() {

	for (int i=0; i<8; i++)
		if (child[i])
			delete child[i];

	zero_children();					

}
	
float COctree::calculate_cube_size() {

	CVector mins, maxs;

	for (unsigned int i=0; i<polygon.size(); i++) {

		CPoly* poly = polygon[i];
		CEdge* edge = poly->edge_head;

		while (edge) {

			for (int i=0; i<3; i++)
				if (edge->p[i] < mins[i])
					mins[i] = edge->p[i];
				else if (edge->p[i] > maxs[i])
					maxs[i] = edge->p[i]; 						

			edge = edge->next;

		}

	}

	float max;

	for (int i=0; i<3; i++) {

		if (fabs(mins[i]) > max)
			max = fabs(mins[i]);
		if (fabs(maxs[i]) > max)
			max = fabs(maxs[i]);				

	}				

	return max;

}

void COctree::create(float min_cube_size, int min_polycount) {

	int levels = calculate_cube_size / min_cube_size;

	calculate(levels, min_polycount);

}

void COctree::create(int levels, int min_polycount) {

	clear_children();

	if (levels <= 0 || polygon.size() <= min_polycount)
		return;

	leaf = false;

	for (int i=0; i<2; i++)
	for (int j=0; j<2; j++)
	for (int k=0; k<2; k++) {

		const int x = i?1:-1;
		const int y = j?1:-1;
		const int z = k?1:-1;

		const int index = i + j*2 + k*4;

		COctree* node = child[index] = new COctree;

		node->size = size * 0.5;

		CVector tmp = mid;			
		const float size4 = size * 0.25;
		tmp[0] += x * size4;
		tmp[1] += y * size4;
		tmp[2] += z * size4;

		node->mid = tmp;

	}

	// Filter polygons to children.

	for (unsigned i=0; i<polygon.size(); i++)
		filter(polygon[i]);

	// Split children.			

	for (int i=0; i<8; i++)
  		child[index]->create(levels-1, min_polycount);			

}

void COctree::filter(CPoly* poly) {

	MASSERT(poly, "");	

	// If this is a leaf node, add polygon to list and stop recursion.

	if (leaf) {

		polygon.push_back(poly);
		return;

	}

	// Pass down child nodes.

	int r[3];

	// Calculate bounding box.

	CVector mins, maxs;

	CEdge* edge = poly->edge_head;
	mins = maxs = edge->p;

	edge = edge->next;

	while (edge) {

		for (int i=0; i<3; i++)
			if (edge->p[i] < mins[i])
				mins[i] = edge->p[i];
			else if (edge->p[i] > maxs[i])
				maxs[i] = edge->p[i];					

		edge = edge->next;

	}

	// Calculate relationships. Before, behind or spanning.

	for (int i=0; i<3; i++)
		if (maxs[i] <= mid[i])
			r[i] = -1;
		else if (mins[i] >= mid[i])
			r[i] = +1;
		else
   			r[i] = 0;

	// Pass polygon down to child nodes.

  	for (int i=0; i<2; i++)
  	for (int j=0; j<2; j++)
	for (int k=0; k<2; k++) {

		const int x = i?1:-1;
		const int y = j?1:-1;
		const int z = k?1:-1;

		if ((r[0] == x || r[0] == 0) &&
		    (r[1] == y || r[1] == 0) &&
		    (r[2] == y || r[2] == 0))
			child[i + j*2 + k*4]->filter(poly);

	}   		     				

}  	
