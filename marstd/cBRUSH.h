#ifndef __Cbrush_h__
#define __Cbrush_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cBRUSH.h: CBrush implementation. */
//////////////////////////////////////////////////////////////////////

#include "CList.h"
//#include "Cprim.h"	// FIXME: remove this include.
#include "CPlane.h"
#include "CPoly.h"

/// Geometry: Brush class.
/**
 * The brush class is used to define a convex volume enclosed by planes. The brush is defined by planes and can
 * be converted to a mesh. Brushes can be added, subtracted and or-ed together using CSG.
\code
// Example:

void random_shape(CMesh& mesh, float radius) {

	// Probably not the most optimized way to do this ;)
	
	CBrush brush;
	CVector origin;	// Origin = (0.0, 0.0, 0.0).
	
	for (int i=0; i<3; i++) {
	
		CPlane* plane = brush.add(new CPlane);
		
		// Make normal random.
		
		for (int j=0; j<3; j++)
			plane->normal[j] = (rand()&1023)/1023.0-0.5;;
   		plane.normal.normalize();
   		
   		// Offset plane from origin.
   		
		plane->distance	= radius;		
		
		// Make sure every plane points away from origin.
		
		if (plane * origin > 0.0)
			plane = -plane;
	
	}
	
	// Convert brush to mesh, and move polygons to output mesh.
	
	CMesh* tmp = brush.mesh();
	
	tmp->move(mesh);
	
	delete tmp;

}
\endcode 
 */
class CBrush {

 public:

	CBrush();
	~CBrush();

 public:

	CPlane* plane_head;		///< First plane in DLL.
	CPlane* plane_tail;		///< Last plane in DLL.
	int plane_count;		///< Number of planes in DLL.
        void* data;			///< Custom data field.

 public:

	CPlane* add(CPlane* plane);
	CPlane* add_head(CPlane* plane);
	CPlane* add_tail(CPlane* plane);
	void remove(CPlane* plane);
	CPlane* unlink(CPlane* plane);
	void clear();

 public:

//	bool on_f(poly3_t* p1, poly3_t* p2);
	bool convex();				// Returns true if this is a convec brush.
	/**
	 * Creates an initial polygon, that we can clip, from a given plane.
	 * @param plane The plane we will create the initial polygon for. Assumes the plane is normalized!
	 */
	CPoly* initial(CPlane* plane);
	CPoly* poly(CPlane* plane);
	CMesh* mesh();
	CEdge* points();

 public:

 	CBrush* copy();				// Returns a copy of this brush.
	CBrush* clip(CPlane* plane);		// Clips this brush, returning the resulting brush
	
 public:
 
	CBrush* prev;
 	CBrush* next;
  
 public:
 
	CBrush* unlink();

};

#include "inline/cBRUSH.inl"

#endif
