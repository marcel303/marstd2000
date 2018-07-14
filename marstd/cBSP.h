#ifndef __cBSP_h__
#define __cBSP_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cBSP.h: CBsp implementation. */
//////////////////////////////////////////////////////////////////////

#include "cDEBUG.h"
#include "cVECTOR.h"
#include "cPLANE.h"
#include "cPOLY.h"
#include "cSPHERE.h"

typedef struct {

	CPlane p;			///< Splitting plane.

	int fc;				///< Number of polygons in front of plane after split.
	int bc;				///< Number of polygons behind plane after split.
	int dc;				///< Difference of front and back polys.
	int sc;				///< Number of polygons spanning plane.

} bsp_split_info_t;

typedef struct {

	void* data;			///< Old data pointer of poly.
	CSphere sphere;			///< Bubble around the origional polygon.
	
} bsp_polydata_t;

//#define BSP_ALTERNATE_CANCEL		///< If uncommented, an alternate 'bail out method' will be used.
#define BSP_USE_SPHERES			///< If uncommented, bounding spheres are used for faster testing.
#define BSP_DEFAULT_EPS	0.001		///< Epsilon value to keep floating point happy and minimize splits.

/// Geometry: BSP class.
/**
 * BSP (Binary Space Partitioning) class. This class will take any mesh and create a BSP from it.
 * Use it to optimize collision detection and to maximize overdraw on accelerated platforms, or z-sorting on software platforms.<BR>
\code
// Example:

void create_bsp(CBsp& bsp) {

	// Add a donut shape.
	MARX::gen_donut(bsp, 10, 10, 2.0, 1.0);
	
	// Add a square.
	CPoly* poly = bsp.add(new CPoly);
	for (int i=0; i<4; i++) {
		CEdge* egde = poly->add(new CEdge);
		edge->p[0] = sin(i/4.0*M_PI);
		edge->p[1] = cos(i/4.0*M_PI);		
	}
	poly->init();	// Very important. This will set up the plane, which is neccessary for BSP to work.
	
	// Partition BSP.
	bsp.split();
	
}
\endcode
 * @author Marcel Smit
 */
class CBsp : public CMesh {

 public:

	CBsp();
	~CBsp();

 public:

 	CPlane p;			///< Splitting plane.
 	CBsp* b_f;			///< Front child.
	CBsp* b_b;			///< Back child/
	float eps;			///< Epsilon to keep floating point happy and reduce splits.
	
	bool initialized;		///< False if this BSP need initialization.
	bsp_polydata_t* polydata;	///< Polygon info structure.
	CVector mins, maxs;		///< Bounding box.
	CSphere sphere;			///< Bounding sphere.
	float volume;			///< Bounding box volume.
	bool no_splitting_plane;	///< Set to true if split_plane() could not find any suitable splitting plane.	
 
 public:

	/**
	 * Return true if all polygons lie in one plane.
	 */
	bool coplanar();
	/**
         * Moves a polygon from this BSP to another.
         * @param poly Polygon to move.
         * @param bsp Destination BSP.
         */ 	
	void move(CPoly* poly, CBsp* bsp);
	/**
	 * Return true if the polygon lies on the front side of the plane.
	 * @param poly Polygon to test.
	 * @param dup If true, the epsilon value will be doubled.
	 */
	int on_f(CPoly* poly, int dup=false);
	/**
	 * Return true if the polygon lies on the back side of the plane.
	 * @param poly Polygon to test.
	 * @param dup If true, the epsilon value will be doubled.
	 */
	int on_b(CPoly* poly, int dup=false);
	/**
	 * Fills in the bsp_split_info_t structure with some statistics on how this plane will split the geometry.
	 * This information is used to decide which plane to use to split.
	 * @param p Splitting plane to get info about.
	 * @param i Info structure that will be filled.
	 * @param flip If true, the plane will be flipped (mirrored).
	 */
	void split_info(CPlane& p, bsp_split_info_t& i, bool flip);
	/**
         * Returns the 'best' splitting plane.
         */
	bool test1(bsp_split_info_t* i1, bsp_split_info_t* i2);
	bool test2(bsp_split_info_t* i1, bsp_split_info_t* i2);
	bool test(bsp_split_info_t* i1, bsp_split_info_t* i2);
	CPlane split_plane();
	/**
         * Split the polygon, dividing it over the front and back child.
         * @param poly Polygon to split. This polygon must not be on the front side or back side. It must be spanning.
         */
	void split(CPoly* poly);
	/**
	 * Splits the BSP node if neccessary.
	 */
	void split();							// splits/moves all polys
	CSphere bounding_sphere(CPoly* poly);
	void init_bounding_spheres();
	void free_bounding_spheres();
	void restore_polydata();
	/**
         * Initializes the bounding box. This is done for every node of the tree.
         */	   
	void init_bounding_box();

};

#include "inline/cBSP.inl"

#endif
