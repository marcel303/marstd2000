#ifndef __cSPHERE_h__
#define __cSPHERE_h__

//////////////////////////////////////////////////////////////////////
// (c)2002 Marcel Smit
//
// p.smit@hccnet.nl
//
// This code may not be used in a commercial product without my permission.
// If you redistribute it, this message must remain intact. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/** @file cSPHERE.h: CSphere implementation. */
//////////////////////////////////////////////////////////////////////

#include "cVECTOR.h"
#include "cLIST.h"

//---------------------------------------------------------------------------

/// Geometry: Sphere type.
/**
 * Sphere type. Generic position, radius.
 */
class CSphere {

 public:
 
	CSphere() {
		radius = 0.0;
		prev = 0;
		next = 0;
	} 
	~CSphere() {
		unlink();
	}
	
 public:	
 
	CVector position;
	float radius;		///< Radius of sphere. Any point for which (point - position).size() == radius lies on sphere.
  
 public:

	/**
	 * Copies sphere.
	 */
	void operator=(CSphere sphere) {
		position = sphere.position;
		radius = sphere.radius;
	} 
	
 public:
 
	CSphere* prev;
 	CSphere* next;  	
 	
 public:
 
	CSphere* unlink() {
		DLLIST_UNLINK_SELF();
		return this;
	}    	

};

//---------------------------------------------------------------------------

#endif
