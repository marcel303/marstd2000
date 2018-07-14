#ifndef __Cpoly_h__
#define __Cpoly_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cPOLY.h: CPoly and CMesh implementation. */
//////////////////////////////////////////////////////////////////////

#include "cEDGE.h"
#include "cMATRIX.h"
#include "cSPHERE.h"

//---------------------------------------------------------------------------

/// Geometry: Polygon type.
/**
 * The polygon class defines a polygon using a DLL of edges in CW order.
 * Polygons should be convex and defined CW to use all of the features.
 */
class CPoly {

 public:
 
	CPoly();
	~CPoly();
	
 public:
 
	CEdge* edge_head;	///< First edge in DLL.
	CEdge* edge_tail;	///< Last edge in DLL.
 	int edge_count;		///< Number of edges.
	CPlane plane; 		///< Plane. Uses left hand rule.
	void* data;		///< Custom data field. Use it to store a pointer to some custom data structure. Eg to define material properties, texture, ...
	
 public:
 
	/**
	 * Links an edge to the tail.
	 * @see add_tail()
  	 */ 
 	CEdge* add(CEdge* edge);
 	/**
 	 * Links an edge to the head.
 	 * @see add_tail()
 	 */
	CEdge* add_head(CEdge* edge);
	/**
	 * Links an edge to the tail.
	 * @see add_head()
	 */
 	CEdge* add_tail(CEdge* edge);
 	/**
 	 * Unlinks an edge from the DLL. Note that this does not free any memory.
 	 * @see remove()
 	 */
 	CEdge* unlink(CEdge* edge);
 	/**
 	 * Removes an edge. This unlinks the edge and then frees it's memory.
 	 * @unlink()
 	 */
 	void remove(CEdge* edge);
 	/**
 	 * Clear the DLL of edges. This will free all memory.
 	 */
 	void clear();
 	
 public:
  	
  	/**
  	 * Initializes the polygon. This will calculate the plane and edge normals.
  	 * This should be done when the polygon creation has finished. If the number of
  	 * edges < 3 this function will bail.
  	 */
 	void init();
 	/**
 	 * Calculates a bounding sphere. The polygon is entirely contained within the
   	 * bounding sphere.
   	 * @return The bounding sphere.
 	 */
 	CSphere calculate_sphere();
 	CPoly* clip(CPlane* plane);
 	
 public:
 
	CPoly* prev;	///< Reference to previous polygon in DLL.
 	CPoly* next;	///< Reference to next polygon in DLL.
  
 public:
 
 	/**
 	 * Unlinks this polygon from a DLL.
 	 */
	CPoly* unlink();         	

};

//////////////////////////////////////////////////////////////////////

/// Geometry: Mesh type.
/**
 * A mesh is just a collection of polygons. Use it to store many polygons belonging to a particular group,
 * so you can render them using a different matrix eg, or to paint them with the same material.
 */
class CMesh {

 public:
	
	CMesh();
	~CMesh();
	
 public:
 
	CPoly* poly_head;	///< First polygon in DLL.
	CPoly* poly_tail;	///< Last polygon in DLL.
	int poly_count;		///< Number of polygons in DLL.
	void* data;		///< Custom data field.
	
 public:
 
 	/**
 	 * Links polygon to tail.
 	 * @see add_tail()
 	 */
 	CPoly* add(CPoly* poly);
 	/**
 	 * Links polygon to head.
 	 * @see add_tail()
 	 */
	CPoly* add_head(CPoly* poly);
 	/**
 	 * Links polygon to tail.
 	 * @see add_head()
 	 */	
	CPoly* add_tail(CPoly* poly);
 	/**
         * Unlinks poygon from DLL.
         */	
	CPoly* unlink(CPoly* poly);
	/**
	 * Unlinks polygon from DLL and frees memory.
	 */
	void remove(CPoly* poly);
	/**
	 * Clear polygon DLL.
	 */
	void clear();
        /** Transforms all vertices in mesh.
         * @param matrix Matrix that will be used to transform vertices.
         */
	void transform(CMatrix& matrix);
	/**
	 * Moves contents of polygons DLL to another mesh.
	 */
	void move(CMesh& mesh);
	/**
	 * Assign a custom data reference to all polygon.
	 * @param data Pointer to custom data structure.
	 */
	void paint(void* data);

};

#include "inline/cPOLY.inl"

//---------------------------------------------------------------------------

#endif
