//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file xBSP.cpp: MARX BSP utilities. */
//////////////////////////////////////////////////////////////////////

#include "marx.h"

/**
 * Find the leaf a point is in.
 * @param bsp Root of BSP tree.
 * @param point Point to locate.
 * @return The BSP leaf the point is in.
 */
CBsp* MARX::get_hit_leaf(CBsp& bsp, CVector point) {

	CBsp* tmp = &bsp;

        while (tmp->poly_count == 0) {

        	float distance = tmp->p * point;

                if (distance < 0.0)
                	tmp = tmp->b_b;
		else
                	tmp = tmp->b_f;
        
        }

        return tmp;

}

static CVector point;
static float radius;
static std::vector<CBsp*>* out;

static void do_get_hit_leafs(CBsp* bsp) {

	if (!bsp->b_f) {
	
		out->push_back(bsp);
		
	} else {

		if (bsp->p*point >= -radius)
			do_get_hit_leafs(bsp->b_f);
		if (bsp->p*point <= +radius)
			do_get_hit_leafs(bsp->b_b);
		
	}		

}

/**
 * Creates a list of all BSP leaf nodes hit by a sphere.
 * @param bsp Root BSP node.
 * @param point Center of sphere.
 * @param radius Radius of sphere.
 * @param out Vector to store pointer to BSP leafs.
 */
void MARX::get_hit_leafs(CBsp& bsp, CVector point, float radius, std::vector<CBsp*>& out) {

	::point = point;
	::radius = radius;
	::out = &out;
	
	do_get_hit_leafs(&bsp);

}

