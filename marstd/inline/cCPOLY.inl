//////////////////////////////////////////////////////////////////////
// CComactVertex implementation.
//////////////////////////////////////////////////////////////////////

inline CCompactVertex::CCompactVertex() : CVertex() {

}

inline CCompactVertex::~CCompactVertex() {

}

//////////////////////////////////////////////////////////////////////
// CCompactEdge implementation.
//////////////////////////////////////////////////////////////////////

inline CCompactEdge::CCompactEdge() {

	zero();

}

inline CCompactEdge::~CCompactEdge() {

}

inline void CCompactEdge::zero() {

	vertex1 = -1;
	vertex2 = -1;

	poly1 = 0;
	poly2 = 0;

	mesh = 0;

}

//////////////////////////////////////////////////////////////////////
// CCompactPoly implementation.
//////////////////////////////////////////////////////////////////////

inline CCompactPoly::CCompactPoly() {

	zero();
	
}

inline CCompactPoly::~CCompactPoly() {

	clear();
	
}

inline void CCompactPoly::zero() {

	vertex = 0;
	vertex_count = 0;
	
	edge = 0;
	edge_count = 0;

}

inline void CCompactPoly::clear() {

	if (vertex)
		delete[] vertex;
 
	if (edge)
 		delete[] edge;

	zero();

}

inline void CCompactPoly::add(int vertex) {

	int* tmp = new int[vertex_count+1];
	if (this->vertex) {
		memcpy(tmp, this->vertex, sizeof(int)*vertex_count);
		delete[] this->vertex;
	}		
	this->vertex = tmp;
	this->vertex[vertex_count] = vertex;
	vertex_count++;

}

inline void CCompactPoly::add_edge(int edge) {

	int* tmp = new int[edge_count+1];
	if (this->edge) {
		memcpy(tmp, this->edge, sizeof(int)*edge_count);
		delete[] this->edge;
	}		
	this->edge = tmp;
	this->edge[edge_count] = edge;
	edge_count++;

}

//////////////////////////////////////////////////////////////////////
// CCompactMesh implementation.
//////////////////////////////////////////////////////////////////////

inline CCompactMesh::CCompactMesh() {

	zero();
	
}

inline CCompactMesh::~CCompactMesh() {

	clear();
	
}

inline void CCompactMesh::zero() {

	vertex = 0;
	vertex_count = 0;
	
	edge = 0;
	edge_count = 0;
	
	poly_count = 0;
	
}

inline void CCompactMesh::clear() {

	if (vertex)
		delete[] vertex;
		
	if (edge)
 		delete[] edge;
   
	while (poly.size() > 0) {
		delete poly[poly.size()-1];
		poly.pop_back();
	}
	
	zero();
	
}

inline int CCompactMesh::add(CCompactVertex vertex) {

	CCompactVertex* tmp = new CCompactVertex[vertex_count+1];
	if (this->vertex) {
		memcpy(tmp, this->vertex, sizeof(CCompactVertex)*vertex_count);
		delete[] this->vertex;
	}
	this->vertex = tmp;
	this->vertex[vertex_count] = vertex;
	vertex_count++;

	return vertex_count - 1;
 	
}

inline int CCompactMesh::add(CCompactEdge edge) {

	CCompactEdge* tmp = new CCompactEdge[edge_count+1];
	if (this->edge) {
		memcpy(tmp, this->edge, sizeof(CCompactEdge)*edge_count);
		delete[] this->edge;
	}
	this->edge = tmp;
	this->edge[edge_count] = edge;
	edge_count++;

	return edge_count - 1;
	
}

inline void CCompactMesh::add(CCompactPoly* poly) {

	poly->mesh = this;

	this->poly.push_back(poly);
	poly_count++;
	
}

inline int CCompactMesh::find_edge(int vertex1, int vertex2) {

	for (int i=0; i<edge_count; i++)
		if (edge[i].vertex1 == vertex1 && edge[i].vertex2 == vertex2)
			return i;
		else if (edge[i].vertex1 == vertex2 && edge[i].vertex2 == vertex1)
  			return i;			

	return -1;

}

inline void CCompactMesh::finalize() {

	// Calculate polygon planes.
	
	for (int i=0; i<poly_count; i++) {
	
		CCompactPoly* poly = this->poly[i];
		
		if (poly->vertex_count < 3)
			continue;
			
		CCompactVertex& vertex1 = vertex[poly->vertex[0]];
		CCompactVertex& vertex2 = vertex[poly->vertex[1]];
		CCompactVertex& vertex3 = vertex[poly->vertex[2]];
		
		poly->plane.setup(vertex1.p, vertex2.p, vertex3.p);
		poly->plane.normalize();
	
	}
	
	// Setup edges.
	
	for (int i=0; i<poly_count; i++) {
	
		CCompactPoly* poly = this->poly[i];
	
		for (int j=0; j<poly->vertex_count; j++) {
		
			int vertex1 = poly->vertex[j];
			int vertex2 = poly->vertex[(j+1)%poly->vertex_count];
		
			int edge = find_edge(vertex1, vertex2);
			
			if (edge < 0) {
				CCompactEdge tmp;
				tmp.vertex1 = vertex1;
				tmp.vertex2 = vertex2;
				tmp.poly1 = 0;
				tmp.poly2 = 0;
				tmp.mesh = this;
				edge = add(tmp);
			}
			
			poly->add_edge(edge);
			
			if (!this->edge[edge].poly1)
				this->edge[edge].poly1 = poly;
			else if (!this->edge[edge].poly2)
				this->edge[edge].poly2 = poly;				
				
		}
		
	}

	// Initialize edge planes.

	for (int i=0; i<edge_count; i++) {

		CCompactEdge& edge = this->edge[i];

		CVector delta = vertex[edge.vertex2].p - vertex[edge.vertex1].p;

		if (edge.poly1) {
			edge.plane1.normal = delta % edge.poly1->plane.normal;
			edge.plane1.distance = edge.plane1.normal * vertex[edge.vertex1].p;
		}

		if (edge.poly2) {
			edge.plane2.normal = edge.poly2->plane.normal % delta;
			edge.plane2.distance = edge.plane2.normal * vertex[edge.vertex1].p;
		}

	}

	// Initialize average vertex normals.

	for (int i=0; i<vertex_count; i++)
		vertex[i].normal.set(0.0, 0.0, 0.0);

	for (int i=0; i<vertex_count; i++) {

		int count = 0;

		for (int j=0; j<poly_count; j++) {
			bool has_vertex = false;
			CCompactPoly* poly = this->poly[j];
			for (int k=0; k<poly->vertex_count && !has_vertex; k++)
				if (poly->vertex[k] == i)
					has_vertex = true;
			if (has_vertex) {
				vertex[i].normal += poly->plane.normal;
				count++;
			}
		}

		// Check for unused vertices.

		ASSERT(count);

		vertex[i].normal /= count;

	}

}
