//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file marx.h: Extra utilities based upon MARSTD. */
//////////////////////////////////////////////////////////////////////

// NOTE: Include the file marx.cpp exactly once into your project.

#include <vector>
#include "marstd.h"

/// MARSTD eXtension library.
/**
 * MARX is an extension to the regular MARSTD librairy. You will need to include
 * marstd/marx.cpp exactly once into your program. MARX adds amongst other thing
 * a shared matrix and polygon generation routines.
 */
namespace MARX {

	// xGLOBALS.cpp:
	
	extern CMatrix matrix;
	
	// xTESTENV.cpp:
	
	extern bool testenv_initialize();
	extern bool testenv_shutdown();
	
	// xMESSAGE.cpp:
	
	void message(char* message, ...);

	// xPOLY.cpp:
	
	extern void gen_cube(CMesh& mesh);
	extern void gen_circle(CMesh& mesh, int slices);
	extern void gen_cilinder(CMesh& mesh, int slices);
	extern void gen_cone(CMesh& mesh, int slices);
	extern void gen_sphere(CMesh& mesh, float radius, int slices, int division);
	extern void gen_donut(CMesh& mesh, int sides, int slices, float r1, float r2);
	
	// xCOLLISION.cpp:
	
	extern bool collision(CPoly& poly, CVector& point);
	extern bool collision(CPoly& poly, CVector& point, CVector& delta, float& t);
	extern bool collision(CPoly& poly, CSphere& sphere);
        extern bool collision(CPoly& poly, float extrusion, CVector& point, CVector& delta, float& t, CVector& normal);
	extern bool collision(CPoly& poly, CSphere& sphere, CVector& delta, float& t, CVector& normal);

	// xBSP.cpp
        
        extern CBsp* get_hit_leaf(CBsp& bsp, CVector point);	
	extern void get_hit_leafs(CBsp& bsp, CVector point, float radius, std::vector<CBsp*>& out);

};
