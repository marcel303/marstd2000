#ifndef __cOCTREE_h__
#define __cOCTREE_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cOCTREE.cpp: COctree implementation. */
//////////////////////////////////////////////////////////////////////

#include <vector>

/// Geometry: Octree space partitioning class.
/**
 * Octree class. Use octrees to reject polygons from your rendering pipeline as soon as possible.
 * Unlike the BSP tree this class doesn't guarantee perfect back-to-front rendering. On the other hand, it doesn't
 * require as much time to create an octree and filter the polygons into it as it takes with BSP creation, and
 * polygons won't get split. Octrees are great in an z-buffered environment.
 */
class COctree {

 public:
 
	COctree();
	~COctree();
	void zero();
	void zero_children();
	void clear();
	void clear_children();
	
 public:
 
	COctree* child[8];		///< Child nodes.
	bool leaf;			///< True if this node is a leaf.
	CVector mid;			///< Center of cube.
	float size;			///< Size of cube.
	std::vector<CPoly*> polygon;	///< Polygons.
  	
 public:

	/**
	 * Calculates the initial cube size.
	 * @return The maximal side length.
	 */
 	float calculate_cube_size();
 	/**
 	 * Starts building an octree.
 	 * @param min_cube_size Minimal size a node must have to be split.
 	 * @param min_polycount Minimal number of polygons a node must have to be split.
 	 */
 	void create(float min_cube_size, int min_polycount);
 	/**
 	 * Creates child nodes from this node and splits child nodes.
 	 */
	void create(int levels, int min_polycount);
	/**
	 * Filters a polygon into the tree. The polygon will be added to every leaf it touches. It is up to the user
	 * not to redraw polygons as they are referenced a second time.
	 */
	void filter(CPoly* poly);

};

#include "inline/cOCTREE.inl"

#endif
