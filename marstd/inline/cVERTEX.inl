//////////////////////////////////////////////////////////////////////
// CVertex implementation.
//////////////////////////////////////////////////////////////////////

inline CVertex::CVertex() {

}

inline CVertex::~CVertex() {

}

inline void CVertex::set_colour(float r, float g, float b, float a) {

	c[0] = r;
	c[1] = g;
	c[2] = b;
	c[3] = a;
	
}

inline void CVertex::set_texture(float u, float v, float w) {

	t[0][0] = u;
	t[0][1] = v;
	t[0][2] = w;
	
}

inline void CVertex::set_texture(int texture, float u, float v, float w) {

	t[texture][0] = u;
	t[texture][1] = v;
	t[texture][2] = w;
	
}

inline CVertex& CVertex::operator=(CVertex vertex) {

	p = vertex.p;
	
	c[0] = vertex.c[0];
	c[1] = vertex.c[1];
	c[2] = vertex.c[2];
	c[3] = vertex.c[3];
	
	for (int i=0; i<VERTEX_TEX_COUNT; i++)
		for (int j=0; j<VERTEX_TEX_COMPONENTS; j++)
			t[i][j] = vertex.t[i][j];
			
	return* this;
	
}   

