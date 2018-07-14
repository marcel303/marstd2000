#ifndef __cCPOLY_h__
#define __cCPOLY_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cCPOLY.h: CCompact* implementation. */
//////////////////////////////////////////////////////////////////////

/// Geometry: shared vertex.
/**
 * Shared vertices have in addition to a normal vertex an average normal.
 * NOTE: This structure may not have any dynamically allocated data!
 */
class CCompactVertex : public CVertex {

 public:
 
	CCompactVertex();
	~CCompactVertex();
 
 public:
 
	CVector normal;

};

/// Geometry: shared egde.
/**
 * Shared edges are usually shared between two polygons in closed polyheadra.
 * Use vertex1 and vertex2 to index into the shared vertex array.
 * After finalization poly1 and poly2 will point to the two polygons that use this edge.
 * NOTE: This structure may not have any dynamically allocated data!
 */
class CCompactEdge {

 public:
 
	CCompactEdge();
	~CCompactEdge();
	void zero();
	
 public:
 
	int vertex1;			///< First vertex.
	int vertex2;			///< Second vertex.

	// Valid after finalization:
  	
 	CPlane plane1;			///< Inwards pointing plane.
	CPlane plane2;
 	
 	class CCompactPoly* poly1;	///< First polygon this edge is part of.
 	class CCompactPoly* poly2;	///< Second polygon this edge is part of (if any).
 	
 	class CCompactMesh* mesh;	///< Parent mesh.

};

/// Geometry: polygon with shared vertices and edges.
/**
 * The compact polygon class stored only indices into a vertex array. The actual vertex data is owned by the mesh.
 * The shared edges are valid after finalization. Edges are usually shared with onather polygon.
 */
class CCompactPoly {

 public:
 
	CCompactPoly();
	~CCompactPoly();
	void zero();
 
 public:
 
 	int* vertex;			///< Indicis into vertex array of mesh.
 	int vertex_count;		///< Number of vertices.
 	
 	int flags;
 	
	// Valid after finalization:
	
	int* edge;			///< Indicis into edge array of mesh. Auto-generated on finalization.
	int edge_count;			///< Number of edges.

	CPlane plane;
	
	class CCompactMesh* mesh;	///< Parent mesh.
  
 public:

	void clear();			///< Frees all memory. 
	void add(int vertex);		///< Adds a vertex index.
	void add_edge(int edge);	///< Adds an edge index. This is done automatically on finalization.

};

/// Geometry: mesh with support for shared vertices and edges.
/**
 * Shared vertices, shared edges mesh.
 * Most file formats have a shared vertices format. This means the mesh structure has an array of vertex data and
 * the faces have indexes into this array. This class also adds shared edges. Every edge in a closed polyheadron
 * is shared between two polygons.
 * Note that polygons are not to be removed from the mesh once they're added.
\code
// Example:

void create_triangles(CCompactMesh& mesh) {

	// Add 4 shared vertices.
	
	CCompactVertex vertex;
	
	vertex.p[0] = -1.0;
	vertex.p[1] = 0.0;
	vertex.p[2] = 0.0;
	
	mesh.add(vertex);
	
	vertex.p[0] = +1.0;
	vertex.p[1] = 0.0;
	vertex.p[2] = 0.0;
	
	mesh.add(vertex);
	
	vertex.p[0] = 0.0;
	vertex.p[1] = -1.0;
	vertex.p[2] = 0.0;
	
	mesh.add(vertex);
	
	vertex.p[0] = 0.0;
	vertex.p[1] = +1.0;
	vertex.p[2] = 0.0;
	
	// Add 2 triangles.
	
	CCompactPoly* poly;
 
  	poly = new CCompactPoly;
	
	poly->add(0);			// Index to vertex 0.
	poly->add(1);			// Index to vertex 1.
	poly->add(2);			// Index to vertex 2.
	
	mesh.add(poly);
	
  	poly = new CCompactPoly;
	
	poly->add(0);			// ...
	poly->add(1);			// ...
	poly->add(3);			// Index to vertex 3.
	
	mesh.add(poly);	
	
	// Finalize mesh. This adds edges, initializes polygons, average vertex normals, ...
	
	mesh.finalize();

}

\endcode 
 */
class CCompactMesh {

 public:

	CCompactMesh();
	~CCompactMesh();
	void zero();

 public:

	CCompactVertex* vertex;			///< Shared vertices. Use vertex index instead of pointers!
 	int vertex_count; 
 	
	CCompactEdge* edge;			///< Shared edges. These are auto-generated from the polygon information on finalization.
	int edge_count; 	

	std::vector<CCompactPoly*> poly;	///< Polygons.
 	int poly_count;
 	
 public:

	void clear();				///< Frees all memory.
	int add(CCompactVertex vertex);		///< Adds a vertex. Returns the vertex index.
	int add(CCompactEdge edge);		///< Adds an edge. Returns the edge index.
	void add(CCompactPoly* poly);		///< Adds a polygon.
	int find_edge(int vertex1, int vertex2);///< Finds an edge by vertices.
	void remove_unsused_vertices();		///< Removes vertices not referenced by the polygons. Fixes all indices.
	void merge_vertices(float bias);	///< Merges identical vertices. Bias is used to overcome FP inaccuracies and should be relatively small.
	void collapse_edge();			///< Collapses _one_ edge. Use collapse_edge repeatedly to create an object with less vertices (LOD). This operation assumes all polygons are triangles!
	void finalize();			///< Finalizaes the mesh. Some member variables are only valid after finalization. Finalizae the mesh after all vertex and polygon data has been loaded.

};

class CCompactBsp {

 public:
 
	CCompactBsp();
	~CCompactBsp();
 
 public:
 
	CCompactMesh* mesh;			///< Mesh with shared vertex and edge data.
	std::vector<CCompactPoly*> poly;	///< List of polygons;
	int poly_count;
	
 public:
 
	void split(CPlane plane);  	

};

#include "inline/cCPOLY.inl"

#endif
